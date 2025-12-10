#ifndef _HFS_H
#define _HFS_H
/*******************************************************************************
 *
 *   @file		hfs.h
 *   @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
 *   @brief		Implements an inode based simple filesystem with support for files
 *              and directories.
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

#define BLOCK_SIZE 						  512
#define INODE_SIZE 						  (sizeof(struct inode))

#define INODES_PER_BLOCK 				  (BLOCK_SIZE / INODE_SIZE)
#define INODE_DIRECT_POINTERS 			  13
#define INODE_INDIRECT_POINTERS_PER_BLOCK (BLOCK_SIZE / sizeof(uint16_t))

#define DIRECTORY_ENTRY_SIZE 			  ((sizeof(struct directory_entry)))
#define DIRECTORY_NAME_SIZE 			  28
#define DIRECTORY_DEPTH_LIMIT 			  10
#define DIRECTORY_ENTRIES_PER_BLOCK 	  (BLOCK_SIZE / DIRECTORY_ENTRY_SIZE)

#define FLAGS_PER_BLOCK 				  (BLOCK_SIZE / sizeof(uint32_t))

#define HFS_MAGIC 						  0x48465331  // "HFS1" in hex

typedef uint32_t off_t;


//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

/* the first block contains the metadata about the filesystem */
struct superblock {

    uint32_t s_blocks_count;             // totoal blocks
    uint32_t s_inodes_count;             // total inodes
    uint32_t s_block_bitmap;             // block bitmap start block
    uint32_t s_inode_bitmap;             // inode bitmap start block
    uint32_t s_inode_table_block_start;  // inode table start block
    uint32_t s_data_blocks_start;        // data blocks start block
    uint32_t s_magic;                    // magic number to test integrity

};

/* an inode entry in the inodes table */
struct inode {

    uint32_t i_size;
    uint32_t i_is_directory;

    /* first direct pointers are used. they point to data blocks */
    uint32_t i_direct_pointers[INODE_DIRECT_POINTERS];

    /* a single indirect pointer points to a block, that contains all pointers
        to other data blocks */
    uint32_t i_single_indirect_pointer;

};

/* directory entry present in a directory block */
struct directory_entry {

    uint32_t inode_number;
    char name[DIRECTORY_NAME_SIZE];

};

/* a directory block contains a bunch of directory entries */
struct directory_block {

    struct directory_entry entries[DIRECTORY_ENTRIES_PER_BLOCK];

};

/* represents a block/sector on the disk. a given block on the disk can be one
    of the following */
union block
{
    struct superblock superblock;                         // Superblock
    struct inode inodes[INODES_PER_BLOCK];                // Inode block
    uint32_t bitmap[FLAGS_PER_BLOCK];       			  // Bitmap block (inode or data)
    struct directory_block directory_block;               // Directory block
    uint8_t data[BLOCK_SIZE];                             // Data block
    uint16_t pointers[INODE_INDIRECT_POINTERS_PER_BLOCK]; // Indirect pointer block
};

//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------


/* filesys specific operations */

int 	    hfs_format (const char* device);
vfs* 	    hfs_mount (const char* device);
int32_t     hfs_unmount (vfs* fsys);

/* Forward declarations of vnode operations. note that the node parameter
	in all the vnode functions will be the root vnode, as called by the VFS
	layer */

/* root vnode is passed in these functions */
int32_t 	hfs_create (vnode* node, const char* path);
int32_t 	hfs_mkdir (vnode* node, const char* path);
int32_t 	hfs_remove (vnode* node, const char* path);
vnode* 	    hfs_open (vnode* node, const char* path, uint32_t flags);

/* the actual vnode is passed, which was retrieved by hfs_open */
int32_t 	hfs_close (vnode* node);
int32_t 	hfs_read (vnode* node, uint32_t offs, uint32_t size, void* buf);
int32_t 	hfs_write (vnode* node, uint32_t offs, uint32_t size, void* buf);

/* debug helpers to list and inspect filesystem */

int32_t     fs_list (vfs* fsys, const char* path);
int32_t     fs_stat_file (vfs* fsys, const char* path);
void 	    fs_stat (vfs* fsys);

//*****************************************************************************
//**
//** 	END [hfs.h]
//**
//*****************************************************************************
#endif // _HFS_H