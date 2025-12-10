#ifndef LOG_H
#define LOG_H
//*****************************************************************************
//*
//*  @file		log.h
//*  @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief		A collection of macros and functions to facilitate logging in a
//*             helpful manner, instead of using plain printf statements, inside
//*             the kernel. Supports module level logs.
//*             Somewhat inspired from the contiki-ng's logging facilities, 
//*             though the usage is different.
//*  @version	1.2
//*  @date		22-07-2025
//*
//****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdarg.h>

#include <stdio.h>
#include <init/tty.h>
#include <driver/vga.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

//! logging modes
#define     LOG_ERR     0   //? critical errors, always get printed
#define     LOG_DBG     1   //? debug print statements, DEBUG must be defined
#define     LOG_PRI     2   //? normal info messages

//! default logger logs to the terminal
//  TODO: must add the support/flags for file level logs
#ifndef     LOG_OUT
#define     LOG_OUT(...)    printk(__VA_ARGS__)
#endif

//! default colors for annotations
#define     LOG_COLOR_ERR   VGA_COLOR_LIGHT_RED
#define     LOG_COLOR_DBG   VGA_COLOR_LIGHT_CYAN
#define     LOG_COLOR_PRI   VGA_COLOR_LIGHT_GREEN

//! default prefixes to use when module name not available
#define     _ERR_PREFIX     " [ERR] - "
#define     _DBG_PREFIX     " [DBG] - "
#define     _INF_PREFIX     " [ + ] - "

//! macros to support module level logs (defines defaults otherwise)
//! whether the logs for the module are enabled or not
#ifndef     LOG_MOD_ENABLE
#define     LOG_MOD_ENABLE  1
#endif
//! module name prefix
#ifndef     LOG_MOD_NAME
#define     LOG_MOD_NAME    0
#endif

//! whether the debug mode is on or off (by default disabled)
//! can be provided as a flag for the preprocessor
#ifndef     DEBUG
#define     DEBUG           0
#endif

/*** The main macros that do the printing, simply a bunch of if statements ***/

//! prints the correct prefix (default or module name)
#define _PRINT_PREFIX(type, color, fallback)                    \
    do {                                                        \
        terminal_settext_color (color);                         \
        if (LOG_MOD_NAME) {                                     \
            LOG_OUT (" [%s] - ", LOG_MOD_NAME);                 \
        } else {                                                \
            LOG_OUT ("%s", fallback);                           \
        }                                                       \
        terminal_reset_color ();                                \
    } while (0)

//! internal logger
//! will customize it later to maybe always print the error messages no matter
//! what, secondly, i can color the error statement entirely as well
#define _LOG(type, fallback_prefix, color, ...)                               \
    do {                                                                      \
        if ((type == LOG_DBG && DEBUG && LOG_MOD_ENABLE) ||                   \
            (type != LOG_DBG && LOG_MOD_ENABLE))                              \
        {                                                                     \
            _PRINT_PREFIX (type, color, fallback_prefix);                  \
            LOG_OUT (__VA_ARGS__);                                            \
        }                                                                     \
    } while (0)

//! the actual exposed macros
#define LOG_ERROR(...) _LOG(LOG_ERR, _ERR_PREFIX, LOG_COLOR_ERR, __VA_ARGS__)
#define LOG_DEBUG(...) _LOG(LOG_DBG, _DBG_PREFIX, LOG_COLOR_DBG, __VA_ARGS__)
#define LOG_P(...)     _LOG(LOG_PRI, _INF_PREFIX, LOG_COLOR_PRI, __VA_ARGS__)


//! Useful macro for pure colored logs
#define LOG_COLORED(color, ...) \
    do { \
        terminal_settext_color(color); \
        LOG_OUT(__VA_ARGS__); \
        terminal_reset_color(); \
    } while (0)


//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------
//*****************************************************************************
//**
//** 	END log.h
//**
//*****************************************************************************
#endif // LOG_H