#ifndef _FAT12_H
#define _FAT12_H
/*******************************************************************************
 *
 *   @file		fat12.h
 *   @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
 *   @brief		Implements a FAT12 filesystem (classic DOS), which will be used
 *			  	to read and write files on FAT12 formatted disks (in our case,
 *              floppy disks).
 *   @version	0.1
 *
*******************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <fs/vfs.h>
#include <driver/block.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

/* File attributes masks (byte 11 in Directory entry) */

#define FAT12_RD 			0x01 // Read-Only
#define FAT12_HID 			0x02 // Hidden
#define FAT12_SYS 			0x04 // System
#define FAT12_VID 			0x08 // Volume ID
#define FAT12_DIR 			0x10 // Directory
#define FAT12_ARC 			0x20 // Archive

/* Mask to retrieve the cluster start from a 16bit value */

#define FAT12_CLUSTER_MASK 	0xFFF // 12 bits used in FAT12

#define FAT12_EOC_MIN		0xFF8 // minimum value for end of cluster marker
#define FAT12_EOC_MAX		0xFFF // maximum value for end of cluster marker
#define FAT12_EOC			0xFFF // end of cluster marker
#define FAT12_BAD_CLUSTER	0xFF7 // bad cluster marker
#define FAT12_FREE_CLUSTER	0x000 // free cluster marker

#define FAT12_DELETED		0xE5 // first byte of filename is 0xE5 if deleted
#define FAT12_ENDDIR		0x00 // first byte of filename is 0x00 if end of dir

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

/* The Bios Parameter Block BPB is generally the sector0 on the formatted 
	drive. It is also a part of the bootsector, hence why the jump instruction
	at the beginning to skip the data area. */

typedef struct {

	//! jump to boot code, disassembles to jmp short followed by a NOP (useless)
	uint8_t 		jump[3];

	//! OEM name (8 bytes)
	char 			oem_name[8];

	//! Bytes per sector (2 bytes)
	uint16_t 		bytes_per_sector;

	//! Sectors per cluster (1 byte)
	uint8_t 		sectors_per_cluster;

	//! Reserved sectors (2 bytes)
	uint16_t 		rsrvd_cnt;

	//! Number of File Allocation Tables (FATs) (1 byte)
	uint8_t 		num_fats;

	//! Maximum number of root directory entries (2 bytes)
	uint16_t 		max_root_dir_entries;

	//! Total sectors in the volume (2 bytes, 0 if more than 65535)
	uint16_t 		total_sectors;

	//! Media descriptor (1 byte)
	uint8_t 		media_descriptor;

	//! Sectors per FAT (2 bytes)
	uint16_t 		sectors_per_fat;

	//! Sectors per track (2 bytes)
	uint16_t 		sectors_per_track;

	//! Number of heads per cylinder (2 bytes)
	uint16_t 		num_heads;

	//! Hidden sectors (represents LBA of beginning of partition) (4 bytes)
	uint32_t 		hidden_sectors;

	//! Total sectors in the volume (4 bytes, if total_sectors is 0)
	uint32_t 		total_sectors_large;

} __attribute__((packed)) fat12_bpb_t;

/* The following fields are part of the extended boot record, added to 
	BPB for convenince only */

typedef struct {

	//! Physical drive number (1 byte)
	uint8_t 		drive_number;

	//! Reserved (1 byte)
	uint8_t 		reserved1;

	//! Extended boot signature (1 byte, usually 0x29)
	uint8_t 		ext_boot_sig;

	//! Volume ID (4 bytes) (can be ignored)
	uint32_t 		volume_id;

	//! Volume label/name padded with spaces (11 bytes) (can be ignored)
	char 			volume_label[11];

	//! File system type padded with spaces (8 bytes) (can be ignored)
	char 			file_system_type[8];

	//! Boot code (448 bytes, can be ignored)
	uint8_t 		boot_code[448];

	//! Boot sector signature (2 bytes, must be 0xAA55)
	uint8_t 		boot_signature[2];

} __attribute__((packed)) fat12_ebr_t;


/* Structure of a directory entry in FAT12. */

typedef struct {
	
	char 			filename[8];		//! 8.3 filename (padded with space)
	char 			extension[3];		//! file extension
	uint8_t 		attributes;			//! file attributes defined above
	uint8_t 		reserved;			//! reserved, can be ignored
	uint8_t 		create_time_tenth;	//! create time (1/10 sec resolution)
	uint16_t 		create_time;		//! create time
	uint16_t 		create_date;		//! create date
	uint16_t 		last_access_date;	//! last access date	
	uint16_t 		first_cluster_high; //! high 16 bits of first cluster (0)
	uint16_t 		last_write_time;	//! last write time
	uint16_t 		last_write_date;	//! last write date
	uint16_t 		first_cluster_low;	//! low 16 bits of first cluster
	uint32_t 		file_size;			//! file size in bytes

} __attribute__((packed)) fat12_dir_entry_t;


/* FAT12 handle is used to represent an instance of a mounted FAT12 fs. Since
	most of the values in the BPB are dumb, we can read it once and release
	the memory by keeping only the relevant values. */

typedef struct {
	
	fat12_bpb_t* 	bpb;				//! Pointer to the BPB structure
	fat12_ebr_t* 	ebr;				//! Pointer to the extended boot record
	uint32_t 		fat_start;			//! start of FAT
	uint32_t 		root_dir_start;		//! start of root directory
	uint32_t 		root_dir_sectors;	//! root directory sector count
	uint32_t 		data_area_start;	//! start of data area
	uint8_t* 		fat_table;			//! FAT table in memory
	block_device_t* block_dev;			//! block device handle

} fat12_handle_t;

/* FS specific data to represent a single node in the underlying filesys driver.
	This will be the `inode`. */

typedef struct _fat12_inode {

	fat12_dir_entry_t	dir_entry;		//! the directory entry on disk
	uint32_t			dir_sector;		//! sector containing this dir entry
	uint32_t			dir_offset;		//! offset within the sector

} inode;

//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

/* Filesystem specific operations (or vfs_ops_t) */

//! mounts a FAT12 filesystem from the given device, returning the handle
vfs*				fat12_mount (const char* device);

//! unmounts a mounted FAT12 filesystem instance and frees the handle
int32_t				fat12_unmount (vfs* fsys);


/* FS specific operations performed on files/vnodes */

//! open a file/directory at the given path, returning a vnode object
vnode*				fat12_open  (vnode* node, const char* path, uint32_t flags);

//! close the given vnode (file/directory), returning an error code
int32_t				fat12_close (vnode* node);

//! read from the given vnode (file), into the buffer, returning bytes read
int32_t				fat12_read  (vnode* node, uint32_t offs, 
								 uint32_t size, void* buf);

//! write to the given vnode (file), from the buffer, returning bytes written
int32_t				fat12_write (vnode* node, uint32_t offs, 
								 uint32_t size, void* buf);

//*****************************************************************************
//**
//** 	END [fat12.h]
//**
//*****************************************************************************
#endif // _FAT12_H