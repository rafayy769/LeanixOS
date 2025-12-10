#ifndef _SERIAL_H
#define _SERIAL_H
//*****************************************************************************
//*
//*  @file		[serial.h]
//*  @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief		Defines the interface for communication with the 16550 UART
//*				serial port.
//*  @version	0.1
//*
//****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

/* Only the first serial port is supported. More ports can be added later, but
	this is sufficient for our purposes. Serial output will practically be used
	for testing and debugging purposes only, and no serial devices are
	technically supported. */

#define COM1_IO_BASE			0x3F8	/* COM1 Base Port */

/* IO registers for serial communications. One small quirk: DLAB (Divisor Latch
	Access Bit) is used to refer to low and high bytes of the divisor. When the
	bit is set, rcv/transmit buffer becomes LSB and MSB respectively of the
	divisor value, otherwise they act as normal registers. */

//! DLAB=0
#define SERIAL_RCVBUF(base)		((base) + 0x00)	// receive buffer (R)
#define SERIAL_XMITBUF(base)	((base) + 0x00)	// transmit buffer (W)
#define SERIAL_IER(base)		((base) + 0x01)	// interrupt enable (R/W)

//! DLAB=1
#define SERIAL_DLLSB(base)		((base) + 0x00)	// divisor latch LSB (R/W)
#define SERIAL_DLMSB(base)		((base) + 0x01)	// divisor latch MSB (R/W)

//! following registers don't care about DLAB
#define SERIAL_INTRID(base)		((base) + 0x02)	// interrupt identification (R)
#define SERIAL_FIFOCTRL(base)	((base) + 0x02)	// FIFO control (W)
#define SERIAL_LINECTRL(base)	((base) + 0x03)	// line control (R/W)
#define SERIAL_MODEMCTRL(base)	((base) + 0x04)	// modem control (R/W)
#define SERIAL_LINESTAT(base)	((base) + 0x05)	// line status (R)
#define SERIAL_MODEMSTAT(base)	((base) + 0x06)	// modem status (R)
#define SERIAL_SCRATCH(base)	((base) + 0x07)	// scratch register (R/W)

/* Line control register is primarily used to specify the line protocol
	parametes to use for communication. We can use the 8bit,no parity,one stop
	bit as the default generally. */

#define LINECTRL_DLAB			0x80	/* Divisor Latch Access Bit */
#define LINECTRL_BREKEN 		0x40	/* Break Enable */

//! bits 3-5 specify parity bits mode to use
#define LINECTRL_PARITY_MASK	0x38
#define LINECTRL_PARITY_NONE	0x00	/* No parity */
#define LINECTRL_PARITY_ODD		0x08	/* Odd parity */
#define LINECTRL_PARITY_EVEN	0x18	/* Even parity */
#define LINECTRL_PARITY_HIGH	0x28	/* Parity bit is always 1 */
#define LINECTRL_PARITY_LOW		0x38	/* Parity bit is always 0 */

//! bit 2 specifies number of stop bits
#define LINECTRL_STOPBIT_MASK	0x04
#define LINECTRL_STOPBIT_1		0x00	/* 1 stop bit */
#define LINECTRL_STOPBIT_2		0x04	/* 1.5 or 2 stop bits */

//! bits 0-1 specify number of data bits
#define LINECTRL_DATABIT_MASK	0x03
#define LINECTRL_DATABIT_5		0x00	/* 5 data bits */
#define LINECTRL_DATABIT_6		0x01	/* 6 data bits */
#define LINECTRL_DATABIT_7		0x02	/* 7 data bits */
#define LINECTRL_DATABIT_8		0x03	/* 8 data bits */

/* Interrupt enable register must be set correctly in order to do communications
	using interrupts. Set the following values to enable interrupts for the 
	corresponding available feature. */

#define INTREN_RCV_DATA			0x01	/* Received Data Available */
#define INTREN_XMIT_EMPTY		0x02	/* Transmission finished */
#define INTREN_RCV_LINESTAT		0x04	/* Receiver Line Status */
#define INTREN_MODEMSTAT		0x08	/* Modem Status */

/* FIFO control register is used for controlling the FIFO buffers. Clear bits
	are used to clear the receive and transmit buffers when set, and they
	set to 0 automatically after clearing is done. */

