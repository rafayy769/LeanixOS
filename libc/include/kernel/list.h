#ifndef __LIBC_LIST_H
#define __LIBC_LIST_H
//*****************************************************************************
//*
//*  @file		list.h
//*  @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief		Defines a doubly linked list structure and its operations.
//*  @version	
//*
//****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

typedef int (*list_compare_func_t)(const void*, const void*);

//! defined to get data for the list element
//! type corresponds to the struct type that contains the list element
//! element is the list element pointer
//! member is the name of the list element in the struct
#define LIST_ENTRY(type, element, member) \
	((type*)((uint8_t*)&(element)->next - offsetof (type, member.next)))

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

//! this struct is embedded inside any other struct that needs to be a part
//!  of a list. makes the struct an element of a doubly linked list
typedef struct _list_element {

	//!< pointer to the next element in the list
	struct _list_element* 		next;

	//!< pointer to the previous element in the list
	struct _list_element* 		prev;
	
} list_element_t;

//! this struct represents a doubly linked list
typedef struct _list {
	
	//!< pointer to the first element in the list
	list_element_t* 	head;  
	
	//!< pointer to the last element in the list
	list_element_t* 	tail; 

	//!< number of elements in the list
	size_t 				size;

} list_t;

//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

//! functions to manipulate lists
void 	list_init (list_t*);
void 	list_append (list_t*, list_element_t*);
void 	list_prepend (list_t*, list_element_t*);
list_element_t* list_remove_head (list_t*);
list_element_t* list_remove_tail (list_t*);
void 	list_remove (list_t*, list_element_t*);
void 	list_insert_after (list_t*, list_element_t*, list_element_t*);
void 	list_insert_before (list_t*, list_element_t*, list_element_t*);
void 	list_insert_sorted (list_t*, list_element_t*, list_compare_func_t);
void 	list_clear (list_t*);

//! functions to access lists
bool 			list_is_empty (const list_t*);
size_t 			list_size (const list_t*);
list_element_t* list_head (const list_t*);
list_element_t* list_tail (const list_t*);
list_element_t* list_next (const list_element_t*);
list_element_t* list_prev (const list_element_t*);

//*****************************************************************************
//**
//** 	END list.h
//**
//*****************************************************************************
#endif // _LIST_H