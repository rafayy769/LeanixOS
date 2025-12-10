#ifndef _IDE_H
#define _IDE_H
/*******************************************************************************
 *
 *   @file		ide.h
 *   @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
 *   @brief		Defines the driver for interfacing with ATA devices,
 *              such as hard drives and CD-ROMs, using the ATA protocol.
 *   			Currently supports ATA PIO mode only.
 *   @version	0.1
 *
*******************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

/* Since there can be two HDD controllers, we will only concern ourselves with
	the primary controller. */

#define IDE_PRIM_CMD_BASE 			0x1F0
#define IDE_PRIM_CTRL_BASE 			0x3F6
#define IDE_SEC_CMD_BASE 			0x170
#define IDE_SEC_CTRL_BASE 			0x376

/* Offsets for different registers */

// command block ports
#define OFFS_DATA					0x0 // data register (RW)
#define OFFS_ERROR					0x1 // error register (R)
#define OFFS_FEATURES				0x1 // features register (W)
#define OFFS_SECTCOUNT				0x2 // sector count register (RW)
#define OFFS_LBALO					0x3 // low byte of LBA (RW)
#define OFFS_LBAMID					0x4 // middle byte of LBA (RW)
#define OFFS_LBAHI					0x5 // high byte of LBA (RW)
#define OFFS_DEVICE					0x6 // device select register (RW) or LBA 24-27
#define OFFS_COMMAND				0x7 // command register (W)
#define OFFS_STATUS					0x7 // status register (R)

// controller block ports
#define OFFS_ALTSTATUS				0x0 // alt status register (R)
#define OFFS_DEVICECTRL				0x0 // device control register (W)

/* GET IDE controller registers, the base registers are stored in the struct */

#define IDE_REG_DATA(ctrl)		 	((ctrl)->command_base + OFFS_DATA)
#define IDE_REG_ERROR(ctrl) 	 	((ctrl)->command_base + OFFS_ERROR)
#define IDE_REG_FEATURES(ctrl)	 	((ctrl)->command_base + OFFS_FEATURES)
#define IDE_REG_SECTCOUNT(ctrl)  	((ctrl)->command_base + OFFS_SECTCOUNT)
#define IDE_REG_LBALO(ctrl)	 	 	((ctrl)->command_base + OFFS_LBALO)
#define IDE_REG_LBAMID(ctrl)	 	((ctrl)->command_base + OFFS_LBAMID)
#define IDE_REG_LBAHI(ctrl)		 	((ctrl)->command_base + OFFS_LBAHI)
#define IDE_REG_DEVICE(ctrl)	 	((ctrl)->command_base + OFFS_DEVICE)
#define IDE_REG_COMMAND(ctrl)	 	((ctrl)->command_base + OFFS_COMMAND)
#define IDE_REG_STATUS(ctrl)	 	((ctrl)->command_base + OFFS_STATUS)
#define IDE_REG_ALTSTATUS(ctrl)	 	((ctrl)->control_base + OFFS_ALTSTATUS)
#define IDE_REG_DEVICECTRL(ctrl) 	((ctrl)->control_base + OFFS_DEVICECTRL)

/* Error register bits definition. Some of these are however non-standard. */

#define IDE_ERR_AMNF 				0x01  // Address mark not found
#define IDE_ERR_TKZNF 				0x02  // Track 0 not found
#define IDE_ERR_ABRT 				0x04  // Command aborted
#define IDE_ERR_MCR 				0x08  // Media change request
#define IDE_ERR_IDNF 				0x10  // ID not found
#define IDE_ERR_MC 					0x20  // Media change
#define IDE_ERR_UNC 				0x40  // Uncorrectable data error
#define IDE_ERR_BBK 				0x80  // Bad block detected

/* Device register bits (since we'll only use LBA mode, we don't consider the 
	CHS mode bits definitons) */

#define IDE_DEV_LBA_BITMSK 			0x0F // bits 0-3: lba 24-27
#define IDE_DEV_DRIVE_SEL 			0x10 // bit 4 selects the drive (master(0)/slave(1))
#define IDE_DEV_LBA_MODE 			0x40 // bit 6 selects the LBA mode
#define IDE_DEV_DEFAULT 			0xA0 // bit 5 and 7 are always 1

