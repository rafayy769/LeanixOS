#ifndef _FDC_H
#define _FDC_H
//*****************************************************************************
//*
//*  @file		fdc.h
//*  @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief		Defines the interface for controlling the intel 8272A Floppy 
//*             Disk Controller (FDC).
//*  @version	0.1
//*
//****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

//! There can be multiple FDCs, but we will only support one for now

#define FDC_PRIMARY_BASE 	0x3F0
#define FDC_SECONDARY_BASE 	0x370

/* FDC is programmed through 9 registers which can be accessed through IO ports.
	The following constants define the offsets for these registers, which added
	to the base address will give the actual port address. */

#define OFFS_SRA 		0x0
#define OFFS_SRB 		0x1
#define OFFS_DOR 		0x2
#define OFFS_TDR 		0x3
#define OFFS_MSR 		0x4
#define OFFS_DRSR 		0x4
#define OFFS_FIFO 		0x5
#define OFFS_DIR 		0x7
#define OFFS_CCR 		0x7

//! Register ports for the primary FDC

// common registers to PC, AT, PS/2
#define FDC_PRIM_DOR 	(FDC_PRIMARY_BASE + OFFS_DOR)  // Digital Output Register
#define FDC_PRIM_MSR 	(FDC_PRIMARY_BASE + OFFS_MSR)  // Main Status Register (Read only)
#define FDC_PRIM_FIFO 	(FDC_PRIMARY_BASE + OFFS_FIFO) // FIFO Data Register

// registers specific to AT
#define FDC_PRIM_DIR 	(FDC_PRIMARY_BASE + OFFS_DIR) // Digital Input Register (Read only)
#define FDC_PRIM_CCR 	(FDC_PRIMARY_BASE + OFFS_CCR) // Configuration Control Register (Write only)

// registers specific to PS/2
#define FDC_PRIM_SRA 	(FDC_PRIMARY_BASE + OFFS_SRA) // Status Register A (Read only)
#define FDC_PRIM_SRB 	(FDC_PRIMARY_BASE + OFFS_SRB) // Status Register B (Read only)
#define FDC_PRIM_DRSR 	(FDC_PRIMARY_BASE + OFFS_DRSR) // Data Rate Select Register (Write only)
// not used
#define FDC_PRIM_TDR 	(FDC_PRIMARY_BASE + OFFS_TDR) // Tape Drive Register


/* Following commands are used to control the FDC and the connected FDD.
	Always make sure to check the MSR's status bit before sending a command */

// data transfer commands
#define FDC_CMD_READ_TRACK 			0x02	// read entire track, generates irq6
#define FDC_CMD_WRITE_SECTOR 		0x05	// write data to the disk
#define FDC_CMD_READ_SECTOR 		0x06	// read data from the disk
#define FDC_CMD_WRITE_DELETED_SECT 	0x09	// write deleted sector
#define FDC_CMD_READ_DELETED_SECT 	0x0C	// read deleted sector
#define FDC_CMD_FORMAT_TRACK 		0x0D	// format a track
// control commands
#define FDC_CMD_SPECIFY 			0x03	// specify drive parameters
#define FDC_CMD_CHECK_STATUS 		0x04	// check status of the FDC 
#define FDC_CMD_RECALIBRATE 		0x07	// recalibrate the drive
#define FDC_CMD_SENSE_INTERRUPT 	0x08	// sense interrupt status
#define FDC_CMD_READ_ID 			0x0A	// read ID from the disk
#define FDC_CMD_SEEK 				0x0F	// seek heads to cylinder X
//extended and extra commands
#define FDC_CMD_DUMPREG				0x0E	// dump the FDC registers
#define FDC_CMD_VERSION 			0x10	// get the FDC version
#define FDC_CMD_SCAN_EQUAL 			0x11	// scan for equal sectors
#define FDC_CMD_PERPENDICULAR_MODE  0x12	// set perpendicular mode
#define FDC_CMD_CONFIGURE 			0x13	// configure the FDC
#define FDC_CMD_LOCK 				0x14	// lock the FDC
#define FDC_CMD_VERIFY 				0x16	// verify the disk
#define FDC_CMD_SCAN_LOW_OR_EQUAL 	0x19	// scan for low or equal sectors
#define FDC_CMD_SCAN_HIGH_OR_EQUAL 	0x1D	// scan for high or equal sectors


