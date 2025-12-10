#ifndef _PROCESS_H
#define _PROCESS_H
//*****************************************************************************
//*
//*  @file		process.h
//*  @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief		Defines the process interface for the kernel, which manages
//*             process creation, scheduling, and management.
//*  @version	
//*
//****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <interrupts.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mm/vmm.h>
#include <kernel/list.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

//! process/thread state
typedef enum {

	STATE_RUNNING,
	STATE_READY,
	STATE_BLOCKED,
	STATE_TERMINATED,

	/* these are helpful states, BUT not being used rn */

	STATE_SLEEPING,
	STATE_WAITING

} state_t;

//! Some defines for process management
#define PROCESS_PRI_MIN 		0
#define PROCESS_PRI_MAX 		10
#define PROCESS_PRI_DEFAULT 	5
#define PROCESS_NUM_PRIORITIES  (PROCESS_PRI_MAX - PROCESS_PRI_MIN + 1)

#define PROCESS_NAME_MAX_LEN 	16


#define TIMESLICE_DEFAULT 5		/* default time slice for processes */
#define KSTACK_SIZE       4096	/* default kernel stack size for threads */
#define KCODE_SEG		  0x08	/* kernel code segment selector */
#define KDATA_SEG		  0x10	/* kernel data segment selector */
#define UCODE_SEG		  0x1B	/* user code segment selector */
#define UDATA_SEG		  0x23	/* user data segment selector */

typedef struct _process_control_block process_t;
typedef struct _thread_control_block thread_t;

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

//! The struct represents a single task (each task can have multiple threads).
struct _process_control_block {

	char 		name[ PROCESS_NAME_MAX_LEN ]; //! process name
	uint32_t 	pid;						  //! process id
	int32_t 	priority; 					  //! process priority
	int32_t     time_slice;                   //! process' time slice
	pagedir_t*	pagedir; 					  //! process page directory

	process_t*	parent;						  //! pointer to parent process
	int32_t	 	exit_code;                    //! process exit code

	// thread_t* 	main_thread;              //! main thread of the process
	list_t  	threads;                      //! list of threads in the process
	list_t	  	child_processes;              //! list of child processes

	/* more properties to be added later such as heap info */

	list_element_t list_all; 			  	//! element of the list of processes
	list_element_t list_child;           	//! list of child processes

};

struct _thread_control_block {


	uint32_t 		tid;		//! thread id
	state_t 		state;		//! thread state
	process_t* 		parent;		//! pointer to the owning process

	//! thread kernel stack pointer
	void*			esp0_start;
	void*			kern_esp;		/* current stack top kernel */
	uint32_t 		kstack_size;

	//! thread's saved trap frame
	//! 	note that this points to where the context is saved on the kstack
	interrupt_context_t* trap_frame;

	//! thread entry point and argument (for startup stub)
	void*			entry_point;
	void*			entry_arg;

	//! thread's list element
	list_element_t 	list_element;
	list_element_t  list_all;		/* ready queue member */

};

//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

/* process management functions */

void process_create (process_t* process, const char* name, int32_t priority);
void process_destroy (process_t* process);
int32_t process_spawn (const char* filename);
int32_t process_fork ();
process_t* process_find_by_pid (uint32_t pid);
void process_exit (process_t* process, int32_t status);
thread_t* _get_main_thread (process_t* process);

/* functions relevant for threads */

thread_t* thread_create (process_t* parent_process, void* entry, void* arg);
int32_t thread_destroy (thread_t* thread);
void thread_exit (thread_t* thread);
void thread_start (thread_t* thread);
void thread_cleanup (thread_t* thread);

/* scheduler functions */

void scheduler_init (void);
void scheduler_tick (interrupt_context_t* context);
void scheduler_switch (thread_t* next_thread);
void scheduler_post (thread_t* thread);	/* adds a new thread to the ready queue */
process_t* get_current_proc (void);
thread_t*  get_current_thread (void);


//*****************************************************************************
//**
//** 	END _[filename]
//**
//*****************************************************************************
#endif // _PROCESS_H