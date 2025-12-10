#ifndef _VFS_H
#define _VFS_H
//*****************************************************************************
//*
//*  @file		[vfs.h]
//*  @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief		
//*  @version	
//*
//****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <kernel/list.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

typedef enum vnode_type {

	V_FILE,      	// Regular file
	V_DIRECTORY, 	// Directory
	V_CHARDEVICE,	// Character device (e.g., /dev/tty)
	V_BLOCKDEVICE,	// Block device (e.g., hdds, fdc)
	V_PIPE,      	// Pipe
	V_SOCKET,    	// Socket
	V_SYMLINK,   	// Symbolic link

} vnode_type_t;

/* forward declaration of necessary types */

//! represents filesystem operations interface
typedef struct _vfs_ops 	vfs_ops_t;

//! represents a mounted filesystem instance
typedef struct _vfs_fs 		vfs;

//! represents a node in the VFS structure
typedef struct _vfs_node 	vnode;

//! represents vnode operations interface
typedef struct _vnode_ops 	vnode_ops_t;


//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------


/* Each mounted filesystem supports mount and unmount functions which allows the
	kernel to mount/unmount filesystems in a uniform manner. These functions
	are then implemented by the filesystem driver. */

struct _vfs_ops {

	//! mount the filesystem from the specified device, returns the ptr to the 
	//! created object
	vfs* 			(*mount) (const char* device);

	//! unmount the filesystem and free the vfs instance, returning err code
	int32_t 		(*unmount) (vfs* filesystem);

};

/* A generic filesystem type:
	Each filesystem type supported by system registers an instance of this type
	which is then used by kernel to generate a table of supported filesystems.
	 */

typedef struct {

	char*			fs_name;	//! name of the filesystem (e.g., "fat12")
	vfs_ops_t		vfs_ops;	//! vfs operations for this filesystem

} fs_type_t;

/* The struct represents a mounted filesystem instance. The kernel should only
	access this instance and the interface functions specified here to be able
	to deal with pretty much all the FSs we will ever need. Each mounted FS is
	put in a list of mounted FSs. */

struct _vfs_fs {

	fs_type_t* 			type; 		//! the type of filesystem (e.g., fat12)
	vnode* 				vcovered; 	//! the vnode that this fs covers
	vnode*				vroot;		//! root vnode of the fs
	void* 		   	   	fs_data; 	//! private data for the filesystem driver

};

/* vnode represents a node in the VFS structure. A VFS is intended to abstract
	the details of the filesystem and the location of the files/devices it 
	represents. Hence, a uniform access API is provided to read/write or perform
	other operations. Implemented as a graph of vnodes, each vnode in the VFS
	can represent a file, directory, symlinks, pipes or IO devices etc. */

struct _vfs_node {

	char		 	name[28];			//! name of the vnode (file/dir name)
	vfs* 			vfs_mountedhere;	//! ptr to the mounted FS at this vnode
	vfs*		 	vfs_ptr;			//! ptr to the VFS this vnode belongs to
	vnode_type_t	type;				//! type of the vnode
	vnode_ops_t* 	ops;				//! vnode operations
	uint32_t 		flags;				//! flags for the vnode (RWX etc.)
	void*		 	data;				//! private vnode data (FS specific)
	
};

/* Each compliant FS driver should support the following operations on vnodes.
	Note that all the possible vnode operations that can be performed by each
	type of vnodes are stored here. Either the functions themselves should
	check the vnode type, or the vfs interface itself. */

struct _vnode_ops {

	/* the first 4 are regular file operations, from which we can read write
		data etc. */

	//! open the vnode (file/device), return an opened vnode object
	vnode* 		(*open)  (vnode* node, const char* path, uint32_t flags);

	//! close the vnode (file/device), returns an error code
	int32_t 	(*close) (vnode* node);

	//! read from the vnode (file/device)
	int32_t 	(*read)  (vnode* node, uint32_t offs, uint32_t size, void* buf);

	//! write to the vnode (file/device)
	int32_t 	(*write) (vnode* node, uint32_t offs, uint32_t size, void* buf);

	/* some directory related functions. */

	//! read directory entries from the directory vnode into a NULL terminated
	//! array of vnode entries
	int32_t 	(*readdir) (vnode* node, vnode** dirents, uint32_t* count);

	//! create a new file/directory entry. passed node is the root vnode
	int32_t 	(*mkdir)  (vnode* node, const char* path);

	//! remove a file/directory entry. passed node is the root vnode
	int32_t 	(*remove) (vnode* node, const char* path);

	//! create a file on the system
	int32_t 	(*create) (vnode* node, const char* path);

};


/* Represents an opened regular file entry. */

typedef struct _file {

	vnode*			vnode_ptr;			//! ptr to the vnode for this file
	uint32_t		f_offset;			//! current offset in the file
	uint32_t		f_flags;			//! flags for this file (RWX etc.)

} file_t;


//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

//! initialize the VFS layer
void 		vfs_init ();

//! mounts a filesystem at the specified path, from the specified device
int32_t 	vfs_mount (const char* src_dev, const char* mount_path, const char* fs_type);

//! unmounts a filesystem mounted at the specified path
int32_t 	vfs_unmount (const char* mount_path);

//! create a new file at the specified path
int32_t 	vfs_create (const char* path, uint32_t flags);

//! create a new directory at the specified path
int32_t 	vfs_mkdir (const char* path);

//! remove a file/directory at the specified path
int32_t 	vfs_remove (const char* path);

//! opens a file at the specified path with the given flags
file_t* 	vfs_open (const char* path, uint32_t flags);

//! closes a previously opened file
int32_t 	vfs_close (file_t* file);

//! reads from a file into a buffer
int32_t 	vfs_read (file_t* file, void* buf, uint32_t size);

//! writes to a file from a buffer
int32_t 	vfs_write (file_t* file, void* buf, uint32_t size);

//! get a mounted filesystem by mount path
vfs* 		vfs_get_mounted (const char* mount_path);

//*****************************************************************************
//**
//** 	END _[filename]
//**
//*****************************************************************************
#endif /* _VFS_H */