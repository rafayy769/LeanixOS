#include <stdbool.h>
#include <driver/ide.h>
#include <driver/block.h>
#include <interrupts.h>
#include <driver/timer.h>
#include <utils.h>
#include <stdint.h>
#include <string.h>

#define LOG_MOD_NAME 	"IDE"
#define LOG_MOD_ENABLE  0
#include <log.h>

/* Currently only the primary controller is supported. */
static ide_controller_t 	_ide_prim;

/* Currently selected drive to be used. */
static ide_device_t* 		_ide_current_dev;

/* Forward declarations for block device operations */
static int32_t _ide_blk_read (void* private, block_lba_t lba, void* buffer);
static int32_t _ide_blk_write (void* private, block_lba_t lba, const void* buffer);

/* Block device operations structure */
static const block_device_ops_t _ide_block_device_ops = {
	.read  = _ide_blk_read,
	.write = _ide_blk_write,
};

/* Private helper routines */

//! identify a device
static void 	_ide_device_identify (ide_device_t* dev);

//! check device type (master/slave)
static void 	_ide_check_type (ide_device_t* dev);


/* - The status register value is valid only if BSY bit is 0. Also clears any
	pending interrupt, by clearing the interrupt condition.
   - The contents of the error register are valid only when BSY and DRQ are 0, 
   	and the ERR bit in the status is set to 1. 
   - The data register is 16 bit and is used for transferring data in PIO. */

//! read the useful registers
static inline uint8_t 	_ide_read_status (ide_device_t* dev); // clears intr
static inline uint8_t 	_ide_read_error (ide_device_t* dev);
static inline uint16_t 	_ide_read_data (ide_device_t* dev);

//! write values to the device registers
static inline void 	_ide_write_lbaregs (ide_device_t* dev, uint32_t lba);
static inline void 	_ide_write_sectcount (ide_device_t* dev, uint8_t count);
static void 		_ide_write_command (ide_device_t* dev, uint8_t command);
static void 		_ide_select_drive (ide_device_t* dev); // writes devsel reg

//! wait for the device to be idle (with timeout)
static int 	_ide_wait_idle (ide_device_t* dev);

/* DRDY bit is set to 1 when Disk is ready to accept commands */
static int 	_ide_wait_drdy (ide_device_t* dev);

/* BSY bit is set to 1 whenever device is busy performing a command. */
static int 	_ide_wait_bsy (ide_device_t* dev);

/* DRQ bit is set to 1 when device is ready to transfer data */
static int	_ide_wait_drq (ide_device_t* dev);

//! the interrupt handler for IDE controllers
static void 	_ide_intr_handler (interrupt_context_t* context);

//! Interrupt handling routine simply acknowledges the interrupt
static void _ide_intr_handler (interrupt_context_t* context) {

	// simply read the status register to clear the interrupt condition
	uint8_t status = _ide_read_status (_ide_current_dev);
	LOG_DEBUG ("IDE interrupt handler called, status=0x%02X\n", status);

}

void ide_reset (ide_controller_t* controller) {

	// Reset the controller (implicitly selects the master device)
	outb (IDE_DEVCTRL_SRST, IDE_REG_DEVICECTRL (controller));
	outb (IDE_DEVCTRL_DEFAULT, IDE_REG_DEVICECTRL (controller));

	// Set the device to default state
	outb (IDE_DEV_DEFAULT, IDE_REG_DEVICE (controller));

}

