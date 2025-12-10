#include <driver/serial.h>
#include <interrupts.h>
#include <stdint.h>
#include <utils.h>
#include <stdbool.h>
#include <stddef.h>

#define LOG_MOD_NAME 	"COM"
#define LOG_MOD_ENABLE 	1
#include <log.h>

/* Implementation private variables */

//! whether the interrupts are enabled or not. this is done to be able to use
//! the serial port before the proper IDT is loaded. in polling, we are supposed
//! to busy wait for the serial port to be ready before writing to it.
static bool 	_serial_mode_intr;

/* Implementation private helper routines.
	TOOD, add support for multiple serial ports. */

//! set the parameters for serial communications
static inline void 	_serial_set_rate (uint32_t baud);	// set baud rate
static inline void 	_serial_set_params (uint8_t data_bits, uint8_t parity_type,
										uint8_t stop_bits); // set line params

//! disable/enable the serial port interrupts
static inline void 	_serial_disable_intrs (void);	// disables all interrupts
static inline void 	_serial_enable_intrs (uint8_t intrs);

//! fifo control routines
static inline void 	_serial_enable_fifo (uint8_t trigger_level);
static inline void 	_serial_disable_fifo (void);

//! write to the modem control register
static inline void 	_serial_set_modemctrl (uint8_t ctrl_bits);

//! checks if a character is available to read
static inline bool 	_serial_byte_avail (void);

//! interrupt handling routine for the serial port
static void 		_serial_intr_handler (interrupt_context_t *context);


/* Implementation of public functions. */

void serial_init (bool intr_mode) {

	_serial_mode_intr = intr_mode;

	//! disable all interrupts at the start
	_serial_disable_intrs ();

	//! we won't use the buffering capapbility of the controller
	_serial_disable_fifo ();

	//! configure to 9600 baud, 8N1	
	_serial_set_rate (SERIAL_DEFAULT_BAUD);
	_serial_set_params (LINECTRL_DATABIT_8, LINECTRL_PARITY_NONE, LINECTRL_STOPBIT_1);

	//! required to enable interrupts in PC, since it is connected to IRQ4
	_serial_set_modemctrl (MODEMCTRL_OUT2);

	/* two modes possible, either no dependence on interrupts mechanism, or via
		polling the line status register in early stages to finish data sending
		and receiving */
	if (intr_mode) {

		_serial_enable_intrs (INTREN_XMIT_EMPTY);
		register_interrupt_handler (IRQ4_SERIAL1, _serial_intr_handler);
		LOG_DEBUG ("Serial interface initialized in interrupt mode\n");
	
	} else {

		LOG_DEBUG ("Serial interface initialized in polling mode\n");
	
	}

}

void serial_putc (char c) {

	/* in polling mode, we only poll for the transmit holding register to be
		empty before writing to it. */
	
	while ( !(inb (SERIAL_LINESTAT(COM1_IO_BASE)) & LINESTAT_THR_EMPTY) )
		;
	outb (c, SERIAL_XMITBUF (COM1_IO_BASE));

}

void serial_puts (const char *str) {

	while (*str) {
		/* needed because qemu needs both for a newline */
		if (*str == '\n') {
			serial_putc ('\r');
		}
		serial_putc (*str++);
	}

}

char serial_getc (void) {

	/* in polling mode, we only poll for the receive buffer to have data
		available before reading from it. */

	while ( !_serial_byte_avail() )
		;
	return inb (SERIAL_RCVBUF (COM1_IO_BASE));

}

/* Private routines implementation. */

static inline void _serial_set_rate (uint32_t baud)
{
	uint16_t divisor = (uint16_t) (SERIAL_BASE_FREQ / baud);

	outb (LINECTRL_DLAB, SERIAL_LINECTRL(COM1_IO_BASE)); // set DLAB
	outb (LOW_BYTE (divisor), SERIAL_DLLSB(COM1_IO_BASE));   // LSB
	outb (HIGH_BYTE (divisor), SERIAL_DLMSB(COM1_IO_BASE)); // MSB
	outb (0x00, SERIAL_LINECTRL(COM1_IO_BASE)); // clear DLAB
}

static inline void _serial_set_params (uint8_t data_bits, uint8_t parity_type,
									   uint8_t stop_bits)
{
	// implicitly clears DLAB
	outb (data_bits | parity_type | stop_bits,
		  SERIAL_LINECTRL(COM1_IO_BASE));
}

static inline void _serial_disable_intrs (void)
{
	outb (0x00, SERIAL_IER(COM1_IO_BASE));
}

static inline void _serial_enable_intrs (uint8_t intrs)
{
	outb (intrs, SERIAL_IER(COM1_IO_BASE));
}

static inline void _serial_enable_fifo (uint8_t trigger_level)
{
	outb (INTRID_FIFO_ENABLED | trigger_level, SERIAL_FIFOCTRL(COM1_IO_BASE));
}

static inline void _serial_disable_fifo (void)
{
	outb (INTRID_FIFO_UNUSABLE, SERIAL_FIFOCTRL(COM1_IO_BASE));
}

static inline void _serial_set_modemctrl (uint8_t ctrl_bits)
{
	outb (ctrl_bits, SERIAL_MODEMCTRL(COM1_IO_BASE));
}

static void _serial_intr_handler (interrupt_context_t* context)
{
	(void) context;
	// identify the interrupt for now and leave
	inb (SERIAL_INTRID(COM1_IO_BASE));
}

static inline bool _serial_byte_avail (void)
{
	return (inb (SERIAL_LINESTAT(COM1_IO_BASE)) & LINESTAT_DATARDY);
}