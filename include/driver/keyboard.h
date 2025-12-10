#ifndef _KEYBOARD_H
#define _KEYBOARD_H
/*******************************************************************************
 *
 *  @file		keyboard.h
 *  @author    	Abdul Rafay (abdul.rafay@lums.edu.pk)
 *  @brief		Keyboard driver interface header file
 *  @version	0.1
 *
*******************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES
//-----------------------------------------------------------------------------

//! Keyboard encoder IO defines
#define KBD_ENC_INPUT_BUF 			0x60
#define KBD_ENC_CMD_REG   			0x60

//! Keyboard encoder commands
#define KBD_ENC_CMD_SET_LEDS  		0xED   // set keyboard LEDs
#define KBD_ENC_CMD_ECHO      		0xEE   // echo command
#define KBD_ENC_CMD_SET_SCANCODE 	0xF0   // set scan code set
#define KBD_ENC_CMD_IDENTIFY  		0xF2   // identify keyboard (from 0x60)
#define KBD_ENC_CMD_AUTODELAY 		0xF3   // set auto repeat delay and rate
#define KBD_ENC_CMD_ENABLE 	  		0xF4   // enable keyboard
#define KBD_ENC_CMD_RESETWAIT 		0xF5   // reset keyboard and wait for enable
#define KBD_ENC_CMD_RESETSCAN 		0xF6   // reset keyboard and begin scanning
#define KBD_ENC_CMD_ALLREPEAT 		0xF7   // set all keys to auto repeat (PS2)
#define KBD_ENC_CMD_ALLMAKEBREAK   	0xF8   // all keys generate make+break codes
#define KBD_ENC_CMD_ALLMAKE   		0xF9   // all keys generate make codes only
#define KBD_ENC_CMD_ALLREPMAKE  	0xFA   // all keys autorepeat and make/break
#define KBD_ENC_CMD_SINGLEREPEAT 	0xFB   // set single key to auto repeat
#define KBD_ENC_CMD_SINGLEMAKE   	0xFC   // set single key to generate make codes only
#define KBD_ENC_CMD_SINGLEBREAK 	0xFD   // set single key to generate break codes only
#define KBD_ENC_CMD_RESEND    		0xFE   // resend last result
#define KBD_ENC_CMD_RESET    		0xFF   // reset keyboard

//! Keyboard controller defines
#define KBD_CTRL_STATUS_REG 		0x64
#define KBD_CTRL_CMD_REG    		0x64
//! keyboard controller status register bits
#define KBD_CTRL_OUTBUF 			0x01	// output buffer full if set
#define KBD_CTRL_INBUF  			0x02	// input buffer full if set
#define KBD_CTRL_SYS_FLAG   		0x04	// system flag set after BAT (self test)
#define KBD_CTRL_CMD_DATA 			0x08	// last write is: 1 for command, 0 for data
#define KBD_CTRL_LOCKED 			0x10	// keyboard locked if set

#define KBD_CTRL_AUX_OUTBUF 0x20	// auxiliary device flag
					                // ps/2: 0: kbd data, 1: mouse data
	                                // AT: 0: ok flag, 1: timeout

#define KBD_CTRL_TIMEOUT 	0x40	// timeout flag set if 1
#define KBD_CTRL_PARITY_ERR 0x80	// parity error with last byte if set

//! Keyboard controller commands
#define KBD_CTRL_CMD_READ 	  	  	0x20	// read command byte
#define KBD_CTRL_CMD_WRITE 	  	  	0x60	// write command byte
#define KBD_CTRL_CMD_SELFTEST 	  	0xAA	// self test command
#define KBD_CTRL_CMD_INTTEST  	  	0xAB	// keyboard interface test command
#define KBD_CTRL_CMD_DISABLE  	  	0xAD	// disable keyboard
#define KBD_CTRL_CMD_ENABLE   	  	0xAE	// enable keyboard
#define KBD_CTRL_CMD_READINP  	  	0xC0	// read keyboard Input port
#define KBD_CTRL_CMD_READOUT  	  	0xD0	// read keyboard Output port
#define KBD_CTRL_CMD_WRITEOUT 	  	0xD1	// write keyboard Output port
#define KBD_CTRL_CMD_READ_TESTINP 	0xE0	// read test inputs
#define KBD_CTRL_CMD_SYSRESET 	  	0xFE	// system reset command
#define KBD_CTRL_CMD_DISABLEMOUSE   0xA7	// disable mouse port
#define KBD_CTRL_CMD_ENABLEMOUSE    0xA8	// enable mouse port
#define KBD_CTRL_CMD_TESTMOUSE      0xA9	// test mouse port
#define KBD_CTRL_CMD_WRITEMOUSE     0xD4	// write to mouse port

//! keyboard error codes
#define KBD_ERR_BUFF_OVERRUN 		0x00 	// internal buffer overrun
#define KBD_ERR_ID_RETURN    		0x83AB  // identify command returned invalid data
#define KBD_ERR_BAT_RET	     		0xAA    // returned by BAT (also keycode for Lshift)
#define KBD_ERR_ECHO_RET     		0xEE    // echo command failed
#define KBD_ERR_ACK 		 		0xFA    // kbd acknowledge to kbd command
#define KBD_ERR_BAT_FAILED   		0xFC    // kbd self test failed
#define KBD_ERR_DIAG_FAILED  		0xFD    // kbd diagnostic failed
#define KBD_ERR_RESEND_CMD   		0xFE    // kbd request to resend last command
#define KBD_ERR_KEY			 		0xFF    // kbd keycode not recognized

#define KBD_INVALID_SCANCODE 		0x00 // invalid scancode

//! keyboard LEDs
#define KBD_LED_SCROLLLOCK   		0x01 // scroll lock LED
#define KBD_LED_NUMLOCK      		0x02 // num lock LED
#define KBD_LED_CAPSLOCK     		0x04 // caps lock LED

//! a simple macro to test if a given byte is ascii or not
#define KBD_IS_ASCII(c) 			((c) >= 0x7 && (c) <= 0x7F)

//-----------------------------------------------------------------------------
// 		INTERFACE TYPES
//-----------------------------------------------------------------------------

//! We define the codes for the keyboard keys
typedef enum __KEYCODE {

// Alphanumeric keys ////////////////

	KEY_SPACE             = ' ',
	KEY_0                 = '0',
	KEY_1                 = '1',
	KEY_2                 = '2',
	KEY_3                 = '3',
	KEY_4                 = '4',
	KEY_5                 = '5',
	KEY_6                 = '6',
	KEY_7                 = '7',
	KEY_8                 = '8',
	KEY_9                 = '9',

	KEY_A                 = 'a',
	KEY_B                 = 'b',
	KEY_C                 = 'c',
	KEY_D                 = 'd',
	KEY_E                 = 'e',
	KEY_F                 = 'f',
	KEY_G                 = 'g',
	KEY_H                 = 'h',
	KEY_I                 = 'i',
	KEY_J                 = 'j',
	KEY_K                 = 'k',
	KEY_L                 = 'l',
	KEY_M                 = 'm',
	KEY_N                 = 'n',
	KEY_O                 = 'o',
	KEY_P                 = 'p',
	KEY_Q                 = 'q',
	KEY_R                 = 'r',
	KEY_S                 = 's',
	KEY_T                 = 't',
	KEY_U                 = 'u',
	KEY_V                 = 'v',
	KEY_W                 = 'w',
	KEY_X                 = 'x',
	KEY_Y                 = 'y',
	KEY_Z                 = 'z',

	KEY_RETURN            = '\r',
	KEY_ESCAPE            = 0x1001,
	KEY_BACKSPACE         = '\b',

// Arrow keys ////////////////////////

	KEY_UP                = 0x1100,
	KEY_DOWN              = 0x1101,
	KEY_LEFT              = 0x1102,
	KEY_RIGHT             = 0x1103,

// Function keys /////////////////////

	KEY_F1                = 0x1201,
	KEY_F2                = 0x1202,
	KEY_F3                = 0x1203,
	KEY_F4                = 0x1204,
	KEY_F5                = 0x1205,
	KEY_F6                = 0x1206,
	KEY_F7                = 0x1207,
	KEY_F8                = 0x1208,
	KEY_F9                = 0x1209,
	KEY_F10               = 0x120a,
	KEY_F11               = 0x120b,
	KEY_F12               = 0x120b,
	KEY_F13               = 0x120c,
	KEY_F14               = 0x120d,
	KEY_F15               = 0x120e,

	KEY_DOT               = '.',
	KEY_COMMA             = ',',
	KEY_COLON             = ':',
	KEY_SEMICOLON         = ';',
	KEY_SLASH             = '/',
	KEY_BACKSLASH         = '\\',
	KEY_PLUS              = '+',
	KEY_MINUS             = '-',
	KEY_ASTERISK          = '*',
	KEY_EXCLAMATION       = '!',
	KEY_QUESTION          = '?',
	KEY_QUOTEDOUBLE       = '\"',
	KEY_QUOTE             = '\'',
	KEY_EQUAL             = '=',
	KEY_HASH              = '#',
	KEY_PERCENT           = '%',
	KEY_AMPERSAND         = '&',
	KEY_UNDERSCORE        = '_',
	KEY_LEFTPAREN   	  = '(',
	KEY_RIGHTPAREN  	  = ')',
	KEY_LEFTBRACKET       = '[',
	KEY_RIGHTBRACKET      = ']',
	KEY_LEFTCURL          = '{',
	KEY_RIGHTCURL         = '}',
	KEY_DOLLAR            = '$',
	KEY_POUND             = '$',
	KEY_EURO              = '$',
	KEY_LESS              = '<',
	KEY_GREATER           = '>',
	KEY_BAR               = '|',
	KEY_GRAVE             = '`',
	KEY_TILDE             = '~',
	KEY_AT                = '@',
	KEY_CARRET            = '^',

// Numeric keypad //////////////////////

	KEY_KP_0              = '0',
	KEY_KP_1              = '1',
	KEY_KP_2              = '2',
	KEY_KP_3              = '3',
	KEY_KP_4              = '4',
	KEY_KP_5              = '5',
	KEY_KP_6              = '6',
	KEY_KP_7              = '7',
	KEY_KP_8              = '8',
	KEY_KP_9              = '9',
	KEY_KP_PLUS           = '+',
	KEY_KP_MINUS          = '-',
	KEY_KP_DECIMAL        = '.',
	KEY_KP_DIVIDE         = '/',
	KEY_KP_ASTERISK       = '*',
	KEY_KP_NUMLOCK        = 0x300f,
	KEY_KP_ENTER          = 0x3010,

	KEY_TAB               = 0x4000,
	KEY_CAPSLOCK          = 0x4001,

// Modify keys ////////////////////////////

	KEY_LSHIFT            = 0x4002,
	KEY_LCTRL             = 0x4003,
	KEY_LALT              = 0x4004,
	KEY_LWIN              = 0x4005,
	KEY_RSHIFT            = 0x4006,
	KEY_RCTRL             = 0x4007,
	KEY_RALT              = 0x4008,
	KEY_RWIN              = 0x4009,

	KEY_INSERT            = 0x400a,
	KEY_DELETE            = 0x400b,
	KEY_HOME              = 0x400c,
	KEY_END               = 0x400d,
	KEY_PAGEUP            = 0x400e,
	KEY_PAGEDOWN          = 0x400f,
	KEY_SCROLLLOCK        = 0x4010,
	KEY_PAUSE             = 0x4011,

	KEY_UNKNOWN,
	KEY_NUMKEYCODES

} KBD_KEYCODE;

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

/* The keyboard driver uses an internal ring buffer to store the last keycode
	that was pressed and also the corresponding ascii translation (which depends
	on the internal state like shift pressed, capslock etc.) */

