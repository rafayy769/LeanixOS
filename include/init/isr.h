#ifndef _ISR_H
#define _ISR_H

/*******************************************************************************
 * 
 * @file 	isr.h
 * @author  Abdul Rafay (abdul.rafay@lums.edu.pk)
 * @brief
 * 
 * The stubs for the first 32 CPU exceptions and interrupts + irqs are declared
 *  here.
 * 
 * These are the standard CPU exceptions and interrupts that are defined by the
 *  x86 architecture. These ISRs will be linked externally via the assembly
 *  implementations.
 * 
 * Note that interrupts 0-31 are reserved for CPU exceptions and interrupts, 
 *  and are not used for user-defined interrupts.
 *  Beyond that, you can define your own interrupts in the range 32-255.
 * 
 * **Exception classification:**
 * - Faults: These exceptions can be handled by the operating system, allowing
 *  		 it to recover. Ret address of the instruction that caused the 
 * 			 fault is pushed onto the stack, instead of the next one.
 * - Traps:  A trap is an exception that is reported immediately following
 * 			 the execution of the trapping instruction. Ret address of the next
 * 			 instruction is pushed.
 * - Aborts: An abort is an exception that does not always report the precise
 * 			 location of the instruction that caused the abort. It is used for
 * 			 unrecoverable errors, such as hardware failures. No return address
 * 			 is pushed onto the stack.
 * 
*******************************************************************************/

//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 		PUBLIC INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 		INTERFACE PROTOTYPES
//-----------------------------------------------------------------------------

extern void isr0();    // Division by zero (fault)
extern void isr1();    // Debug (trap)
extern void isr2();    /* Non-maskable interrupt (NMI interrupt) occurs when
							processor's NMI pin is triggered, or the processor
							receives a message on the system bus or the APIC
							bus with a delivery mode of NMI. */
extern void isr3();    // Breakpoint (trap)
extern void isr4();    // Overflow (trap)
extern void isr5();    // Bound range exceeded (fault)
extern void isr6();    // Invalid opcode (fault)
extern void isr7();    // Device not available (fault)
extern void isr8();    // Double fault (abort) pushes an error code (0)
extern void isr9();    // Coprocessor segment overrun (fault)
extern void isr10();   // Invalid TSS (fault) pushes an error code
extern void isr11();   // Segment not present (fault) pushes an error code
extern void isr12();   // Stack segment fault (fault) pushes an error code
extern void isr13();   // General protection fault (fault) pushes an error code
extern void isr14();   // Page fault (fault) pushes an error code
extern void isr15();   // Reserved (fault)
extern void isr16();   // Floating point error (fault)
extern void isr17();   // Alignment check (fault) pushes an error code
extern void isr18();   // Machine check (abort)
extern void isr19();   // SIMD floating point exception (fault)
extern void isr20();   // Virtualization exception (fault)
extern void isr21();   // Control protection exception (fault) pushes error code
extern void isr22();   // Reserved (fault)
extern void isr23();   // Reserved (fault)
extern void isr24();   // Reserved (fault)
extern void isr25();   // Reserved (fault)
extern void isr26();   // Reserved (fault)
extern void isr27();   // Reserved (fault)
extern void isr28();   // Reserved (fault)
extern void isr29();   // Reserved (fault)
extern void isr30();   // Reserved (fault)
extern void isr31();   // Reserved (fault)

/*******************************************************************************
 * 
 * @brief The declarations for the interrupt handling routines for the
 * 			interrupts generated via the PIC (mapped to 32-48 ig, 
 * 			basically right after the CPU specific ISRs)
 * 
*******************************************************************************/


extern void isr32();
extern void isr33();
extern void isr34();
extern void isr35();
extern void isr36();
extern void isr37();
extern void isr38();
extern void isr39();
extern void isr40();
extern void isr41();
extern void isr42();
extern void isr43();
extern void isr44();
extern void isr45();
extern void isr46();
extern void isr47();

//! The syscall interrupt isr
extern void isr128();


#endif // _ISR_H