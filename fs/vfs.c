#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <fs/vfs.h>
#include <fs/fat12.h>
#include <mm/kheap.h>

#define LOG_MOD_NAME 	"VFS"
#define LOG_MOD_ENABLE  0
#include <log.h>

extern fs_type_t 		fat12_fs_type; // defined in fat12.c
extern fs_type_t 		hfs_fs_type;   // defined in hfs.c

/* start with 16 possible, then realloc */
#define INIT_MPOINTS 	16

/* Some helpful macros to help reduce verbosity */

#define MOUNT(fs_type, src_dev) \
	( (fs_type)->vfs_ops.mount(src_dev) )

#define UNMOUNT(fs) \
	( (fs)->type->vfs_ops.unmount(fs) )

/* Private variables */

//! table of supported filesystem types
fs_type_t* fs_types[] = {
	&fat12_fs_type,
	&hfs_fs_type,
	NULL
};

//! table of mounted filesystems information
static  vfs** 		_mountpoints = NULL;
static 	size_t		_curr_size 	 = 0;

//! current root vnode of the VFS (will be deprecated)
static 	vnode* 		_vfs_root_vnode = NULL;

//! add a newly created entry to the list of mount points
static void 		_add_to_mpoints (vfs* _v);

//! helper to resolve mountpoint from path and get root vnode and relative path
static vnode* 		_vfs_resolve_mountpoint (const char* path, char* rel_path, size_t rel_path_size);


/* Public functions of the interface */

void vfs_init () {

	_vfs_root_vnode = NULL;
	_mountpoints 	= malloc (INIT_MPOINTS * sizeof(vfs*));
	memset (_mountpoints, 0, INIT_MPOINTS * sizeof (vfs*));
	_curr_size 		= 0;

}

int32_t vfs_mount (const char* src_dev, const char* mount_path, const char* fs_type) {

	/* first find the corresponding fs type object from the available ones. */
	fs_type_t* 		fst = NULL;
	for (int i = 0; fs_types[i] != NULL; i++) {

		if (strcmp (fs_types[i]->fs_name, fs_type) == 0) {
			fst = fs_types[i];
			break;
		}
	
	}
	if (!fst) {
		LOG_ERROR ("filesystem type %s not supported\n", fs_type);
		return -1;
	}

	/* the fs driver first tries to read the superblock etc. on the specified
		device, by the mount function. */
	vfs* filesystem = MOUNT (fst, src_dev);
	if (!filesystem) {
		LOG_ERROR ("failed to mount filesystem %s from device %s\n",
					fs_type, src_dev);
		return -1;
	}

	/* create the vnode where this fsys is mounted at and initialize to all 0 */
	vnode*  mount_at = malloc (sizeof(vnode));
	memset (mount_at, 0, sizeof(vnode));

	/* 1 added to mnt path to skip the starting '/'. the vnode type will be a
		directory. all mountpoints are always dirs */
	
	mount_at->vfs_mountedhere = filesystem;
	mount_at->type 			  = V_DIRECTORY;
	strncpy (mount_at->name, mount_path+1, sizeof(mount_at->name));
	
	/* the vcovered param is set to be NULL by the fs driver, and the kernel sets
		the appropriate mapping */
	
	filesystem->vcovered = mount_at;
	_add_to_mpoints (filesystem);

	LOG_DEBUG ("mounted filesystem %s from device %s at %s\n",
				fs_type, src_dev, mount_path ? mount_path : "/");

	return 0;
}

int32_t vfs_unmount (const char* mount_path) {

	/* find the mounted fsys at the specified path */
	for (size_t i = 0; i < _curr_size; i++)
	{
		vfs* _mp = _mountpoints[i];
		if (!_mp)
			continue;

		if ( strcmp(_mp->vcovered->name, mount_path+1) == 0 )
		{
			int32_t ret = UNMOUNT (_mp);
			if (ret == 0) {
				LOG_DEBUG ("unmounted filesystem %s at %s\n",
							_mp->type->fs_name, mount_path);
			} else {
				LOG_ERROR ("failed to unmount filesystem %s at %s\n",
							_mp->type->fs_name, mount_path);
			}
			return ret;
		}
	}

	LOG_ERROR ("no mounted filesystem found at %s\n", mount_path);
	return -1;
}