typedef struct __KBD_BUFFER_ENTRY {

	KBD_KEYCODE keycode; 	// the keycode
	char 		ascii;  	// the ascii translation (0 if not ascii)

} KBD_ENTRY;

//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

//! initializes	keyboard driver
void 		kbd_init(void);

//! get last scan code or keystroke
uint8_t 	kbd_get_lastscancode(void);
KBD_KEYCODE kbd_get_lastkey(void);
void 		kbd_clear_lastkey(void);
char 		kbd_keycode_to_ascii(KBD_KEYCODE keycode);
bool 		kbd_is_key_released(void);

//! buffered mode operation, returns the last key from the buffer
KBD_ENTRY 	kbd_getlastkey_buf();

//! status of lock keys
bool 		kbd_get_numlock(void);
bool 		kbd_get_capslock(void);
bool 		kbd_get_scrolllock(void);

//! status of modifier keys
bool 		kbd_get_shift(void);
bool 		kbd_get_ctrl(void);
bool 		kbd_get_alt(void);

//! resend last command
void 		kbd_ignore_resend(void);
bool 		kbd_should_resend(void);

//! control the leds
void 		kbd_set_leds(bool numlock, bool capslock, bool scrolllock);

//! keyboard controller status and tests
bool 		kbd_self_test(void);
bool 		kbd_bat_result(void);
bool 		kbd_diag_result(void);

//! enable/disable keyboard
bool 		kbd_is_enabled(void);
void 		kbd_setstate(bool enable);

//*****************************************************************************
//**
//** 	END _[filename]
//**
//*****************************************************************************
#endif // ! _KEYBOARD_H