void ide_init() {

	// Initialize controller structure
	_ide_prim.command_base = IDE_PRIM_CMD_BASE;
	_ide_prim.control_base = IDE_PRIM_CTRL_BASE;
	strncpy (_ide_prim.name, "ide0", 8);

	LOG_DEBUG ("Initializing IDE controller %s at ports 0x%04x and 0x%04x\n",
				_ide_prim.name, _ide_prim.command_base, _ide_prim.control_base);

	// Reset the IDE controller
	LOG_DEBUG ("Resetting IDE controller...\n");
	ide_reset (&_ide_prim);

	// Small delay after reset
	for (volatile int i = 0; i < 10000; i++);

	// Register interrupt handler for IDE controller
	register_interrupt_handler (IRQ14_HDC, _ide_intr_handler);

	/* Set default current device to master
	   (we'll primarily use the master device hd0, but initialization
	   code works for both master and slave) */
	_ide_current_dev = &_ide_prim.devices[0];

	// Detect and identify all connected devices
	for (int i = 0; i < IDE_MAX_DEVICES; i++) {

		ide_device_t* dev = &_ide_prim.devices[i];
		dev->ctrl = &_ide_prim;
		dev->device_num = i;
		dev->present = false; // assume not present initially

		LOG_DEBUG ("Checking for device hd%d...\n", i);

		// Select the drive
		_ide_select_drive (dev);

		// Small delay after selecting
		for (volatile int j = 0; j < 1000; j++);

		// Check device type by reading LBA registers
		_ide_check_type (dev);

		// Try to identify the device
		_ide_device_identify (dev);

		// Report status
		if (dev->present) {
			LOG_P ("IDE: Found hd%d - %s (%s, %u sectors)\n",
				   i,
				   dev->model[0] ? dev->model : "Unknown Model",
				   dev->is_hdd ? "HDD" : "ATAPI",
				   dev->total_sectors);

			// Register HDDs as block devices
			if (dev->is_hdd && dev->total_sectors > 0) {
				// Create device name (hd0, hd1, etc.)
				const char* dev_name = (i == 0) ? "hd0" : "hd1";

				// Register the block device
				int ret = blkdev_register (dev_name, IDE_SECTOR_SIZE,
										   dev->total_sectors,
										   &_ide_block_device_ops, dev);

				if (ret == 0) {
					LOG_P ("  Registered %s as block device "
						   "(%u sectors x 512 bytes)\n",
						   dev_name, dev->total_sectors);
				} else {
					LOG_ERROR ("  Failed to register %s as block device\n",
							   dev_name);
				}
			}
		} else {
			LOG_DEBUG ("Device hd%d not present\n", i);
		}
	}

	LOG_P ("IDE initialization complete\n");
}


/* Private functions implementation */

static inline uint8_t _ide_read_status (ide_device_t* dev) {
	return inb (IDE_REG_STATUS(dev->ctrl));
}

static inline uint8_t _ide_read_error (ide_device_t* dev) {
	return inb (IDE_REG_ERROR(dev->ctrl));
}

static inline uint16_t _ide_read_data (ide_device_t* dev) {
	return inw (IDE_REG_DATA(dev->ctrl)); // 16 bit data register
}

static inline void _ide_select_drive (ide_device_t* dev) {

	/* setup the default values and the drive select bit */

	uint8_t device_reg = IDE_DEV_LBA_MODE | IDE_DEV_DEFAULT | 
						((dev->device_num) ? IDE_DEV_DRIVE_SEL : 0);
	outb (device_reg, IDE_REG_DEVICE (dev->ctrl));

	/* generally a 400ns delay is required after selecting the drive 
		we can emulate this with 4 reads from the alt status register */

	for (int i = 0; i < 4; i++) {
		inb (IDE_REG_ALTSTATUS (dev->ctrl));
	}

}

static inline void _ide_write_lbaregs (ide_device_t* dev, uint32_t lba) {

	/* write the LBA registers, the LBA is 28 bits long, so we need to
		write it in 3 parts */

	outb ((lba & 0xFF), IDE_REG_LBALO(dev->ctrl));
	outb (((lba >> 8) & 0xFF), IDE_REG_LBAMID(dev->ctrl));
	outb (((lba >> 16) & 0xFF), IDE_REG_LBAHI(dev->ctrl));

}

static inline void _ide_write_sectcount (ide_device_t* dev, uint8_t count) {
	outb (count, IDE_REG_SECTCOUNT(dev->ctrl));
}

