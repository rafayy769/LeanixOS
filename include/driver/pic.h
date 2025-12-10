#ifndef _PIC_H
#define _PIC_H
//*****************************************************************************
//*
//*  @file		pic.h
//*  @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief		Defines the necessary interface to communicate with Intel 8259A 
//* 			Programmable Interrupt Controller (PIC) driver for x86 PC.
//*  @version	0.1
//*
//****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdint.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES
//-----------------------------------------------------------------------------

/* constants for the PIC ports */

#define MASTER_PIC_COMMAND_PORT  	0x20  // Command port for the master PIC
#define MASTER_PIC_DATA_PORT     	0x21  // Data port for the master PIC
#define SLAVE_PIC_COMMAND_PORT   	0xA0  // Command port for the slave PIC
#define SLAVE_PIC_DATA_PORT      	0xA1  // Data port for the

/* Initialization Command Word 1 (ICW1) bits 
	(7 bit value put in command port of the primary) */

#define PIC_ICW1_EXPECT_ICW4        0x01  // Expect ICW4
#define PIC_ICW1_SINGLE             0x02  // Single PIC mode
#define PIC_ICW1_CALL_ADD_INTERVAL  0x04  // Call address interval bit (ignored on x86)
#define PIC_ICW1_LEVEL_TRIGGERED    0x08  // Use Level triggered mode
#define PIC_ICW1_START_INIT         0x10  // Start initialization

//! for the sake of completeness, the other ICW1 bits are included here, although
//! 	they are not used on the x86 (must be defaulted to 0)

#define _PIC_ICW1_MCS8085_IVT0     	0x20  // MCS-8085 IVT0
#define _PIC_ICW1_MCS8085_IVT1     	0x40  // MCS-8085 IVT1
#define _PIC_ICW1_MCS8085_IVT2     	0x60  // MCS-8085 IVT2

/* Initialization Command Word 2 (ICW2) constants. ICW2 is used to set the 
	base address of the interrupt vectors for the PIC. */

#define PIC_ICW2_IVT_BASE(base) 	((base) & 0xFF)  // IVT base address

/* Initialization Command Word 3 (ICW3) bits. used to let the PICs know what 
	IRQ lines to use when communicating with each other
	ICW3 must be sent after sending an ICW1 with cascading enabled
 	(ICW1_SINGLE not set) */

#define _ICW3_PRIM_MASK 			0xFF  // master PIC pins mask (bits 0-2)
#define _ICW3_SEC_MASK 				0x07  // slave PIC pins mask (bits 0-2)

//! for the master PIC, we provide the pin number for the slave PIC and vice versa

#define PIC_ICW3_PRIM_SLAVE_PIN(pin) ((1 << (pin)) & _ICW3_PRIM_MASK)
#define PIC_ICW3_SEC_MASTER_PIN(pin) ((pin) & _ICW3_SEC_MASK)


/* Initialization Command Word 4 (ICW4) bits. sets the operating mode bits. note
	some of the features may not be present in IBM PC */

#define PIC_ICW4_8086_MODE          0x01  // 8086/88 mode (if not set, 8080/85 mode)
#define PIC_ICW4_AUTO_EOI           0x02  // Auto End of Interrupt (EOI) mode 
#define PIC_ICW4_MASTER_BUF         0x04  // only for master PIC, and if BUF is 
										  //   set, this bit is set to indicate
										  //   that the master PIC is buffered
#define PIC_ICW4_BUFFERED_MODE      0x08  // Use buffered mode
#define PIC_ICW4_SFNM               0x10  // Special Fully Nested Mode (otherwise normal)

/* Operation Command Word (OCW2) defines. the primary operation control word */

#define PIC_OCW2_ROT        		0x80			// rotate bit
#define PIC_OCW2_SEL        		0x40			// select bit
#define PIC_OCW2_EOI        		0x20			// end of interrupt bit
#define PIC_OCW2_LEVEL(x)   		((x) & 0x07)	// intr level (0,1,2)

//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

/**
 * @brief Initializes the 8259 Programmable Interrupt Controller (PIC) for x86.
 * 
 * @param master_base The base address for the master PIC (usually 0x20).
 * @param slave_base The base address for the slave PIC (usually 0x28).
 */
void pic_init (uint8_t master_base, uint8_t slave_base);

/**
 * @brief Sends an End of Interrupt (EOI) signal to the PICs.
 * 
 * @param int_no The interrupt number that has been handled.
 */
void pic_send_eoi (uint8_t int_no);

#endif // !_PIC_H