//*****************************************************************************
//*
//*  @file       list.c
//*  @author     Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief      Implementation of doubly linked list operations.
//*  @version    1.0
//*  @date       22-07-2025
//*
//****************************************************************************/

// ========== LOGGING CONFIGURATION ==========

#define LOG_MOD_NAME 		"LIST"
#define LOG_MOD_ENABLED 	1
#include <log.h>

// ========== OTHER INCLUDES ==========

#include <stddef.h>
#include <stdbool.h>

#include <kernel/list.h>

//-----------------------------------------------------------------------------
// 		LIST MANIPULATION FUNCTIONS
//-----------------------------------------------------------------------------

/**
 * @brief Initialize a list to an empty state
 * @param list Pointer to the list to initialize
 */
void list_init(list_t* list)
{
    if (!list) {
        return;
    }
    
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

}

/**
 * @brief Add an element to the end of the list
 * @param list Pointer to the list
 * @param element Pointer to the element to add
 */
void list_append(list_t* list, list_element_t* element)
{
    if (!list || !element) {
        return;
    }
    
    element->next = NULL;
    element->prev = list->tail;
    
    if (list->tail) {
        list->tail->next = element;
    } else {
        // List was empty, this is the first element
        list->head = element;
    }
    
    list->tail = element;
    list->size++;
}

list_element_t* list_remove_head (list_t* list)
{
    if (!list || list->size == 0) {
        return NULL;
    }
    
    list_element_t* element = list->head;
    
    list->head = element->next;
    if (list->head) {
        list->head->prev = NULL;
    } else {
        // List is now empty
        list->tail = NULL;
    }
    
    element->next = NULL;
    element->prev = NULL;
    
    list->size--;
    return element;
}

list_element_t* list_remove_tail (list_t* list)
{
    if (!list || list->size == 0) {
        return NULL;
    }
    
    list_element_t* element = list->tail;
    
    list->tail = element->prev;
    if (list->tail) {
        list->tail->next = NULL;
    } else {
        // List is now empty
        list->head = NULL;
    }
    
    element->next = NULL;
    element->prev = NULL;
    
    list->size--;
    return element;
}

/**
 * @brief Add an element to the beginning of the list
 * @param list Pointer to the list
 * @param element Pointer to the element to add
 */
void list_prepend(list_t* list, list_element_t* element)
{
    if (!list || !element) {
        return;
    }
    
    element->prev = NULL;
    element->next = list->head;
    
    if (list->head) {
        list->head->prev = element;
    } else {
        // List was empty, this is the first element
        list->tail = element;
    }
    
    list->head = element;
    list->size++;
}

/**
 * @brief Remove an element from the list
 * @param list Pointer to the list
 * @param element Pointer to the element to remove
 */
void list_remove(list_t* list, list_element_t* element)
{
    if (!list || !element) {
        return;
    }
    
    if (list->size == 0) {
        return;
    }
    
    // Update previous element's next pointer
    if (element->prev) {
        element->prev->next = element->next;
    } else {
        // This was the head element
        list->head = element->next;
    }
    
    // Update next element's previous pointer
    if (element->next) {
        element->next->prev = element->prev;
    } else {
        // This was the tail element
        list->tail = element->prev;
    }
    
    // Clear the element's pointers
    element->next = NULL;
    element->prev = NULL;
    
    list->size--;
}

/**
 * @brief Insert an element after a given element in the list
 * @param list Pointer to the list
 * @param after Element to insert after
 * @param element Element to insert
 */
void list_insert_after(list_t* list, list_element_t* after, list_element_t* element)
{
    if (!list || !after || !element) {
        return;
    }
    
    element->prev = after;
    element->next = after->next;
    
    if (after->next) {
        after->next->prev = element;
    } else {
        // 'after' was the tail, update tail
        list->tail = element;
    }
    
    after->next = element;
    list->size++;
}

/**
 * @brief Insert an element before a given element in the list
 * @param list Pointer to the list
 * @param before Element to insert before
 * @param element Element to insert
 */
void list_insert_before(list_t* list, list_element_t* before, list_element_t* element)
{
    if (!list || !before || !element) {
        return;
    }
    
    element->next = before;
    element->prev = before->prev;
    
    if (before->prev) {
        before->prev->next = element;
    } else {
        // 'before' was the head, update head
        list->head = element;
    }
    
    before->prev = element;
    list->size++;
}

/**
 * @brief Insert an element in sorted order
 * @param list Pointer to the list
 * @param element Element to insert
 * @param compare_func Function to compare elements
 */
void list_insert_sorted(list_t* list, list_element_t* element, list_compare_func_t compare_func)
{
    if (!list || !element || !compare_func) {
        return;
    }
    
    // If list is empty, just append
    if (list->size == 0) {
        list_append(list, element);
        return;
    }
    
    // Find the correct position to insert
    list_element_t* current = list->head;
    while (current) {
        if (compare_func(element, current) <= 0) {
            // Insert before current
            list_insert_before(list, current, element);
            return;
        }
        current = current->next;
    }
    
    // If we get here, element should be inserted at the end
    list_append(list, element);
}

//-----------------------------------------------------------------------------
// 		LIST ACCESS FUNCTIONS
//-----------------------------------------------------------------------------

/**
 * @brief Check if the list is empty
 * @param list Pointer to the list
 * @return true if empty, false otherwise
 */
bool list_is_empty(const list_t* list)
{
    if (!list) {
        return true;
    }
    
    return (list->size == 0);
}

/**
 * @brief Get the number of elements in the list
 * @param list Pointer to the list
 * @return Number of elements in the list
 */
size_t list_size(const list_t* list)
{
    if (!list) {
        return 0;
    }
    
    return list->size;
}

/**
 * @brief Get the first element of the list
 * @param list Pointer to the list
 * @return Pointer to the first element, or NULL if empty
 */
list_element_t* list_head(const list_t* list)
{
    if (!list) {
        return NULL;
    }
    
    return list->head;
}

/**
 * @brief Get the last element of the list
 * @param list Pointer to the list
 * @return Pointer to the last element, or NULL if empty
 */
list_element_t* list_tail(const list_t* list)
{
    if (!list) {
        return NULL;
    }
    
    return list->tail;
}

/**
 * @brief Get the next element in the list
 * @param element Pointer to the current element
 * @return Pointer to the next element, or NULL if at end
 */
list_element_t* list_next(const list_element_t* element)
{
    if (!element) {
        return NULL;
    }
    
    return element->next;
}

/**
 * @brief Get the previous element in the list
 * @param element Pointer to the current element
 * @return Pointer to the previous element, or NULL if at beginning
 */
list_element_t* list_prev(const list_element_t* element)
{
    if (!element) {
        return NULL;
    }
    
    return element->prev;
}