//! create a new file at the specified path
int32_t vfs_create (const char* path, uint32_t flags) {

	char rel_path[256];
	vnode* root_node = _vfs_resolve_mountpoint (path, rel_path, sizeof(rel_path));
	if (!root_node) {
		return -1;
	}

	// call the create function of the filesystem to create the file
	int32_t ret = root_node->ops->create (root_node, rel_path);
	if (ret < 0) {
		LOG_ERROR ("vfs_create: failed to create file %s\n", path);
		return -1;
	}

	return 0;

}

//! create a new directory at the specified path
int32_t vfs_mkdir (const char* path) {

	char rel_path[256];
	vnode* root_node = _vfs_resolve_mountpoint (path, rel_path, sizeof(rel_path));
	if (!root_node) {
		return -1;
	}

	// call the mkdir function of the filesystem to create the directory
	int32_t ret = root_node->ops->mkdir (root_node, rel_path);
	if (ret < 0) {
		LOG_ERROR ("vfs_mkdir: failed to create directory %s\n", path);
		return -1;
	}

	return 0;
}

//! remove a file/directory at the specified path
int32_t vfs_remove (const char* path) {

	char rel_path[256];
	vnode* root_node = _vfs_resolve_mountpoint (path, rel_path, sizeof(rel_path));
	if (!root_node) {
		return -1;
	}

	// call the remove function of the filesystem to remove the file/directory
	int32_t ret = root_node->ops->remove (root_node, rel_path);
	if (ret < 0) {
		LOG_ERROR ("vfs_remove: failed to remove %s\n", path);
		return -1;
	}

	return 0;
}

//! OPEN the file at the specified path with the given flags
file_t* vfs_open (const char* path, uint32_t flags) {

	char rel_path[256];
	vnode* root_node = _vfs_resolve_mountpoint (path, rel_path, sizeof(rel_path));
	if (!root_node) {
		return NULL;
	}

	// call the open function of the filesystem to open the file
	vnode* result = root_node->ops->open (root_node, rel_path, flags);
	if (!result) {
		LOG_ERROR ("vfs_open: failed to open file %s\n", path);
		return NULL;
	}

	file_t* file 	= malloc (sizeof(file_t));
	file->vnode_ptr = result;
	file->f_offset  = 0;
	file->f_flags   = flags;

	return file;

}

//! CLOSE a previously opened file
int32_t vfs_close (file_t* file) {

	if (!file || !file->vnode_ptr) {
		LOG_ERROR ("invalid file specified for close\n");
		return -1;
	}

	int32_t ret = file->vnode_ptr->ops->close (file->vnode_ptr);
	free (file);

	return ret;
	
}

//! read the file into a buffer
int32_t vfs_read (file_t* file, void* buf, uint32_t size) {

	LOG_DEBUG ("vfs_read: file=%p, buff=%p, size=%u\n", file, buf, size);

	if (!file || !file->vnode_ptr || !buf || size == 0) {
		LOG_ERROR ("invalid file or buffer specified for read\n");
		return -1;
	}

	/* read operation returns non-zero num bytes read */
	int32_t bytes_read = file->vnode_ptr->ops->read (file->vnode_ptr,
													 file->f_offset,
													 size, buf);
	if (bytes_read > 0) {
		file->f_offset += bytes_read;
	}

	return bytes_read;
	
}