//! Digital Output Register (DOR) bits
/* It is a write only register, that allows us to control different functions
	of FDC. Remember a single FDC can communicate with upto 4 drives */

// bits 0,1
#define FDC_DOR_DRIVE0 				0x01  // Drive 0 (usually A:)
#define FDC_DOR_DRIVE1 				0x02  // Drive 1 (usually B:)
#define FDC_DOR_DRIVE2 				0x04  // Drive 2 (usually C:)
#define FDC_DOR_DRIVE3 				0x08  // Drive 3 (usually D:)
// bit 2
#define FDC_DOR_DISABLE 			0x00  // Disable the FDC
#define FDC_DOR_ENABLE 				0x04  // Enable the FDC
// bit 3
#define FDC_DOR_IRQ_CHANNEL 		0x00  // Use IRQ 6 for FDC
#define FDC_DOR_IRQ_DMA	 			0x08  // Use DMA channel 2 for FDC
// bits 4-7 Motor control
#define FDC_DOR_MOTOR0_MASK 		0x10  // Motor for drive 0
#define FDC_DOR_MOTOR1_MASK 		0x20  // Motor for drive 1
#define FDC_DOR_MOTOR2_MASK 		0x40  // Motor for drive 2
#define FDC_DOR_MOTOR3_MASK 		0x80  // Motor for drive 4

//! Main Status Register (MSR) bits

#define FDC_MSR_D1_STATUS_MASK 		0x01  // Drive 1 status
#define FDC_MSR_D2_STATUS_MASK 		0x02  // Drive 2 status
#define FDC_MSR_D3_STATUS_MASK 		0x04  // Drive 3 status
#define FDC_MSR_D4_STATUS_MASK 		0x08  // Drive 4 status
#define FDC_MSR_BUSY_MASK      		0x10  // FDC status (busy or ready)
#define FDC_MSR_DMA_MASK       		0x20  // FDC in DMA mode or not
#define FDC_MSR_DIO_MASK       		0x40  // 1 if FDC needs to send data
#define FDC_MSR_DATAREG_MASK       	0x80  // 1 if data register (FIFO) is ready

//! Configuration Control Register (CCR) bits
/* Only the first 2 bits needed, which define the data transfer rate for FDD */

#define FDC_CCR_500KBPS 			0x00
#define FDC_CCR_300KBPS 			0x01
#define FDC_CCR_250KBPS 			0x02
#define FDC_CCR_1MBPS 				0x03

//! Extended command bits
/* Some commands require us to pass several bytes before the command execution.
	These bits are passed as higher 3 bits of the command byte. */
	
#define FDC_EXT_SD 					0x20  // Skip deleted data address marks
#define FDC_EXT_DD 					0x40  // Double density mode if set
#define FDC_EXT_MT 					0x80  // Operate on both tracks if set

//! GAP3 code
/* GAP3 refers to the space between sectors on the physical disk. */

#define FDC_GPL_STD 				0x2A  // standard
#define FDC_GPL_5_14 				0x20  
#define FDC_GPL_3_5 				0x1B

//! FDC Bytes Per Sector Codes

#define FDC_BPS_128 				0x00  // 128 bytes per sector
#define FDC_BPS_256 				0x01  // 256 bytes per sector
#define FDC_BPS_512 				0x02  // 512 bytes per sector
#define FDC_BPS_1024 				0x04  // 1024 bytes per sector

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

/* the fdc init function registers the device as a block device, however, the
	functions are still available for direct interfacing with the fdc. */

//! initialize the driver, and install the interrupt handler
void 		fdc_init (void);

//! resets the primary FDC and all drives
void 		fdc_reset (void);

//! set current working drive for the primary FDC
void 		fdc_set_drive (uint8_t drive);

//! get current working drive
uint8_t 	fdc_get_drive (void);

//! read a sector from the current drive, accepts LBA address, assumes the 
//! buffer is at least SECTOR_SIZE bytes long
int32_t 	fdc_read_sector (uint32_t sectorLBA, uint8_t* buff);

//! write a sector to the disk, accepts LBA address
int32_t 	fdc_write_sector (uint32_t sectorLBA, const uint8_t* data);

//! convert a logical block address to cylinder, head, and sector
void 		fdc_lba_to_chs (uint32_t lba, uint32_t* cylinder, 
							uint32_t* head, uint32_t* sector);

//*****************************************************************************
//**
//** 	END _[filename]
//**
//*****************************************************************************
#endif