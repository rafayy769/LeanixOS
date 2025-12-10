// the header file template for all the files
#ifndef _DMA_H
#define _DMA_H
//*****************************************************************************
//*
//*  @file		[dma.h]
//*  @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief		This file defines the functions and constants for interfacing
//*             with the Intel 8237A ISA DMA controller.
//*  @version	
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

/* DMAC generic registers. Remember, there's two DMACs on a ISA motherboad,
	DMAC0 is the 8 bit slave, while the DMAC1 is the 16 bit master. The slave
	controller is connected to the master using the DRQ4 (channel 4), and hence
	this channel can't be used for the transfers. */

#define DMAC0_REG_STATUS 			0x08  // Status register (R)
#define DMAC0_REG_COMMAND 			0x08  // Command register (W)
#define DMAC0_REG_REQUEST 			0x09  // Request register (W)
#define DMAC0_REG_CHANMASK	 		0x0A  // Channel Mask register (W)
#define DMAC0_REG_MODE 				0x0B  // Mode register (W)
#define DMAC0_REG_CLEARFLIPFLOP 	0x0C  // Clear flip flop register (W)
#define DMAC0_REG_TEMP 				0x0D  // Temporary register (R)
#define DMAC0_REG_MASTERCLEAR 		0x0D  // Master clear register (W)
#define DMAC0_REG_CLEARMASK 		0x0E  // Clear mask register (W)
#define DMAC0_REG_MASK 				0x0F  // Write Mask register (W)

#define DMAC1_REG_STATUS 			0xD0  // Status register (R)
#define DMAC1_REG_COMMAND 			0xD0  // Command register (W)
#define DMAC1_REG_REQUEST 			0xD2  // Request register (W)
#define DMAC1_REG_CHANMASK	 		0xD4  // Channel Mask register (W)
#define DMAC1_REG_MODE 				0xD6  // Mode register (W)
#define DMAC1_REG_CLEARFLIPFLOP 	0xD8  // Clear flip flop register (W)
#define DMAC1_REG_TEMP 				0xDA  // Temporary register (R)
#define DMAC1_REG_MASTERCLEAR 		0xDA  // Master clear register (W)
#define DMAC1_REG_CLEARMASK 		0xDC  // Clear mask register (W)
#define DMAC1_REG_MASK 				0xDE  // Write Mask register (W)

/* Channel registers provide the way to initialize the DMA channels. Each
	channel has a base address and a counter. Base address refers to the memory
	location to start reading/writing from and the counter refers to amount
	of data to transfer based on the mode. */

// 8 bit slave controller channels
#define DMAC0_CHAN0_ADDR 		0x00  // Channel 0 address register
#define DMAC0_CHAN0_COUNT 		0x01  // Channel 0 count
#define DMAC0_CHAN1_ADDR 		0x02  // Channel 1 address register
#define DMAC0_CHAN1_COUNT 		0x03  // Channel 1 count
#define DMAC0_CHAN2_ADDR 		0x04  // Channel 2 address register
#define DMAC0_CHAN2_COUNT 		0x05  // Channel 2 count
#define DMAC0_CHAN3_ADDR 		0x06  // Channel 3 address register
#define DMAC0_CHAN3_COUNT 		0x07  // Channel 3 count

// 16 bit master controller channels (channel 4 is unused)
#define DMAC1_CHAN4_ADDR 		0xC0  // Channel 4 address register
#define DMAC1_CHAN4_COUNT 		0xC2  // Channel 4 count
#define DMAC1_CHAN5_ADDR 		0xC4  // Channel 5 address register
#define DMAC1_CHAN5_COUNT 		0xC6  // Channel 5 count
#define DMAC1_CHAN6_ADDR 		0xC8  // Channel 6 address register
#define DMAC1_CHAN6_COUNT 		0xCA  // Channel 6 count
#define DMAC1_CHAN7_ADDR 		0xCC  // Channel 7 address register
#define DMAC1_CHAN7_COUNT 		0xCE  // Channel 7 count

/* Page address registers set which page to use for the memory transfer. DMAC is
	limited to access only 64K memory because the registers are at max 16 bits.
	In order to extend the size, we got 8 bit page registers, which store the
	upper 8 bits of the address, effectively allowing us to access 16MB RAM.
	A page register is available for each channel. Original PC had only 4 bits
	for page registers, while the AT design added 4 more bits. */

