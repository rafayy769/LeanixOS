#ifndef _TTY_H
#define _TTY_H
/*******************************************************************************
 *
 *  @file		tss.h
 *  @author    	Abdul Rafay (abdul.rafay@lums.edu.pk)
 *  @brief		This file contains the definitions and declarations for the
 * 				terminal interface used by the system. Currently only configured
 *				to use the Keyboard for input, and VGA for output, but that
 * 				is enough for the current purposes. However, serial support
 *				will be added in the next iteration to make tests.
 *  @version	0.1
 *
*******************************************************************************/

//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <driver/vga.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

//! set the default color for the background and foreground
#define TTY_DEFAULT_COLOR 	vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE)

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

//! todo need to add a terminal management struct like termios

//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

//! (added later/depc.) Initializes the terminal interface for input and output.
void 	tty_init (void);

//! Initializes terminal's stdout facilities, including the drivers
void 	terminal_stdout_init(void);

//! Initializes the terminal's stdin facilities, including device drivers
void 	terminal_stdin_init(void);

//! Clear terminal screen
void 	terminal_clear_scr (void);
void 	terminal_clear_log (void);      // clears the log only

//! Modify the current location of the cursor on the terminal
void 	terminal_move_cursor (uint8_t x, uint8_t y);

//! Modify terminal color behavior
uint8_t terminal_get_colour (void);
void 	terminal_setcolor (uint8_t color);      // fg and bg
void 	terminal_settext_color (enum vga_color color);  // text color
void 	terminal_setbg_color (enum vga_color color);    // bg
void 	terminal_reset_color (void);    // reset to the configured default

//! Output to terminal. note strings should be null terminated
void 	terminal_putc(char c);
void 	terminal_write(const char* data, uint32_t size);
void 	terminal_writestring(const char* data);

//! input functions read from the currently configured input device
char 	terminal_getc(void);
void 	terminal_read(char* buffer, uint32_t size);

#endif // _TTY_H
