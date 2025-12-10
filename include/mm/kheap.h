#ifndef _KHEAP_H
#define _KHEAP_H

//*****************************************************************************
//*
//*  @file		[kheap.h]
//*  @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief		Kernel Heap Management Header
//*  @version	
//*
//****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

//! the heap allocator always aligns the sizes to the word size 
#define ALLOCATOR_ALIGNMENT sizeof(void*)

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

typedef struct _free_block_hdr free_block_hdr;

/* the data structure represents a generic heap, underlying algorithm can be 
	anything. */
struct __heap_descriptor {

	void* 			  state;      	  // pointer to the first free block in the heap
	uintptr_t         start;          // start address of the heap
	uintptr_t         end;            // end address of the heap
	uint32_t          max_size;       // maximum size of the heap
	uint8_t           is_supervisor;  // if the heap is supervisor only
	uint8_t           is_readonly;    // if the heap is read-only

};

typedef struct __heap_descriptor heap_t;

//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

//! intializes a memory region as heap
void  		kheap_init( heap_t* heap, void* start, size_t size,
				  		size_t max_size, bool is_supervisor, bool is_readonly );

//! allocates a block of memory from the heap
void* 		kmalloc(heap_t* heap, size_t size);
void* 		malloc(size_t size);	// uses the kernel heap by default

//! frees a block of memory back to the heap
void  		kfree(heap_t* heap, void* ptr);
void 		free(void* ptr);		// uses the kernel heap by default

//! reallocate more memory for a previously allocated region
void* 		krealloc (heap_t* heap, void* ptr, size_t size);
void* 		realloc (void* ptr, size_t size);

//! gets the kernel heap descriptor
heap_t* 	get_kernel_heap(void);

//! display heap usage statistics
void 		kheap_stats(heap_t* heap);

//*****************************************************************************
//**
//** 	END _[filename]
//**
//*****************************************************************************

#endif // !_KHEAP_H