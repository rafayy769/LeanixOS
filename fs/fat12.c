#include <fs/fat12.h>
#include <driver/fdc.h>
#include <driver/block.h>
#include <mm/kheap.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <fs/vfs.h>

#define LOG_MOD_NAME 	"FAT"
#define LOG_MOD_ENABLE  0
#include <log.h>

/* We register a defined filesystem in the VFS layer by exporting a global 
	object here, other instances in the kernel use this object. */

//! defines the FAT12 filesystem type, uniquely determined by name and ops
fs_type_t 			fat12_fs_type = {

	.fs_name = "fat12",
	.vfs_ops = {
		.mount   = fat12_mount,
		.unmount = fat12_unmount
	}

};

//! Operations table for vnode associated operations
static vnode_ops_t  fat12_vnode_ops = {

	.open  = fat12_open,
	.close = fat12_close,
	.read  = fat12_read,
	.write = fat12_write,
	.readdir = NULL			/* to be added later */,
	.create  = NULL,
	.mkdir   = NULL,
	.remove  = NULL

};

/* Implementation private helper functions declarations */

//! return the FAT12 entry value for the given cluster number
static uint16_t _fat12_get_fentry (fat12_handle_t* fs, uint16_t cluster);

//! convert 8.3 filename to normal string
static void 	_fat12_8_3to_normal (const char* fat_name, char* output);

//! convert a normal string to 8.3 filename
static void 	_fat12_normal_to_8_3 (const char* name, char* fat_name);

//! convert a cluster number to LBA sector number
static uint32_t _fat12_cluster_to_lba (fat12_handle_t* fs, uint16_t cluster);

//! lookup a file/directory in the given directory vnode
static vnode* 	_fat12_lookup (vnode* dir_node, const char* name);

/* Implementation of helper private functions. */

uint16_t 	_fat12_get_fentry (fat12_handle_t* fs, uint16_t cluster) {

	/* each FAT12 entry is 12 bits long, so we need to read 1.5 bytes per entry.
		this means that every two entries take up 3 bytes. To find the offset of
		the entry in the FAT table, we can use the formula:
			offset = (cluster_number * 3) / 2
		If the cluster number is even, the entry starts at the calculated offset.
		If it's odd, the entry starts 4 bits (0.5 bytes) after the calculated 
		offset. */

	uint32_t fat_offset = (cluster * 3) / 2;
	uint16_t entry      = *(uint16_t*) ((uintptr_t) fs->fat_table + fat_offset);

	if (cluster & 0x1) {
		/* odd cluster number, use high 12 bits */
		entry >>= 4;
	} else {
		/* even cluster number, use low 12 bits */
		entry &= 0x0FFF;
	}

	return entry & FAT12_CLUSTER_MASK;
}

void 		_fat12_8_3to_normal (const char* fat_name, char* output) {

	int i, j = 0;

	/* copy name part */
	for (i = 0; i < 8 && fat_name[i] != ' '; i++) {
		output[j++] = fat_name[i];
	}

	/* add extension if present */
	if (fat_name[8] != ' ') {
		output[j++] = '.';
		for (i = 8; i < 11 && fat_name[i] != ' '; i++) {
			output[j++] = fat_name[i];
		}
	}

	output[j] = '\0';

	LOG_DEBUG ("converted 8.3 format %.11s to %s\n", fat_name, output);

}

void 		_fat12_normal_to_8_3 (const char* name, char* fat_name) {

	/* clear the fat_name first */
	memset (fat_name, ' ', 11);

	int i, j = 0;

	/* copy name part */
	for (i = 0; i < 8 && name[j] != '\0' && name[j] != '.'; i++, j++) {
		fat_name[i] = name[j];
	}

	/* skip to extension if present */
	if (name[j] == '.') {
		j++;
	}

	/* copy extension part */
	for (i = 8; i < 11 && name[j] != '\0'; i++, j++) {
		fat_name[i] = name[j];
	}
	
}

uint32_t 	_fat12_cluster_to_lba (fat12_handle_t* fs, uint16_t cluster) {

	/* Cluster numbers start at 2, so cluster 2 is the first data cluster.
		The formula to convert a cluster number to LBA sector number is:
			LBA = data_area_start + (cluster_number - 2) * sectors_per_cluster */

	if (cluster < 2) {
		LOG_ERROR ("invalid cluster number %u\n", cluster);
		return 0;
	}

	uint32_t lba = fs->data_area_start + 
				   (cluster - 2) * fs->bpb->sectors_per_cluster;

	return lba;
}

