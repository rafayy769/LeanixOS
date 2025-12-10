#include <driver/block.h>
#include <mm/kheap.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define LOG_MOD_NAME 	"BLK"
#define LOG_MOD_ENABLE  0
#include <log.h>

/* Implementation private data */

//! an array of registered block devices (pointers to block_device_t)
static block_device_t** block_devices = NULL;

//! number of block devices registered in the system
static size_t 		   	num_block_devices = 0;
static size_t 		   	max_block_devices = 16;

/* Implementation of public facing functions */

int32_t blkdev_register (const char* name, size_t block_size, size_t num_blocks,
						 const block_device_ops_t* ops, void* driver_private)
{
	if (!block_devices)
	{
		num_block_devices = 0;
		max_block_devices = 16;
		block_devices     = malloc (sizeof(block_device_t*) * 
									max_block_devices);
		if (!block_devices) {
			LOG_ERROR ("blkdev_register: malloc failed for block devices arr");
			return -1;
		}

		for (size_t i = 0; i < max_block_devices; i++)
		{
			block_devices[i] = NULL;
		}
	}

	// gotta make sure we have enough space
	if (num_block_devices >= max_block_devices) {
		// TODO realloc logic will be added later
		LOG_ERROR ("blkdev_register: Maximum number of block devices reached");
		return -1;
	}

	block_device_t* dev = malloc (sizeof(block_device_t));
	if (!dev) {
		LOG_ERROR ("blkdev_register: malloc failed for block device struct");
		return -1;
	}
	
	for (size_t i = 0; i < num_block_devices; i++) {

		if (block_devices[i] && 
			strcmp (block_devices[i]->name, name) == 0)  {

			LOG_ERROR ("blkdev_register: Device with name '%s' already exists",
					  name);
			return -1;
		
		}

	}

	dev->name = name;
	dev->block_size = block_size;
	dev->num_blocks = num_blocks;
	dev->driver_private = driver_private;
	dev->ops = ops;
	block_devices[ num_block_devices++ ] = dev;

	LOG_P ("Registered block device '%s': %u blocks, %u blk_size\n",
			name, num_blocks, block_size);

	return 0;

}

block_device_t* blkdev_get_by_name (const char *name) {

	if (!name) {
		LOG_ERROR ("blkdev_get_by_name: NULL name provided");
		return NULL; // Invalid name
	}

	for (size_t i = 0; i < num_block_devices; i++) {

		if (block_devices[i] && 
			strcmp (block_devices[i]->name, name) == 0)
		{
			return block_devices[i]; // Found the device
		}

	}

	LOG_ERROR ("blkdev_get_by_name: Device with name '%s' not found", name);
	return NULL; // Device not found

}

size_t 	blkdev_get_block_size (block_device_t* dev) {

	if (!dev) {
		LOG_ERROR ("blkdev_get_block_size: NULL device provided");
		return 0; // Invalid device
	}
	return dev->block_size;

}

size_t 	blkdev_get_num_blocks (block_device_t* dev) {
	
	if (!dev) {
		LOG_ERROR ("blkdev_get_num_blocks: NULL device provided");
		return 0; // Invalid device
	}
	return dev->num_blocks;
	
}

int32_t blkread (block_device_t* dev, block_lba_t lba, void* buffer) {

	if (!dev || !dev->ops || !dev->ops->read) {
		LOG_ERROR ("blkread: Invalid block device or read operation not defined");
		return -1; // Invalid device or read operation not defined
	}

	if (lba >= dev->num_blocks) {
		LOG_ERROR ("blkread: LBA %u out of range for device '%s' with %u blocks",
				   lba, dev->name, dev->num_blocks);
		return -1; // LBA out of range
	}

	return dev->ops->read (dev->driver_private, lba, buffer);

}

int32_t blkwrite (block_device_t* dev, block_lba_t lba, const void* buffer) {

	if (!dev || !dev->ops || !dev->ops->write) {
		LOG_ERROR ("blkwrite: Invalid block device or write operation not defined");
		return -1; // Invalid device or write operation not defined
	}

	if (lba >= dev->num_blocks) {
		LOG_ERROR ("blkwrite: LBA %u out of range for device '%s' with %u blocks",
				   lba, dev->name, dev->num_blocks);
		return -1; // LBA out of range
	}

	return dev->ops->write (dev->driver_private, lba, buffer);

}