static void _ide_write_command (ide_device_t* dev, uint8_t command) {
	outb (command, IDE_REG_COMMAND(dev->ctrl));
}

//! wait functions with timeout, return 0 on success, -1 on timeout
static int _ide_wait_idle (ide_device_t* dev) {

	// wait for both BSY and DRQ to be 0
	for (int i = 0; i < 100000; i++) {
		if (!(_ide_read_status (dev) & (IDE_STAT_BSY | IDE_STAT_DRQ)))
			return 0;
	}
	return -1; // timeout

}

static int _ide_wait_drq (ide_device_t* dev) {

	// wait for DRQ to be 1
	for (int i = 0; i < 100000; i++) {
		if (_ide_read_status (dev) & IDE_STAT_DRQ)
			return 0;
	}
	return -1; // timeout

}

static int _ide_wait_bsy (ide_device_t* dev) {

	// wait for BSY to be 0
	for (int i = 0; i < 100000; i++) {
		if (!(_ide_read_status (dev) & IDE_STAT_BSY))
			return 0;
	}
	return -1; // timeout

}

static int _ide_wait_drdy (ide_device_t* dev) {

	// wait for DRDY to be 1 and BSY to be 0
	for (int i = 0; i < 100000; i++) {
		uint8_t status = _ide_read_status (dev);
		if (!(status & IDE_STAT_BSY) && (status & IDE_STAT_RDY))
			return 0;
	}
	return -1; // timeout

}

static void _ide_check_type (ide_device_t* dev) {

	/* After selecting a device, we can check the LBA mid and high registers
	   to determine device type:
	   - 0x00, 0x00 = ATA device (HDD)
	   - 0x14, 0xEB = ATAPI device (CD-ROM, etc.)
	   - 0x3C, 0xC3 = SATA device (rare in this context)
	*/

	uint8_t lba_mid = inb (IDE_REG_LBAMID(dev->ctrl));
	uint8_t lba_hi  = inb (IDE_REG_LBAHI(dev->ctrl));

	if (lba_mid == 0x00 && lba_hi == 0x00) {
		dev->is_hdd = true;
		LOG_DEBUG ("Device hd%d is ATA (HDD)\n", dev->device_num);
	}
	else if (lba_mid == 0x14 && lba_hi == 0xEB) {
		dev->is_hdd = false;
		LOG_DEBUG ("Device hd%d is ATAPI (CD-ROM)\n", dev->device_num);
	}
	else {
		dev->is_hdd = false;
		LOG_DEBUG ("Device hd%d has unknown type (0x%02X, 0x%02X)\n",
				   dev->device_num, lba_mid, lba_hi);
	}

}

