#ifndef _TSS_H
#define _TSS_H
//*****************************************************************************
//*
//*  @file		tss.h
//*  @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief		This file contains the definitions and declarations for the
//* 			x86 Thread State Segment (TSS) and its related operations.
//*  @version	
//*
//****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stddef.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

/******************************************************************************
 * 
 * @struct tss_t
 * @brief  The Task State Segment (TSS) structure for x86 architecture
 * 
 * This structure is used to store the state of a task, including kernel and
 *  user stack pointers, segment selectors and GP regs. Primarily used for 
 *  hardwarer switching but that's generally obsolete, but one is still required
 * 	for software switching.
 * 
*******************************************************************************/
typedef struct {

	//! Previous TSS segment selector, linked list for hardware task switching
	uint32_t prev_tss;

	//! Stack pointer and segment for privilege level 0
	uint32_t esp0, ss0;

	//! Stack pointer and segment for privilege level 1
	uint32_t esp1, ss1;

	//! Stack pointer and segment for privilege level 2
	uint32_t esp2, ss2;

	//! Control register (contains pdbr)
	uint32_t cr3;

	//! Instruction pointer
	uint32_t eip;
	
	//! Flags register
	uint32_t eflags;

	//! General-purpose registers
	uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;

	//! Segment selectors (value loaded when switched to kernel mode)
	uint32_t es, cs, ss, ds, fs, gs; 

	//! Local Descriptor Table segment selector (unused)
	uint32_t ldt;          

	//! Trap flag
	uint16_t trap;
	
	//! I/O map base address
	uint16_t iomap_base;
	
} __attribute__((packed)) tss_t;

//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

//! Load the Task State Segment (TSS) with the given selector in the TSR
void 	tss_flush (uint16_t selector);

//! Updates the global TSS with the provided values
void 	tss_update_esp0 (uint32_t esp0);

//! Returns a pointer to the global TSS structure
tss_t* 	tss_get_global (void);

//*****************************************************************************
//**
//** 	END _[filename]
//**
//*****************************************************************************
#endif // _TSS_H