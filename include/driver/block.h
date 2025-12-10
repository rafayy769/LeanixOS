#ifndef _BLOCKDEV_H
#define _BLOCKDEV_H
//*****************************************************************************
//*
//*  @file		block.h
//*  @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief		Implements block device abstraction for the kernel. Block
//*             devices refer to devices that can read/write data in fixed-size
//*             blocks, such as hard drives, SSDs, and USB drives. Block device
//*             abstraction provides a uniform interface for reading and writing
//*             data to these devices, allowing the kernel to interact with them
//*             without needing to know the specifics of each device type.
//*  @version	0.1
//*
//****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stddef.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

/* We use the block/sector size of 512 as default for our devices, however,
	certain devices can just update it according to their specifics. */

#define DEFAULT_BLOCK_SIZE 		512

/* A type to represent a LOGICAL BLOCK ADDRESS number to use for block devices,
	this basically represents the sector number to use for block device
	operations. */

typedef uint32_t block_lba_t;

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

/* The struct represents functions interface for all the block devices to 
	comply with. Each device, when registered, also registers the internal
	functions to perform these actions. Polymorphism but in C. */

typedef struct _block_device_ops {

	//! function to read a single block from the block device
	int32_t (*read) (void* private, block_lba_t lba, void* buffer);

	//! function to write a single block to the block device
	int32_t (*write) (void* private, block_lba_t lba, const void* buffer);

	// TODO support for multiple sector read/write operations to be added later

} block_device_ops_t;

/* block_device_t represents a block device. Contains the useful information
	common to any block device, including the operations, and then a pointer
	to the block device driver specific information, e.g. IDE hard disks
	specific info or FDD data etc. */

typedef struct _block_device {

	//! The name of the block device
	const char* 		name;

	//! Size of a sector/block in bytes
	size_t 				block_size;

	//! The total number of blocks in the block device (if available)
	size_t 				num_blocks;

	//! Private data specific to the driver (set at the time of registration)
	void* 				driver_private;

	//! The operations interface for the block device
	const block_device_ops_t* ops;

} block_device_t;

//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

//! Function to register a block device with the kernel
int32_t 		blkdev_register (const char* name, size_t block_size,
								size_t num_blocks, const block_device_ops_t* ops,
								void* driver_private);

//! Get a block device by its name
block_device_t* blkdev_get_by_name (const char *name);

//! query the block size, and number of blocks of a block device
size_t 			blkdev_get_block_size (block_device_t* dev);
size_t 			blkdev_get_num_blocks (block_device_t* dev);

//! Exposed functions to easily read or write block(s)
int32_t 		blkread  (block_device_t* dev, block_lba_t lba, void* buffer);
int32_t 		blkwrite (block_device_t* dev, block_lba_t lba, 
						  const void* buffer);

//*****************************************************************************
//**
//** 	END _[filename]
//**
//*****************************************************************************

#endif /* _BLOCKDEV_H */