vnode* 		_fat12_lookup (vnode* dir_node, const char* name) {

	// check if its the root directory
	if (strcmp (dir_node->name, "/") != 0) {
		LOG_ERROR ("lookup in non-root directories not supported yet\n");
		return NULL;
	}

	fat12_handle_t* fs = (fat12_handle_t*) dir_node->vfs_ptr->fs_data;
	if (!fs) {
		LOG_ERROR ("FAT12 handle is NULL\n");
		return NULL;
	}

	/* convert the name to 8.3 format */
	char fat_name[11];
	const char* p = name;
	
	// skip leading slashes
	while (*p == '/') p++;

	_fat12_normal_to_8_3 (p, fat_name);

	/* read the root directory entries sector by sector */
	fat12_dir_entry_t dir_entries [16]; /* read 16 entries at a time */
	uint32_t total_entries = 0;

	for (uint32_t s = 0; s < fs->root_dir_sectors; s++) {

		uint32_t sector = fs->root_dir_start + s;

		/* read the sector */
		blkread (fs->block_dev, sector, (uint8_t*) dir_entries);

		/* scan the entries */
		for (uint32_t i = 0; i < 16; i++) {

			/* check for end of directory */
			if (dir_entries[i].filename[0] == FAT12_ENDDIR) {

				LOG_DEBUG ("end of directory reached after %u entries\n",
							total_entries);
				return NULL;
			
			}

			/* skip deleted entries and volume labels */
			if ((uint8_t)dir_entries[i].filename[0] == FAT12_DELETED ||
				(dir_entries[i].attributes & FAT12_VID)) 
				{ continue; }

			total_entries++;

			if ( memcmp (dir_entries[i].filename, fat_name, 11) == 0 ) {

				/* found the entry */
				LOG_DEBUG ("found entry %.11s in root dir\n", fat_name);

				/* create and setup a vnode for this entry */
				vnode* new_node = malloc (sizeof(vnode));
				if (!new_node) {
					LOG_ERROR ("failed to allocate memory for vnode\n");
					return NULL;
				}
				strncpy (new_node->name, name, sizeof(new_node->name));
				new_node->vfs_mountedhere = NULL;
				new_node->vfs_ptr         = dir_node->vfs_ptr;
				new_node->ops 		   	  = &fat12_vnode_ops;
				new_node->flags 		  = 0;
				new_node->type 			  = (dir_entries[i].attributes & FAT12_DIR)
											? V_DIRECTORY : V_FILE;
				

				/* create and setup the inode object, the internal data for the
					vnode */
				
				inode* new_inode = malloc (sizeof(inode));
				if (!new_inode) {
					LOG_ERROR ("failed to allocate memory for inode\n");
					free (new_node);
					return NULL;
				}

				memcpy (&new_inode->dir_entry, &dir_entries[i],
						sizeof(fat12_dir_entry_t)); // read the entry data
				new_inode->dir_sector = sector;
				new_inode->dir_offset = i;

				new_node->data = new_inode;

				return new_node;
			}
		}
	}

	LOG_DEBUG ("file %s not found in root directory after %u entries\n",
				name, total_entries);
	return NULL;
}


/* Implementation of public functions. */

