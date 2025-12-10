#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <mem.h>
#include <driver/vga.h>
#include <driver/keyboard.h>
#include <driver/timer.h>
#include <driver/fdc.h>
#include <driver/ide.h>
#include <driver/serial.h>
#include <driver/block.h>
#include <init/gdt.h>
#include <init/idt.h>
#include <init/tty.h>
#include <mm/vmm.h>
#include <mm/kmm.h>
#include <mm/kheap.h>
#include <init/syscall.h>
#include <proc/process.h>
#include <fs/fat12.h>
#include <fs/hfs.h>
#include <fs/vfs.h>
#include <interrupts.h>
#include <utils.h>
#include <log.h>

#ifdef TESTING
extern void start_tests ();
#endif

extern heap_t 		kernel_heap;  // Kernel heap defined in kheap.c
extern uint32_t 	kernel_start; // from linker script
extern uint32_t 	kernel_end;   // from linker script

//! this is currently a kernel commandline, built for debugging kernel
//! add debug commands as needed to the shell interpreter
extern void shell ();

static void load_msg ();	//! displays the load msg
static void tests_msg ();   //! displays the tests msg

static void hlt ();			//! halts the CPU and disables interrupts

//! helper to zero out the BSS section
static void zero_bss ();

void kmain () 
{

	zero_bss ();	// Zero out the BSS section

	tty_init ();	// Initialize the terminal output interface
	load_msg ();	// Display the kernel message
	serial_init (false); // initialize the serial port in polling mode

	// Kernel log starts here
	LOG_DEBUG ("Kernel size: %d bytes\n", 
			  (uintptr_t)&kernel_end - (uintptr_t)&kernel_start);

	LOG_P ("Initializing system descriptors...\n");
	gdt_init_flat_protected (); // Initialize GDT

	LOG_P ("Initializing system interrupts...\n");
	setup_x86_interrutps (); // Initialize IDT and PIC

	LOG_P ("Initializing terminal input...\n");
	terminal_stdin_init (); // Initialize the terminal input (keyboard)

	LOG_P ("Initializing syscalls interface\n");
	syscall_init (); // Initialize the syscall interface

	//! --- pa1 ^

	LOG_P ("Initializing kernel memory manager...\n");
	kmm_init (); // Initialize the kernel memory manager
	
	LOG_P ("Initializing virtual memory manager...\n");
	vmm_init (); // Initialize the virtual memory manager

	LOG_P ("Initializing kernel heap...\n");
	kheap_init (&kernel_heap, 
				(void*)KERNEL_HEAP_VIRT, KERNEL_HEAP_SIZE,
	 			KERNEL_HEAP_SIZE, true, false); // Initialize Kernel heap
	
	//! --- pa2 ^

	LOG_P ("Initializing system timer at 1000 Hz...\n");
	init_system_timer (1000); // Initialize the system timer with 1000Hz freq

	LOG_P ("Initializing floppy disk controller...\n");
	fdc_init (); // Initialize the floppy disk controller

	LOG_P ("Initializing IDE controller...\n");
	ide_init (); // Initialize the IDE controller

	LOG_P ("Initializing VFS layer...\n");
	vfs_init (); // Initialize the VFS layer

	LOG_P ("Mounting initfs FAT12 on /fd0\n");
	vfs_mount ("fd0", "/fd0", "fat12"); // mount the floppy disk as root fs

	LOG_P ("Formatting and mounting HFS filesystem on /hd1...\n");
	hfs_format ("hd1");
	vfs_mount ("hd1", "/hd1", "hfs");
	
	LOG_P ("Initializing scheduler...\n");
	scheduler_init (); // Initialize the process scheduler

#ifdef TESTING
	LOG_P ("Running kernel tests...\n");
	tests_msg (); // Display the tests message
	start_tests (); // Run kernel tests
#endif

	LOG_P ("Starting init user process...\n");
	process_spawn ("/fd0/SHALL");

	// main thread loops (becomes the idle thread)
	while (1) {asm volatile ("hlt;");}
	
}

void load_msg () {

	printk ("                     ");
	terminal_setbg_color (VGA_COLOR_CYAN);
	terminal_settext_color (VGA_COLOR_WHITE);
	printk (" ~[ leanix1.0 kernel initalizing! ]~ \n\n");
	terminal_reset_color ();
	printk (" +---------------------------------------------------------------------------+\n");

}

void tests_msg () {

	printk ("\n +---------------------------------------------------------------------------+\n");
	printk ("                     ");
	terminal_setbg_color (VGA_COLOR_CYAN);
	terminal_settext_color (VGA_COLOR_RED);
	printk (" ~[ Starting Kernel Tests! ]~ ");
	terminal_reset_color ();
	printk("\n\n");
	// printk (" +---------------------------------------------------------------------------+\n");

}

static void zero_bss () {

	extern char kbss_start, kbss_end; // from linker script
	memset (&kbss_start, 0, &kbss_end - &kbss_start);

}

void hlt () {

	asm volatile ("cli; hlt");

}