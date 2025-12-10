#ifndef _SYSCALL_H
#define _SYSCALL_H
//*****************************************************************************
//*
//*  @file		syscall.h
//*  @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief		System call interface header file. basically kernel side
//*              declarations for syscalls.
//*  @version	
//*
//****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

//! enum to define syscall numbers
typedef enum {

	SYSCALL_READ = 0,   //? syscall number for read
	SYSCALL_WRITE,      //? syscall number for write
	SYSCALL_FORK,       //? syscall number for fork
	SYSCALL_EXEC,       //? syscall number for exec
	
} syscall_nr;

//! Macros to declare syscall functions, with 0 to 5 params
#define _DECL_SYSCALL_P0(fn) int32_t fn ();
#define _DECL_SYSCALL_P1(fn, p1) int32_t fn (p1);
#define _DECL_SYSCALL_P2(fn, p1, p2) int32_t fn (p1, p2);
#define _DECL_SYSCALL_P3(fn, p1, p2, p3) int32_t fn (p1, p2, p3);
#define _DECL_SYSCALL_P4(fn, p1, p2, p3, p4) int32_t fn (p1, p2, p3, p4);
#define _DECL_SYSCALL_P5(fn, p1, p2, p3, p4, p5) int32_t fn (p1, p2, p3, p4, p5);

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

//! declaration of syscall functions
_DECL_SYSCALL_P2(read, char*, size_t);
_DECL_SYSCALL_P2(write, const char*, size_t);
_DECL_SYSCALL_P0(fork);
_DECL_SYSCALL_P1(exec, const char*);

//! initialize system call interface and make it available
void syscall_init();

//*****************************************************************************
//**
//** 	END _[filename]
//**
//*****************************************************************************
#endif //! _SYSCALL_H