#define DMA_CHAN0_PAGE 			0x87  // Channel 0 page register
#define DMA_CHAN1_PAGE			0x83  // Channel 1 page register
#define DMA_CHAN2_PAGE			0x81  // Channel 2 page register
#define DMA_CHAN3_PAGE			0x83  // Channel 3 page register
#define DMA_CHAN5_PAGE 			0x8B  // Channel 5 page register
#define DMA_CHAN6_PAGE 			0x89  // Channel 6 page register
#define DMA_CHAN7_PAGE 			0x8A  // Channel 7 page register

// some extra registers in the page addr reguigsters space
#define DMA_EXTRA0_PAGE			0x80  // Diagnostics port
#define DMA_DRAM_REFRESH_PAGE	0x8F  // also channel 4 (not used)

/* Command register is the primary register to use to send commands to a DMA
	controller. Contains the following bits. However, only controller enable
	bit is used on x86. The rest are included for completeness. */

#define DMA_CMD_MEMTOMEM 		0x01  // Memory to memory transfer enable
#define DMA_CMD_CHAN0ADHE		0x02  // Channel 0 address hold enable
#define DMA_CMD_DISABLE			0x04  // controller disable
#define DMA_CMD_TIMING			0x08  // timing (normal(0)/compressed(1))
#define DMA_CMD_PRIORITY		0x10  // priority (fixed(0)/normal(1))
#define DMA_CMD_EXTENDEDWRITE	0x20  // write selection (late/extended)
#define DMA_CMD_DREQ			0x40  // DMA request
#define DMA_CMD_DACK			0x80  // DMA Acknowledge

/* Mode register bits. We specify the operation mode, channel to use etc. via 
	this register. Always mask off the channel to use before writing to this. */

// bits 0-1 specify channel
#define DMA_MODE_CHAN0			0x00   //00000000
#define DMA_MODE_CHAN1			0x01   //00000001
#define DMA_MODE_CHAN2			0x02   //00000010
#define DMA_MODE_CHAN3			0x03   //00000011

// bits 2-3 specify the transfer type
#define DMA_MODE_SELFTEST		0x00   //00000000 (verify transfer)
#define DMA_MODE_READ			0x04   //00000100
#define DMA_MODE_WRITE			0x08   //00001000
/* both bits set is invalid transfer type */

// bit 4,5 automatic reinitialization after transfer completion, dec/increment
#define DMA_MODE_AUTOREINIT		0x10   //00010000
#define DMA_MODE_IINC			0x00   //00000000 (increment address)
#define DMA_MODE_IDEC			0x20   //00100000

// bits 6-7 transfer mode
#define DMA_MODE_TOD 			0x00	//00000000 (transfer on demand)
#define DMA_MODE_SINGLE 		0x40	//01000000
#define DMA_MODE_BLOCK			0x80    //10000000
#define DMA_MODE_CASCADE		0xc0    //11000000

/* Helper macros */

// get the low and high byte of a 16 bit address
#define BYTE_LOW(addr) 	((uint8_t)((addr) & 0x00FF))
#define BYTE_HIGH(addr) ((uint8_t)(((addr) & 0xFF00) >> 8))

/* Pre-defined DMA channels to use on PC-AT. The rest are free to use */

#define DMA_CHAN_DRAM   		0
#define DMA_CHAN_FLOPPY 		2
#define DMA_CHAN_HDD			3

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

//! reset the specified DMA controller
void 		dma_reset (uint8_t dmac);

//! disable the specified DMA controller
void 		dma_disable (uint8_t dmac);

//! set the memory address for the DMA transfer on the specified channel
void 		dma_set_address (uint8_t channel, uint16_t address);

//! set the count for the DMA transfer on the specified channel
void		dma_set_count (uint8_t channel, uint16_t count);

//! set the external page register for the specified channel
void 		dma_set_external_pagereg (uint8_t channel, uint8_t page);

//! mask a DMA channel to disable it
void		dma_mask_channel (uint8_t channel);

//! unmaks a DMA channel to enable it
void		dma_unmask_channel (uint8_t channel);

//! set the transfer type and transfer mode for a transfer on a channel
void 		dma_set_mode (uint8_t channel, uint8_t mode);

//! reset the DMA flip flop
void		dma_reset_flipflop (uint8_t dmac);

//! sets the DMA in read transfer mode on the specified channel
void		dma_setup_read (uint8_t channel);

//! sets the DMA in write transfer mode on the specified channel
void 		dma_setup_write (uint8_t channel);

//! unmaks all DMA channels for use on the given DMAC
void 		dma_unmask_all ();

//*****************************************************************************
//**
//** 	END _[filename]
//**
//*****************************************************************************
#endif //! _DMA_H