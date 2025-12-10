#ifndef _TIMER_H
#define _TIMER_H
//*****************************************************************************
//*
//*  @file		keyboard.h
//*  @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief		PIT driver interface header file
//*  @version	
//*
//*****************************************************************************
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdint.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

// Defines for the Programmable Interval Timer (PIT) ports
#define PIT_CHANNEL0_DATA_PORT 		0x40
#define PIT_CHANNEL1_DATA_PORT 		0x41
#define PIT_CHANNEL2_DATA_PORT 		0x42
#define PIT_COMMAND_PORT 			0x43

// defines for the command register
#define PIT_SELECT_CHANNEL0 		0x00
#define PIT_SELECT_CHANNEL1 		0x40
#define PIT_SELECT_CHANNEL2 		0x80

#define PIT_READ_BACK_CMD 			0xC0

#define PIT_LATCH_COUNT_CMD 		0x00
#define PIT_ACCESS_MODE_LOBYTE 		0x10
#define PIT_ACCESS_MODE_HIBYTE 		0x20
#define PIT_ACCESS_MODE_LO_HI 		0x30

#define PIT_MODE(x) 				((x) << 1)
#define PIT_MODE_TERMINAL_COUNT     PIT_MODE(0)
#define PIT_MODE_ONESHOT            PIT_MODE(1)
#define PIT_MODE_RATE_GENERATOR     PIT_MODE(2)
#define PIT_MODE_SQUARE_WAVE        PIT_MODE(3)
#define PIT_MODE_SOFTWARE_STROBE    PIT_MODE(4)
#define PIT_MODE_HARDWARE_STROBE    PIT_MODE(5)
#define PIT_MODE_RATE_GENERATOR_2   PIT_MODE(6)
#define PIT_MODE_SQUARE_WAVE_2      PIT_MODE(7)

#define PIT_BCD_MODE 				0x01
#define PIT_BINARY_MODE 			0x00

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

/**
 * @brief Initializes the system PIC timer to generate interrupts at a
 * specified frequency.
 * 
 * @param frequency 
 */
void init_system_timer(uint32_t frequency);

/**
 * @brief Returns the current tick count since the timer was initialized.
 * 
 */
uint32_t get_system_tick_count(void);

/**
 * @brief Suspends the current running thread for given number of milliseconds.
 * 
 * @param ms The number of milliseconds to suspend the thread.
 */
void sleep(uint32_t ms);

//*****************************************************************************
//**
//** 	END _[filename]
//**
//*****************************************************************************
#endif