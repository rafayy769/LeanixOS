/**
 * @file pic.c
 * @author Abdul Rafay (abdul.rafay@lums.edu.pk)
 * @brief Contains definitions and functions to setup the 8259 Programmable Interrupt Controller (PIC).
 * 
 * @version 0.1
 * @date 2025-06-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#include <stdint.h>

#include <utils.h>
#include <driver/pic.h>

void pic_init (uint8_t master_base, uint8_t slave_base)
{
    // step 1. send an ICW1 to both PICs
    outb(PIC_ICW1_START_INIT | PIC_ICW1_EXPECT_ICW4, MASTER_PIC_COMMAND_PORT);  // Master PIC ICW1
    outb(PIC_ICW1_START_INIT | PIC_ICW1_EXPECT_ICW4, SLAVE_PIC_COMMAND_PORT);   // Slave PIC ICW1

    // step 2. remap the base address of the PICs using ICW2 (send to data ports)
    outb(PIC_ICW2_IVT_BASE(master_base), MASTER_PIC_DATA_PORT);  // Master PIC ICW2 (IVT base address 0x20)
    outb(PIC_ICW2_IVT_BASE(slave_base), SLAVE_PIC_DATA_PORT);   // Slave PIC ICW2 (IVT base address 0x28)

    // step 3. send an ICW3 to both PICs. on x86, slave PIC is connected to pin 2 of the master PIC
    outb(PIC_ICW3_PRIM_SLAVE_PIN(2), MASTER_PIC_DATA_PORT);
    outb(PIC_ICW3_SEC_MASTER_PIN(2), SLAVE_PIC_DATA_PORT);

    // step 4. send an ICW4 to both PICs
    outb(PIC_ICW4_8086_MODE, MASTER_PIC_DATA_PORT);  // Master PIC ICW4 (8086 mode, auto EOI)
    outb(PIC_ICW4_8086_MODE, SLAVE_PIC_DATA_PORT);   // Slave PIC ICW4 (8086 mode, auto EOI)

    // step 5. null out the data ports
    outb(0x00, MASTER_PIC_DATA_PORT);  // Clear master PIC data port
    outb(0x00, SLAVE_PIC_DATA_PORT);   // Clear slave PIC data port
}

void pic_send_eoi (uint8_t int_no)
{
    if (int_no >= 0x28) {

        outb (PIC_OCW2_EOI, SLAVE_PIC_COMMAND_PORT);
    
    }
    
    outb (PIC_OCW2_EOI, MASTER_PIC_COMMAND_PORT);
}