static void _ide_device_identify (ide_device_t* dev) {

	/* The IDENTIFY command returns 256 words (512 bytes) of device info.
	   We need to read this data and parse relevant fields. */

	uint16_t identify_data[256];

	// Select the drive first
	_ide_select_drive (dev);

	// Wait for drive to be ready (with timeout)
	if (_ide_wait_drdy (dev) != 0) {
		LOG_DEBUG ("Device hd%d timeout waiting for DRDY\n", dev->device_num);
		dev->present = false;
		return;
	}

	// Send IDENTIFY command
	_ide_write_command (dev, IDE_CMD_IDENTIFY);

	// Wait for BSY to clear (with timeout)
	if (_ide_wait_bsy (dev) != 0) {
		LOG_DEBUG ("Device hd%d timeout waiting for BSY clear\n",
				   dev->device_num);
		dev->present = false;
		return;
	}

	// Check status
	uint8_t status = _ide_read_status (dev);

	if (status == 0) {
		LOG_DEBUG ("Device hd%d does not exist (status=0)\n", dev->device_num);
		dev->present = false;
		return;
	}

	// Check for errors
	if (status & IDE_STAT_ERR) {
		LOG_DEBUG ("Device hd%d returned error on IDENTIFY\n",
				   dev->device_num);
		dev->present = false;
		return;
	}

	// Wait for DRQ (data ready) with timeout
	if (_ide_wait_drq (dev) != 0) {
		LOG_DEBUG ("Device hd%d timeout waiting for DRQ\n", dev->device_num);
		dev->present = false;
		return;
	}

	// Read 256 words of identify data
	for (int i = 0; i < 256; i++) {
		identify_data[i] = _ide_read_data (dev);
	}

	// Parse the identify data

	// Word 0: General configuration
	uint16_t gen_config = identify_data[0];
	if (gen_config & 0x8000) {
		// Bit 15 set = ATAPI device
		dev->is_hdd = false;
		LOG_DEBUG ("Device hd%d is ATAPI (from IDENTIFY)\n",
				   dev->device_num);
	} else {
		dev->is_hdd = true;
	}

	// Words 27-46: Model string (40 bytes, word-swapped)
	for (int i = 0; i < 20; i++) {
		dev->model[i * 2]     = (identify_data[27 + i] >> 8) & 0xFF;
		dev->model[i * 2 + 1] = identify_data[27 + i] & 0xFF;
	}
	dev->model[40] = '\0';

	// Trim trailing spaces from model string
	for (int i = 39; i >= 0; i--) {
		if (dev->model[i] == ' ') {
			dev->model[i] = '\0';
		} else {
			break;
		}
	}

	// Words 60-61: Total addressable sectors in LBA28 mode (32-bit value)
	dev->total_sectors = ((uint32_t)identify_data[61] << 16) |
						 identify_data[60];

	// CHS geometry (mostly obsolete but still present)
	dev->cylinders        = identify_data[1];
	dev->heads            = identify_data[3];
	dev->sectors_per_track = identify_data[6];

	LOG_DEBUG ("Device hd%d identified:\n", dev->device_num);
	LOG_DEBUG ("  Model: %s\n", dev->model);
	LOG_DEBUG ("  Total sectors: %u\n", dev->total_sectors);
	LOG_DEBUG ("  CHS: %u/%u/%u\n", dev->cylinders, dev->heads,
			   dev->sectors_per_track);

	// Mark device as present
	dev->present = true;

}

/* Public API implementations */

void ide_read_sector (void* drive, uint32_t sector, void* buffer) {

	ide_device_t* dev = (ide_device_t*)drive;

	if (!dev || !dev->present) {
		LOG_ERROR ("ide_read_sector: Invalid or non-present device\n");
		return;
	}

	if (!dev->is_hdd) {
		LOG_ERROR ("ide_read_sector: Device is not a hard disk\n");
		return;
	}

	if (sector >= dev->total_sectors) {
		LOG_ERROR ("ide_read_sector: Sector %u out of range (max: %u)\n",
				   sector, dev->total_sectors);
		return;
	}

	LOG_DEBUG ("Reading sector %u from hd%d\n", sector, dev->device_num);

	// Select the drive
	_ide_select_drive (dev);

	// Wait for drive to be ready
	if (_ide_wait_drdy (dev) != 0) {
		LOG_ERROR ("ide_read_sector: Timeout waiting for drive ready\n");
		return;
	}

	// Set sector count to 1
	_ide_write_sectcount (dev, 1);

	// Write LBA address (lower 24 bits)
	_ide_write_lbaregs (dev, sector);

	// Write upper 4 bits of LBA to device register
	// The device register already has LBA mode and drive select bits set
	uint8_t device_reg = IDE_DEV_LBA_MODE | IDE_DEV_DEFAULT |
						((dev->device_num) ? IDE_DEV_DRIVE_SEL : 0) |
						((sector >> 24) & 0x0F);
	outb (device_reg, IDE_REG_DEVICE(dev->ctrl));

	// Send READ_SECTORS command
	_ide_write_command (dev, IDE_CMD_READ_SECTORS);

	// Wait for BSY to clear
	if (_ide_wait_bsy (dev) != 0) {
		LOG_ERROR ("ide_read_sector: Timeout waiting for BSY clear\n");
		return;
	}

	// Check for errors
	uint8_t status = _ide_read_status (dev);
	if (status & IDE_STAT_ERR) {
		uint8_t error = _ide_read_error (dev);
		LOG_ERROR ("ide_read_sector: Error reading sector (status=0x%02X, error=0x%02X)\n",
				   status, error);
		return;
	}

	// Wait for DRQ (data ready)
	if (_ide_wait_drq (dev) != 0) {
		LOG_ERROR ("ide_read_sector: Timeout waiting for DRQ\n");
		return;
	}

	// Read 256 words (512 bytes) from data register
	uint16_t* buf = (uint16_t*)buffer;
	for (int i = 0; i < 256; i++) {
		buf[i] = _ide_read_data (dev);
	}

	LOG_DEBUG ("Successfully read sector %u\n", sector);

}

