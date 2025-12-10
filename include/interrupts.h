/**
 * @file interrupts.h
 * @author Abdul Rafay (abdul.rafay@lums.edu.pk)
 * @brief Provides the interface to setup and manage interrupts in the x86 architecture. This includes defining the interrupt context, registering interrupt handlers, and setting up the IDT (Interrupt Descriptor Table) and PIC (Programmable Interrupt Controller).
 * @version 0.1
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _SYS_INTERRUPTS_H
#define _SYS_INTERRUPTS_H

#include <stdint.h>

/**
 * @brief Represents the context/state before servicing an interrupt. Note that this context is saved by the routine called `isr_common_handler` from the file `isr.s`, which is the common handler for all interrupts.
 * The order of the fields in this structure is important, as it matches the order in which the registers are pushed onto the stack by the `isr_common_handler` routine. Note that x86 calling convention dictates parameters are pushed onto the stack in reverse order, so the first parameter is pushed last.
 * 
 */
struct interrupt_context {
    
    /* segment registers */
    uint32_t ds;         // ds segment register

    // The pusha instruction pushes the GP registers in the order specified by the x86 arch.
    uint32_t edi;        // edi gets pushed last
    uint32_t esi;        // 32 bit register esi
    uint32_t ebp;        // 32 bit register ebp
    uint32_t esp;        // 32 bit register esp (value before the `isr_common_handler` routine was called))
    uint32_t ebx;        // 32 bit register ebx
    uint32_t edx;        // 32 bit register edx
    uint32_t ecx;        // 32 bit register ecx
    uint32_t eax;        // 32 bit register eax

    // The following fields were pushed by the generic ISRs wrote in assembly.
    uint32_t interrupt_number; // Interrupt number (0-255)
    uint32_t error_code;       // Error code. pushed by some ISRs or otherwise 0 is pushed manually

    // The following fields are pushed by the CPU automatically when an interrupt occurs.
    uint32_t eip;      // IP (depending on the interrupt, can be the address of the instruction that caused the interrupt or the next instruction to execute)
    uint32_t cs;       // Code segment selector (CS)
    uint32_t eflags;   // EFLAGS register (status flags)
    uint32_t useresp;  // User stack pointer (ESP) if the interrupt was triggered from user mode, otherwise it is the kernel stack pointer
    uint32_t ss;       // Stack segment selector (SS)
};

typedef struct interrupt_context interrupt_context_t;

/**
 * @brief Defines the standard interface used by the interrupt handlers setup by the kernel for each PIC device type maybe. After setting up the context, the generic irq stub should pass the control and context to the corresponding ISR. Update: should be pointer to the context structure.
 * 
 */
typedef void (*interrupt_service_t)(interrupt_context_t*);

/**
 * @brief Sets up interrupts for the x86 machine.
 * 
 */
void setup_x86_interrutps ();

/**
 * @brief Interface to register the interrupt handler for a given interrupt number.
 * 
 * @param int_no Interrupt number for which to set the handler. Note that this interrupt number corresponds to the entry in the IDT for the interrupt
 * @param routine Pointer to the handler function.
 */
void register_interrupt_handler (uint8_t int_no, interrupt_service_t routine);


/**
 * @brief Interface to unregister the interrupt handler for a given interrupt number.
 * 
 */
void unregister_interrupt_handler (uint8_t int_no);

/**
 * @brief Interface to get the interrupt handler for a given interrupt number.
 * 
 */
interrupt_service_t get_interrupt_handler (uint8_t int_no);


/* This is a standard assignment of IRQ pins to the hardware devices in the 
    original PC AT. QEMU simulates that. */

//! defines for the IRQs and interrupt numbers
#define IRQ0_TIMER          32 // Timer interrupt
#define IRQ1_KEYBOARD       33 // Keyboard interrupt
#define IRQ3_SERIAL2        35 // Serial port 2 interrupt
#define IRQ4_SERIAL1        36 // Serial port 1 interrupt
#define IRQ5_PARALLEL2      37 // Parallel port 2 interrupt
#define IRQ6_FLOPPY         38 // Floppy disk controller interrupt
#define IRQ7_PARALLEL1      39 // Parallel port 1 interrupt
#define IRQ8_CMOSRTC        40 // CMOS real-time clock interrupt
#define IRQ9_CGA_VRETRACE   41 // CGA vertical retrace
#define IRQ13_FPU           45 // Floating Point Unit
#define IRQ14_HDC           46 // Hard Disk Controller

//! software generated interrupt for the system call
#define ISR128_SYSCALL    0x80 // syscall interrupt

#endif // !_SYS_INTERRUPTS_H