#define FIFOCTRL_ENABLE			0x01	/* Enable FIFO */
#define FIFOCTRL_CLEAR_RCVBUF	0x02	/* Clear Receive FIFO */
#define FIFOCTRL_CLEAR_XMITBUF	0x04	/* Clear Transmit FIFO */
#define FIFOCTRL_DMA_MODE		0x08	/* DMA Mode Select */
#define FIFOCTRL_TRIGGER_MASK	0xC0	/* Mask for the trigger level bits */

//! trigger level specifies how many bytes must be received before an
//! interrupt is generated
#define FIFOCTRL_TRIGGER_1		0x00	/* 1 byte */
#define FIFOCTRL_TRIGGER_4		0x40	/* 4 bytes */
#define FIFOCTRL_TRIGGER_8		0x80	/* 8 bytes */
#define FIFOCTRL_TRIGGER_14		0xC0	/* 14 bytes */

/* Interrupt identification register's purpose is to identify any pending
	interrupts. */

#define INTRID_NO_INTR			0x01	/* No interrupt pending */
#define INTRID_TIMEOUT			0x80	/* Timeout interrupt pending */

//! bits 1-2 specify the type of interrupt pending
#define INTRID_INTRSTATE_MASK	0x06	/* Mask for the interrupt ID bits */
#define INTRID_MODEM_STAT		0x00	/* Modem Status */
#define INTRID_XMIT_EMPTY		0x02	/* Transmit buffer empty */
#define INTRID_RCV_DATA			0x04	/* Received data available */
#define INTRID_RCV_LINESTAT		0x06	/* Receiver Line Status */

//! bits 6-7 specify the FIFO status
#define INTRID_FIFO_MASK		0xC0	/* Mask for the FIFO status bits */
#define INTRID_FIFO_EMPTY		0x00	/* No FIFO on chip. */
#define INTRID_FIFO_UNUSABLE	0x40	/* FIFO is unusable */
#define INTRID_FIFO_ENABLED 	0x80	/* FIFO is enabled */

/* Modem control register. Only bit3 is relevant which is used to enable the 
	IRQ in PC. */

#define MODEMCTRL_DTR			0x01	/* Data Terminal Ready */
#define MODEMCTRL_RTS			0x02	/* Request To Send */
#define MODEMCTRL_OUT1			0x04	/* Output 1 */
#define MODEMCTRL_OUT2			0x08	/* Output 2 */
#define MODEMCTRL_LOOPBACK		0x10	/* Loopback mode */

/* Line status register is useful for diagnostics of errors or for enabling
	communications via polling. */

#define LINESTAT_DATARDY		0x01	/* Data Ready */
#define LINESTAT_OVERRUN		0x02	/* Overrun Error */
#define LINESTAT_PARITYERR		0x04	/* Parity Error */
#define LINESTAT_FRAMINGERR		0x08	/* Framing Error (stop bit missing) */
#define LINESTAT_BREAK			0x10	/* Break in data input */
#define LINESTAT_THR_EMPTY		0x20	/* Transmitter Holding Register Empty */
#define LINESTAT_TEMT			0x40	/* Transmitter Empty */
#define LINESTAT_ERR			0x80	/* Error in input data */

/* Modem status register bits included only for completeness. */

#define MODEMSTAT_DCTS			0x01	/* Delta Clear To Send */
#define MODEMSTAT_DDSR			0x02	/* Delta Data Set Ready */
#define MODEMSTAT_TERI			0x04	/* Trailing Edge Ring Indicator */
#define MODEMSTAT_DDCD			0x08	/* Delta Data Carrier Detect */
#define MODEMSTAT_CTS			0x10	/* Clear To Send */
#define MODEMSTAT_DSR			0x20	/* Data Set Ready */
#define MODEMSTAT_RI			0x40	/* Ring Indicator */
#define MODEMSTAT_DCD			0x80	/* Data Carrier Detect */


#define SERIAL_BASE_FREQ		115200	/* Base frequency for UART clock */
#define SERIAL_DEFAULT_BAUD		9600	/* Default baud rate for serial port */

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

// TODO add support for multiple serial ports and character device abstraction

//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

//! initialize the serial interface (in polling mode or using interrupts)
void 		serial_init (bool intr_mode);

//! write a character to the serial port
void 		serial_putc (char c);

//! write a string to the serial port
void 		serial_puts (const char *str);

//! read a character from the serial port
char 		serial_getc (void);

//! read a string from the serial port
size_t 		serial_gets (char *buf, size_t buf_size);

//*****************************************************************************
//**
//** 	END _[filename]
//**
//*****************************************************************************

#endif /* _SERIAL_H */