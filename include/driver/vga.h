#ifndef _VGA_H
#define _VGA_H
//*****************************************************************************
//*
//*  @file		vga.h
//*  @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief		Defines the interface for VGA text mode operations
//*  @version	0.1
//*
//*****************************************************************************
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <string.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

/**
 * Enumeration of VGA colors. Max value is 15, which corresponds to the color 
 * 	white. Can be used to set the foreground and background colors of text in 
 *  the VGA text mode.
 */

enum vga_color {
	VGA_COLOR_BLACK         = 0,
	VGA_COLOR_BLUE          = 1,
	VGA_COLOR_GREEN         = 2,
	VGA_COLOR_CYAN          = 3,
	VGA_COLOR_RED           = 4,
	VGA_COLOR_MAGENTA       = 5,
	VGA_COLOR_BROWN         = 6,
	VGA_COLOR_LIGHT_GREY    = 7,
	VGA_COLOR_DARK_GREY     = 8,
	VGA_COLOR_LIGHT_BLUE    = 9,
	VGA_COLOR_LIGHT_GREEN   = 10,
	VGA_COLOR_LIGHT_CYAN    = 11,
	VGA_COLOR_LIGHT_RED     = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN   = 14,
	VGA_COLOR_WHITE         = 15,
};

//! Defines for the VGA text mode
#define VGA_WIDTH  			80
#define VGA_HEIGHT 			25
#define VGA_ADDRESS 		0xB8000
#define VGA_SIZE 			(VGA_WIDTH * VGA_HEIGHT)
#define VGA_ENTRY_SIZE 		2 //bytes

//! Defines for the VGA hardware cursor
#define VGA_CRTC_INDEX_PORT 0x3D4
#define VGA_CRTC_DATA_PORT 	0x3D5

#define VGA_HARDWARE_CURSOR_CMD_SETHIGH 0x0E
#define VGA_HARDWARE_CURSOR_CMD_SETLOW  0x0F

//! typedef a VGA entry as a 16-bit unsigned integer
typedef uint16_t vga_entry_t;

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

/**
 * @brief Combines foreground and background colors into a single color byte.
 * 
 * @param fg Foreground color (should be one of the enum vga_color values)
 * @param bg Background color (should be one of the enum vga_color values)
 * @return uint8_t Combined color byte where the lower 4 bits are the foreground
 * color and the upper 4 bits are the background color.
 */
uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);
 
/**
 * @brief Creates a VGA entry with the specified character and color.
 * 
 * @param uc Character to be displayed
 * @param color Color of the character (should be an entry returned by
 * 				vga_entry_color)
 * @return uint16_t VGA entry combining character and color
 */
vga_entry_t vga_entry(unsigned char uc, uint8_t color);

/**
 * @brief Moves the hardware cursor to the specified position on the VGA text
 *		   mode screen.
 * 
 * @param x X coordinate (column) of the cursor position, 0 to VGA_WIDTH - 1
 * @param y Y coordinate (row) of the cursor position, 0 to VGA_HEIGHT - 1
 */
void vga_move_cursor_to(uint8_t x, uint8_t y);

/**
 * @brief Puts a character with a specific color at the specified position on
 *		  the VGA text mode screen.
 * 
 * @param entry VGA entry containing the character and color 
 *				(created using vga_entry) 
 * @param x X coordinate (column) where the character should be placed,
 *			 ranging from 0 to VGA_WIDTH - 1
 * @param y Y coordinate (row) where the character should be placed, ranging
 *			 from 0 to VGA_HEIGHT - 1
 */
void vga_putentry_at(vga_entry_t entry, uint8_t x, uint8_t y);

/**
 * @brief Returns a pointer to the VGA screen buffer.
 * 
 * @return vga_entry_t* Pointer to the VGA screen buffer, which is an array of 
 *	vga_entry_t entries representing the characters and their colors on the screen.
 */
vga_entry_t* vga_get_screen_buffer(void);

#endif // _VGA_H