//! write the file to a buffer
int32_t vfs_write (file_t* file, void* buf, uint32_t size) {

	LOG_DEBUG ("vfs_write: file=%p, buff=%p, size=%u\n", file, buf, size);

	if (!file || !file->vnode_ptr || !buf || size == 0) {
		LOG_ERROR ("invalid file or buffer specified for write\n");
		return -1;
	}

	/* write operation returns non-zero num bytes written */
	int32_t bytes_written = file->vnode_ptr->ops->write (file->vnode_ptr,
														file->f_offset,
														size, buf);
	if (bytes_written > 0) {
		file->f_offset += bytes_written;
	}

	return bytes_written;
}

// Get mounted filesystem by mount path
vfs* vfs_get_mounted(const char* mount_path) {
	if (!mount_path) return NULL;

	const char* search_name = mount_path;
	if (mount_path[0] == '/') search_name = mount_path + 1;

	for (size_t i = 0; i < _curr_size; i++) {
		vfs* _mp = _mountpoints[i];
		if (!_mp) continue;

		if (strcmp(_mp->vcovered->name, search_name) == 0) {
			return _mp;
		}
	}
	return NULL;
}

// Private helpers

void _add_to_mpoints (vfs* _v) {

	for (size_t i = 0; i < _curr_size; i++)
	{
		if (!_mountpoints[i])
		{
			_mountpoints [i] = _v;
			return;
		}
	}

	void* temp = realloc (_mountpoints, (_curr_size + 5) * sizeof(vfs*));
	if (!temp)
	{
		LOG_DEBUG ("failed to reallocate memory for mountpoints\n");
		return;
	}

	_mountpoints = temp;
	_mountpoints [_curr_size] = _v;
	_curr_size += 5;

}

//! Helper function to resolve mountpoint from path
//! Returns the root vnode of the mounted filesystem and fills rel_path with the
//! relative path (including leading '/') to pass to the underlying filesystem
vnode* _vfs_resolve_mountpoint (const char* path, char* rel_path, size_t rel_path_size) {

	if (!path || !rel_path || rel_path_size == 0) {
		LOG_ERROR ("invalid arguments to _vfs_resolve_mountpoint\n");
		return NULL;
	}

	/* Handle paths that don't start with '/' */
	if (path[0] != '/') {
		LOG_ERROR ("path must be absolute (start with /)\n");
		return NULL;
	}

	/* Parse the mountpoint name (first component of path) */
	char buf[256];
	strncpy (buf, path, sizeof(buf));
	buf[sizeof(buf) - 1] = '\0';

	char* next_token = NULL;
	char* token = strtok_r (buf, "/", &next_token);

	if (!token) {
		LOG_ERROR ("failed to parse mountpoint from path %s\n", path);
		return NULL;
	}

	/* Find the mounted filesystem with this mountpoint name */
	vnode* root_node = NULL;
	for (size_t i = 0; i < _curr_size; i++)
	{
		if (!_mountpoints[i])
			continue;

		vfs* _mp = _mountpoints[i];
		if (strcmp(_mp->vcovered->name, token) == 0)
		{
			root_node = _mp->vroot;
			LOG_DEBUG ("resolved mountpoint %s to fs %s\n",
						token, _mp->type->fs_name);
			break;
		}
	}

	if (!root_node) {
		LOG_ERROR ("no mounted filesystem found for mountpoint %s\n", token);
		return NULL;
	}

	/* Construct the relative path with leading '/' */
	/* Calculate where the relative path starts in the original path */
	const char* rel_start = path + 1 + strlen(token);  /* skip "/mountpoint" */

	/* If there's nothing after the mountpoint, use "/" as the relative path */
	if (*rel_start == '\0') {
		strncpy (rel_path, "/", rel_path_size);
	} else {
		/* rel_start already points to the '/' after mountpoint, so just copy it */
		strncpy (rel_path, rel_start, rel_path_size);
	}
	rel_path[rel_path_size - 1] = '\0';

	LOG_DEBUG ("relative path: %s\n", rel_path);

	return root_node;
}