//! mounts a FAT12 filesystem from the given device
vfs* fat12_mount (const char* device) {

	fat12_handle_t* fs = malloc (sizeof(fat12_handle_t));
	if (!fs) {
		LOG_ERROR ("failed to allocate memory for FAT12 handle\n");
		return NULL;
	}

	uint8_t* bootsector = malloc (512);	/* default sector size 512 */
	if (!bootsector) {
		LOG_ERROR ("failed to allocate memory for bootsector\n");
		free (fs);
		return NULL;
	}

	block_device_t* block_dev = blkdev_get_by_name (device);
	fs->block_dev = block_dev;
	if (!block_dev) {
		LOG_ERROR ("block device %s not found\n", device);
		free (bootsector);
		free (fs);
		return NULL;
	}

	LOG_DEBUG ("reading bootsector from dev %s...\n", block_dev->name);
	blkread (block_dev, 0, bootsector);

	/* Initialize the values */

	fs->bpb              = (fat12_bpb_t*) bootsector;
	fs->ebr              = (fat12_ebr_t*) (bootsector + sizeof(fat12_bpb_t));

	uint8_t lba_begin 	 = fs->bpb->hidden_sectors;
	fs->fat_start        = lba_begin + fs->bpb->rsrvd_cnt;
	fs->root_dir_start   = fs->fat_start + 
							(fs->bpb->num_fats * fs->bpb->sectors_per_fat);
	fs->root_dir_sectors = (fs->bpb->max_root_dir_entries * sizeof(fat12_dir_entry_t))
							/ fs->bpb->bytes_per_sector;
	fs->data_area_start  = fs->root_dir_start + fs->root_dir_sectors;

	/* allocate and load the FAT */

	size_t fat_size = fs->bpb->sectors_per_fat * fs->bpb->bytes_per_sector;
	fs->fat_table 	= malloc (fat_size);
	if (!fs->fat_table) {

		LOG_ERROR ("failed to allocate memory for FAT table\n");
		free (bootsector);
		free (fs);
		return NULL;
	
	}

	LOG_DEBUG ("reading FAT table from dev %s... at %p\n", device, fs->fat_table);
	for (uint32_t i = 0; i < fs->bpb->sectors_per_fat; i++) {
		
		blkread (block_dev, fs->fat_start + i,
				 fs->fat_table + (i * fs->bpb->bytes_per_sector));
	
	}

	/* dump information about the mounted filesystem */
	
	LOG_DEBUG ("FAT12 filesystem mounted:\n");
	LOG_DEBUG ("  fs handle:\n");
	LOG_DEBUG ("  fat_start:        %u\n", fs->fat_start);
	LOG_DEBUG ("  num_fats:         %u\n", fs->bpb->num_fats);
	LOG_DEBUG ("  root_dir_start:   %u\n", fs->root_dir_start);
	LOG_DEBUG ("  root_dir_sectors: %u\n", fs->root_dir_sectors);
	LOG_DEBUG ("  data_area_start:  %u\n", fs->data_area_start);
	LOG_DEBUG ("  bytes/sector:     %u\n", fs->bpb->bytes_per_sector);
	LOG_DEBUG ("  fat_table:        %p\n", fs->fat_table);

	/* the underlying driver is compliant because of this: it generates an 
		instance of the VFS (mounted) instance that can be read transparently
		by the kernel. */

	vfs* fsys = malloc (sizeof(vfs));
	if (!fsys) {
		LOG_ERROR ("failed to allocate memory for vfs structure\n");
		free (fs->fat_table);
		free (bootsector);
		free (fs);
		return NULL;
	}

	fsys->type   	= &fat12_fs_type;
	fsys->fs_data 	= fs;

	/* zero init the parts set by VFS */
	fsys->vcovered       = NULL;

	/* read and setup the root vnode of the mounted fs */
	vnode* vroot = malloc (sizeof(vnode));

	fsys->vroot  = vroot;	// root vnode of this fs

	strncpy (vroot->name, "/", sizeof(vroot->name));
	vroot->vfs_mountedhere = NULL;
	vroot->vfs_ptr         = fsys;
	vroot->type            = V_DIRECTORY;
	vroot->ops             = &fat12_vnode_ops;
	vroot->flags           = 0;

	/* creatup and setup private inode data */
	inode* root_inode = malloc (sizeof(inode));

	root_inode->dir_offset           = 0;
	root_inode->dir_sector           = fs->root_dir_start;
	root_inode->dir_entry.attributes = FAT12_DIR;
	// zero init the rest of the entry, since hardcoding for root dir
	memset (&root_inode->dir_entry, 0, sizeof(fat12_dir_entry_t));
	
	vroot->data = root_inode;

	return fsys;

}

//! unmounts a FAT12 filesystem
int32_t fat12_unmount (vfs* fsys) {

	fat12_handle_t* fs = (fat12_handle_t*) fsys->fs_data;

	if (!fs) {
		LOG_ERROR ("FAT12 handle is NULL\n");
		return -1;
	}
	if (fs->fat_table) {
		free (fs->fat_table);
		fs->fat_table = NULL;
	}
	/* should free the entire bootsector */
	if (fs->bpb) {
		free (fs->bpb);
		fs->bpb = NULL;
	}
	free (fs);
	
	free (fsys->vroot->data);
	free (fsys->vroot);
	free (fsys);

	fs   = NULL;
	fsys = NULL;

	LOG_DEBUG ("FAT12 filesystem unmounted\n");
	return 0;

}