/* Status register bits. Reading this register clears the interrupt condition */

#define IDE_STAT_ERR 				0x01 // error
#define IDE_STAT_IDX 				0x02 // index (unused)
#define IDE_STAT_CORR 				0x04 // data read corrected successfully (not used)
#define IDE_STAT_DRQ 				0x08 // buffer requires service (accept or send)
#define IDE_STAT_SCOM 				0x10 // seek complete
#define IDE_STAT_DF 				0x20 // drive fault (which does not set ERR)
#define IDE_STAT_RDY 				0x40 // drive ready
#define IDE_STAT_BSY 				0x80 // drive busy

/* Device control register bits. Generally this register should be 0. */

#define IDE_DEVCTRL_SRST 			0x04 // trigger software reset
#define IDE_DEVCTRL_NIEN 			0x02 // disable interrupts
#define IDE_DEVCTRL_HOB 			0x01 // read HOB of last LBA48 value
#define IDE_DEVCTRL_DEFAULT 		0x00 // default value for the register

/* Valid ATA commands. Only defining the ones that will be used for now. */

#define IDE_CMD_IDENTIFY 			0xEC // identify device
#define IDE_CMD_READ_SECTORS 		0x20 // read sectors in PIO mode
#define IDE_CMD_WRITE_SECTORS 		0x30 // write sectors in PIO mode

/* A single IDE controller has max two ATA devices attached to it. */

#define IDE_MAX_DEVICES 			0x2
#define IDE_DEV_MASTER 				0x0 // Master device
#define IDE_DEV_SLAVE 				0x1 // Slave device

/* The default sector size to use that should be good enough for almost all 
	purposes. */

#define IDE_SECTOR_SIZE 			512 // bytes

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

/* _ide_device represents a single ATA device attached to the IDE controller. */
typedef struct _ide_device ide_device_t;
typedef struct _ide_controller ide_controller_t;


/* IDE device struct represents a single IDE device, connected to an IDE 
	controller. Note that this device can be a HDD, or some other ATA device
	such as a CD controller. */

struct _ide_device {

	//! device model string
	char 		model[41];            // model string (40 bytes + \0)

	//! Whether the device is present or not
	bool 		present;

	//! If the device is a hard disk or a CD-ROM (uses ATAPI)
	bool 		is_hdd;

	//! hard disk characteristics
	uint32_t 	cylinders;  		  // number of cylinders
	uint32_t 	heads;      		  // number of heads
	uint32_t 	sectors_per_track;    // number of sectors per track
	uint32_t 	total_sectors;        // total number of sectors

	//! is master or slave device
	uint8_t 	device_num;       	  // device0, device1

	//! pointer to the controller this device is attached to
	struct _ide_controller* 	ctrl;

};

/* A single IDE controller has at most 2 devices connected, a slave and a 
	master device. Each controller gets its own IO space. */

struct _ide_controller {
	
	//! Boolean array of devices connected to the controller, to represent
	//! if the connected drive is present or not
	ide_device_t 	devices[ IDE_MAX_DEVICES ];

	//! Base I/O port for command registers
	uint16_t 		command_base;

	//! Base I/O port for control registers
	uint16_t 		control_base;

	//! A name for the controller, for debugging purposes
	char 			name[8];

};

//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

//! initializes the disk subsystem and connected controllers
void 	ide_init ();

//! resets the given IDE controller
void 	ide_reset (ide_controller_t* controller);

//! read a sector from the given drive and given sector number, assumes that the
//! buffer is at least SECTOR_SIZE bytes long
void 	ide_read_sector (void* drive, uint32_t sector, void* buffer);

//! write a sector to the given drive and given sector number, assumes buffer to
//! be SECTOR_SIZE bytes long
void 	ide_write_sector (void* drive, uint32_t sector, const void* buffer);

// TODO these functions will be added later to read/write multiple sectors


//*****************************************************************************
//**
//** 	END _[filename]
//**
//*****************************************************************************
#endif // _IDE_H