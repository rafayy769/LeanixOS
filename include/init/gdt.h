#ifndef _GDT_H
#define _GDT_H
//******************************************************************************
//*
//*  @file		gdt.h
//*  @author	Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief		Contains definitions and functions to setup the Global
//* 			Descriptor Table (GDT) for the x86 architecture. Exposes the
//* 			interface to initialize the GDT and load it. Currently, only
//* 			the flat protected mode GDT is supported, which is sufficient
//*				for our bootloader and kernel setup.
//*  @version	0.1
//*
//****************************************************************************/

//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdint.h>

//-----------------------------------------------------------------------------
// 		PUBLIC INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

//! Segment present
#define GDT_ACCESS_PRESENT 		0x80        //10000000

//! Descriptor Privilege Level (DPL) bits (0-3)
#define	GDT_ACCESS_RING0		0x00        //00000000
#define	GDT_ACCESS_RING1		0x20        //00100000
#define	GDT_ACCESS_RING2		0x40        //01000000
#define	GDT_ACCESS_RING3		0x60        //01100000

//! non system segment
#define GDT_ACCESS_USER 		0x10        //User segment
#define GDT_ACCESS_TYPE 		0x10        //Non system segment
#define GDT_ACCESS_CODE 		0x08        //Code segment
#define GDT_ACCESS_DATA 		0x00        //Data segment

//! non system segment flags
#define GDT_ACCESS_EXECUTABLE	0x08        //Executable segment
#define GDT_ACCESS_CONFORMING	0x04        //Conforming code segment
#define GDT_ACCESS_DIRECTION	0x04        //Direction bit for data segments
#define GDT_ACCESS_READABLE		0x02        //Readable segment
#define GDT_ACCESS_READWRITE	0x02        //Read/write "data" segment 
#define GDT_ACCESS_ACCESSED		0x01        //Segment accessed

//! system segment flags
#define GDT_ACCESS_SYSTEM		0x00        //System segment
#define GDT_ACCESS_TSS16        0x01        //16-bit TSS segment
#define GDT_ACCESS_LDT          0x02        //LDT segment
#define GDT_ACCESS_TSS16_BUSY   0x03        //Busy 16-bit TSS segment
#define GDT_ACCESS_TSS32        0x09        //32-bit TSS segment
#define GDT_ACCESS_TSS32_BUSY   0x0B        //Busy 32-bit TSS segment

//-----------------------------------------------------------------------------
// 		PUBLIC INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

//! gdt_entry_t The struct represents a single entry in the GDT. Our bootloader
//! has already setup a flat GDT without any protections levels.

typedef struct
{

    //! Lower 16 bits of the limit
    uint16_t        limit_low;

    //! Lower 16 bits of the base address
    uint16_t 	    base_low;

    //! Next 8 bits of the base address
    uint8_t 	    base_middle;

    //! Access flags -> [P, DPL1, DPL0, S, E, DC, RW, A]
    uint8_t 	    access;

    //! Upper 4 bits of the limit and granularity flags
    //! Granularity -> [G, D, 0, A, L3, L2, L1, L0]
    uint8_t 	    granularity;

    //! Upper 8 bits of the base address
    uint8_t 	    base_high;

} __attribute__((packed)) gdt_entry_t;


//! `gdt_ptr_t` Represents the 48bit pointer to the GDT in memory. This
//! structure is used to load the GDT into the CPU's GDTR register.

typedef struct
{

    //! Size of the GDT in bytes - 1
    uint16_t	limit;

    //! Base address of the GDT in memory
    uint32_t	base;

} __attribute__((packed)) gdt_ptr_t;

//-----------------------------------------------------------------------------
// 		INTERFACE PROTOTYPES
//-----------------------------------------------------------------------------

/*******************************************************************************
 *
 * @brief Returns a pointer to the global GDT pointer structure.
 * 
*******************************************************************************/
gdt_ptr_t*      get_gdt_ptr (void);


/*******************************************************************************
*	 
*  @brief   Creates a GDT entry with the specified parameters.
*
*  @param   entry Pointer to the GDT entry to be created.
*  @param   base 32 bit base address of the segment.
*  @param   limit 32 bit Limit of the segment.
*  @param   access 8 bit access flags for the segment.
*  @param   granularity 8 bit granularity flags for the segment.
*	
*******************************************************************************/
void    create_gdt_entry (gdt_entry_t *entry, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t granularity);


/*******************************************************************************
*	 
*  @brief   Initializes the Global Descriptor Table (GDT) using a flat protected
*           mode setup (user mode and kernel mode).
*	
*******************************************************************************/
void    gdt_init_flat_protected ();


/*******************************************************************************
* 
* @brief	Initializes the Global Descriptor Table (GDT) using a flat protected
* 			mode setup (user mode and kernel mode).
* @param 	uint32_t        32 bit pointer to the GDT entries.
*
******************************************************************************/
extern void     load_gdt (uint32_t);

#endif // _GDT_H