void ide_write_sector (void* drive, uint32_t sector, const void* buffer) {

	ide_device_t* dev = (ide_device_t*)drive;

	if (!dev || !dev->present) {
		LOG_ERROR ("ide_write_sector: Invalid or non-present device\n");
		return;
	}

	if (!dev->is_hdd) {
		LOG_ERROR ("ide_write_sector: Device is not a hard disk\n");
		return;
	}

	if (sector >= dev->total_sectors) {
		LOG_ERROR ("ide_write_sector: Sector %u out of range (max: %u)\n",
				   sector, dev->total_sectors);
		return;
	}

	LOG_DEBUG ("Writing sector %u to hd%d\n", sector, dev->device_num);

	// Select the drive
	_ide_select_drive (dev);

	// Wait for drive to be ready
	if (_ide_wait_drdy (dev) != 0) {
		LOG_ERROR ("ide_write_sector: Timeout waiting for drive ready\n");
		return;
	}

	// Set sector count to 1
	_ide_write_sectcount (dev, 1);

	// Write LBA address (lower 24 bits)
	_ide_write_lbaregs (dev, sector);

	// Write upper 4 bits of LBA to device register
	uint8_t device_reg = IDE_DEV_LBA_MODE | IDE_DEV_DEFAULT |
						((dev->device_num) ? IDE_DEV_DRIVE_SEL : 0) |
						((sector >> 24) & 0x0F);
	outb (device_reg, IDE_REG_DEVICE(dev->ctrl));

	// Send WRITE_SECTORS command
	_ide_write_command (dev, IDE_CMD_WRITE_SECTORS);

	// Wait for DRQ (ready to accept data)
	if (_ide_wait_drq (dev) != 0) {
		LOG_ERROR ("ide_write_sector: Timeout waiting for DRQ\n");
		return;
	}

	// Write 256 words (512 bytes) to data register
	const uint16_t* buf = (const uint16_t*)buffer;
	for (int i = 0; i < 256; i++) {
		outw (buf[i], IDE_REG_DATA(dev->ctrl));
	}

	// Wait for write to complete
	if (_ide_wait_bsy (dev) != 0) {
		LOG_ERROR ("ide_write_sector: Timeout waiting for write completion\n");
		return;
	}

	// Check for errors
	uint8_t status = _ide_read_status (dev);
	if (status & IDE_STAT_ERR) {
		uint8_t error = _ide_read_error (dev);
		LOG_ERROR ("ide_write_sector: Error writing sector (status=0x%02X, error=0x%02X)\n",
				   status, error);
		return;
	}

	LOG_DEBUG ("Successfully wrote sector %u\n", sector);

}

/* Block device operations for IDE drives */

static int32_t _ide_blk_read (void* private, block_lba_t lba, void* buffer) {
	ide_device_t* dev = (ide_device_t*)private;
	ide_read_sector (dev, lba, buffer);
	return 0; // TODO: proper error handling
}

static int32_t _ide_blk_write (void* private, block_lba_t lba, const void* buffer) {
	ide_device_t* dev = (ide_device_t*)private;
	ide_write_sector (dev, lba, buffer);
	return 0; // TODO: proper error handling
}