//! open a file/directory at the given path, returning a vnode object
vnode* fat12_open (vnode* node, const char* path, uint32_t flags) {

	// paths always absolute wrt root of this fs
	if (!node || !path || strlen (path) == 0) {
		LOG_ERROR ("invalid node or path specified\n");
		return NULL;
	}

	LOG_DEBUG ("FAT12 open called on path %s\n", path);

	vnode* root = node->vfs_ptr->vroot;
	
	// check if file is found in root directory
	vnode* found = _fat12_lookup (root, path);

	if (found) {
		LOG_DEBUG ("FAT12 open: file %s found\n", path);
		return found;
	} else {
		LOG_DEBUG ("FAT12 open: file %s not found\n", path);
		return NULL;
	}

}

//! read from the given vnode into the provided buffer, offset specified
int32_t fat12_read (vnode* node, uint32_t offs, uint32_t size, void* buf) {

	if (!node || !node->data || !buf || size == 0) {
		LOG_ERROR ("invalid node or buffer specified for read\n");
		return -1;
	}

	inode*      _inode 	  = (inode*) node->data;
	fat12_handle_t* fs    = (fat12_handle_t*) node->vfs_ptr->fs_data;

	if (!fs) {
		LOG_ERROR ("FAT12 handle is NULL\n");
		return -1;
	}
	if (_inode->dir_entry.attributes & FAT12_DIR) {
		LOG_ERROR ("read operation on directories not supported yet\n");
		return -1;
	}

	uint32_t file_size = _inode->dir_entry.file_size;
	if (offs >= file_size) {
		LOG_DEBUG ("read offset %u beyond end of file %u\n", offs, file_size);
		return 0; // EOF
	}

	if (offs + size > file_size) {
		size = file_size - offs; // adjust size to read only till EOF
	}

	LOG_DEBUG ("reading %u bytes from file at offset %u\n", size, offs);

	/* calculate bytes per cluster */
	uint32_t bytes_per_cluster  = fs->bpb->sectors_per_cluster *
								  fs->bpb->bytes_per_sector;

	/* first calculate the first cluster to read */
	uint32_t cluster_offset     = offs / bytes_per_cluster;
	uint16_t start_cluster 		= _inode->dir_entry.first_cluster_low
								  + (cluster_offset);
	uint16_t curr_cluster 		= start_cluster;

	/* required clusters to read and starting offset in first cluster */
	uint32_t offset_in_cluster  = offs % bytes_per_cluster;
	uint16_t clusters_to_read   = ((offset_in_cluster + size) / bytes_per_cluster) +
								  (((offset_in_cluster + size) % bytes_per_cluster) ? 1 : 0);

	LOG_DEBUG ("start_cluster: %u, clusters_to_read: %u, offset_in_cluster: %u\n",
				start_cluster, clusters_to_read, offset_in_cluster);

	void* read_buf = malloc (bytes_per_cluster);
	if (!read_buf) {
		LOG_ERROR ("failed to allocate memory for read buffer\n");
		return -1;
	}

	/* read cluster by cluster */
	uint16_t 	cluster_count = 0;
	uint32_t 	bytes_read    = 0;

	while (cluster_count < clusters_to_read) {

		uint16_t fentry = _fat12_get_fentry (fs, curr_cluster);
		uint32_t lba 	= _fat12_cluster_to_lba (fs, curr_cluster);

		// read in the entire cluster/sector
		blkread ( fs->block_dev, lba, (uint8_t*) read_buf );

		size_t copy_start = (cluster_count == 0) ? offset_in_cluster : 0;
		size_t copy_end   = (cluster_count == clusters_to_read - 1) ?
							((offs + size) % bytes_per_cluster) : 
							bytes_per_cluster;
		if (copy_end == 0) copy_end = bytes_per_cluster; // full cluster

		memcpy ( (uint8_t*) buf + bytes_read,
				 (uint8_t*) read_buf + copy_start,
				 copy_end - copy_start );

		bytes_read 	+= (copy_end - copy_start);
		LOG_DEBUG ("copying bytes 0x%x to 0x%x (%u) from cluster %u\n",
					copy_start, copy_end, bytes_read, curr_cluster);
		
		cluster_count++;

		if (curr_cluster >= FAT12_EOC_MIN) {

			/* end of cluster chain reached */
			LOG_DEBUG ("end of cluster chain reached at cluster %u\n",
						curr_cluster);
			break;

		}

		curr_cluster = fentry; // next cluster in chain

	}

	free (read_buf);
	return size; // return bytes read (as requested)
}

// empty impls of the remaining ones

int32_t fat12_close (vnode* node) {return -1;}
int32_t fat12_write (vnode* node, uint32_t offs, uint32_t size, void* buf) {return -1;}
int32_t fat12_readdir (vnode* node, vnode** dirents, uint32_t* count) {return -1;}
