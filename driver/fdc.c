/* This implementation has been tested on QEMU and works completely fine, but,
	BOCHS has some timing issues with the IRQ6 firing, which causes us to
	spinloop indefinitely. For now, QEMU does the job, but need to make it 
	compatible with BOCHS in future. Can be fixed hopefully by adding sleeps. */

#include <driver/fdc.h>
#include <driver/dma.h>
#include <driver/timer.h>
#include <driver/block.h>
#include <interrupts.h>
#include <mem.h>
#include <stdint.h>
#include <utils.h>

#define LOG_MOD_NAME 	"FDC"
#define LOG_MOD_ENABLE  0
#include <log.h>

/* some constraints for the DMA buffer:
	- buffer address should be < 16 MB, and to be safe should be in low mem
	- the DMA transfer should never cross a 64KB page boundary */
#define FLOPPY_DMA_BUFFER 	DMA_BUFFER_START

/* internally used for all operations, will be updated later to support both
	controllers, however, for the primary controller only, this is fine */
static uint8_t 		_fdc_current_drive = 0; //! current drive number ( <= 3)

/* indicates whether the FDC IRQ has been fired. it is set to 1 when the FDC
	IRQ handler is called, and reset to 0 when the FDC interrupt is 
	acknowledged. */
static volatile uint8_t 	_fdc_irq_fired = 0;

/* stores block device operations for the floppy disk driver for use by the 
	relevant filesystem code */

static block_device_ops_t 	_fdc_block_device_ops;


/* DMA and interrupt control private routines */

//! sets up the DMA for a R/W transfer, using a fixed buffer
static 		  void 	  	_fdc_init_dma (uint16_t count, bool is_write);

//! the FDC IRQ handler
static 		  void 		_fdc_irq_handler (interrupt_context_t* context);

//! waits for the FDC IRQ to be fired
static inline void 		_fdc_wait_for_irq (void);

/* Helper private routines to read and write different registers of FDC. */

//! read MSR register
static inline uint8_t 	_fdc_get_status ();

//! write to Digital Output Register
static inline void	  	_fdc_write_dor (uint8_t value);

//! write to configuration control register
static inline void		_fdc_write_ccr (uint8_t value);

//! send a command to FDC with timeout
static 		  int8_t 	_fdc_send_command (uint8_t command);

//! read from the FIFO data register for results phase of commands
static 		  uint8_t 	_fdc_read_fifo ();

//! enable the primary controller and resets it in DMA mode
static inline void 	  	_fdc_enable ();

//! disable the primary controller
static inline void 	  	_fdc_disable ();

//! enables/disables the motor for the specified drive, includes the sleep
//! to allow the motor to spin up or turn off
static  	  void 		_fdc_control_motor (uint8_t drive, bool enable);

/* Private routines implementing different FDC commands */

//! specify the floppy disk parameters to the fdc
static 	void 	_fdc_fix_drivedata (uint32_t steprate, uint32_t loadtime,
							   		uint32_t unloadtime, bool usedma);

//! check status information about the FDC after commands that issue interrupts
//! also clears the IRQ pin on the FDC
static 	void 	_fdc_sense_interrupt (uint32_t* st0, uint32_t* cyl);

//! recalibrates the drive to move the RW head to cylinder 0
//! necessary after a reset to position the head correctly
static  int32_t _fdc_recalibrate (uint8_t drive);

//! seek to the provided track/cylinder on current drive
static 	int32_t _fdc_seek ( uint8_t cyl, uint8_t head);

//! read a sector from the drive, accepts CHS address
static  void 	_fdc_read_sector_chs (uint8_t head, uint8_t track,
								  	  uint8_t sector);

//! write a sector to the drive, accepts CHS address
static  void 	_fdc_write_sector_chs (uint8_t head, uint8_t track,
								   	  uint8_t sector);

/* Block device functions */

static int32_t _blk_read (void *priv, block_lba_t lba, void *buffer) 
{
	// assumes its fd0 for now
	return fdc_read_sector (lba, (uint8_t*)buffer);
}

static int32_t _blk_write (void *priv, block_lba_t lba, const void *buffer)
{
	// assumes its fd0 for now
	return fdc_write_sector (lba, (const uint8_t*)buffer);
}

/* Implementation of private routines */

static void _fdc_init_dma (uint16_t count, bool is_write) {

	/* the FDC can either work in IRQ mode or in DMA mode for data transfers.
		the DMA channel 2 is pre connected with the FDC, and must be configured
		correctly to use with the FDC. here we are simply setting up the DMA
		params before initiating a transfer. */

	dma_mask_channel (DMA_CHAN_FLOPPY); // disable the channel first
	dma_reset_flipflop (0); 	// reset flip flop on DMA0 (channel2 on DMAC0)
	dma_set_address (DMA_CHAN_FLOPPY, FLOPPY_DMA_BUFFER);	// fixed buffer
	dma_reset_flipflop (0);
	dma_set_count (DMA_CHAN_FLOPPY, count);
	dma_set_external_pagereg (DMA_CHAN_FLOPPY, 0); // top 8 address bits are 0
	if (is_write) {
		dma_setup_write (DMA_CHAN_FLOPPY);
	} else {
		dma_setup_read (DMA_CHAN_FLOPPY);
	}
	dma_unmask_all ();		// unmask all channels on DMA0

}

static inline uint8_t _fdc_get_status () {
	return inb (FDC_PRIM_MSR);	
}

static inline void _fdc_write_dor (uint8_t value) {
	outb (value, FDC_PRIM_DOR);
}

static inline void _fdc_write_ccr (uint8_t value) {
	outb (value, FDC_PRIM_CCR);
}

static int8_t _fdc_send_command (uint8_t command) {

	for (size_t i = 0; i < 500; i++)
	{
		if (_fdc_get_status() & FDC_MSR_DATAREG_MASK) {
			outb (command, FDC_PRIM_FIFO);
			return 0; // Command sent successfully
		}
	}
	
	LOG_ERROR ("_fdc_send_command: timed out\n");
	return -1; // Timeout, command not sent

}

static uint8_t _fdc_read_fifo () {
	for (size_t i = 0; i < 500; i++)
	{
		if (_fdc_get_status() & FDC_MSR_DATAREG_MASK) {
			return inb (FDC_PRIM_FIFO);
		}
	}
	
	LOG_ERROR ("_fdc_read_fifo: timed out\n");
	return 0; // Timeout, no data read
}

static void _fdc_disable ()
{
	_fdc_write_dor (FDC_DOR_DISABLE);
}

static void _fdc_enable ()
{
	// enables the FDC in DMA mode
	_fdc_write_dor (FDC_DOR_ENABLE | FDC_DOR_IRQ_DMA);
}

static void _fdc_control_motor (uint8_t drive, bool enable)
{
	uint8_t motor = 0;
	switch (drive)
	{
	case 0: { motor = FDC_DOR_MOTOR0_MASK; break; }
	case 1: { motor = FDC_DOR_MOTOR1_MASK; break; }
	case 2: { motor = FDC_DOR_MOTOR2_MASK; break; }
	case 3: { motor = FDC_DOR_MOTOR3_MASK; break; }
	default:
		LOG_ERROR ("_fdc_control_motor: invalid drive %d\n", drive);
		return;
	}

	_fdc_write_dor ( enable ? 
		(uint8_t)(motor | drive | FDC_DOR_IRQ_DMA | FDC_DOR_ENABLE) : 
		(FDC_DOR_ENABLE | FDC_DOR_IRQ_DMA) );

	// sleep a bit to allow the motor to spin up/turn off
	sleep (10); //10 ms
}

static void _fdc_irq_handler (interrupt_context_t* context)
{
	/* used for the spinloop.
		clearly, race conditions safe is not the goal right now 
	*/
	_fdc_irq_fired = 1;
	
}

static inline void _fdc_wait_for_irq (void)
{

	while (!_fdc_irq_fired) 	/* busy wait */
		;
	_fdc_irq_fired = 0; 		/* set the flag to zero */

}

/* Implementation of FDC commands */

static void _fdc_read_sector_chs (uint8_t head, uint8_t track, uint8_t sector)
{
	// initialize DMA for read transfer
	_fdc_init_dma (511, false); // 512 bytes (count starts at 511), no write

	// read sector command with extended bits set, multi-track, double density
	// skip deleted
	_fdc_send_command ( FDC_CMD_READ_SECTOR |
						FDC_EXT_DD | FDC_EXT_MT | FDC_EXT_SD );

	/* the read command accepts the following parameters
		- head + drive num (first 3 bits only, top 5 dont care)
		- cylinder
		- head
		- sector
		- sector size (BPS encoding)
		- track length/end of track
		- GAP3
		- data length */
	_fdc_send_command ( (head << 2) | _fdc_current_drive );
	_fdc_send_command ( track );
	_fdc_send_command ( head );
	_fdc_send_command ( sector );
	_fdc_send_command ( FDC_BPS_512 ); // 512 bytes per sector
	// TODO hardcoded, needs update and debugging
	_fdc_send_command ( ((sector + 1) >= 18) ? 18 : (sector + 1) );
	_fdc_send_command ( FDC_GPL_3_5 ); // GAP3 code
	_fdc_send_command ( 0xff );

	_fdc_wait_for_irq (); // wait for the IRQ to be fired

	// read the return data from the fifo (returns 7 bytes)
	for (size_t i = 0; i < 7; i++)
		_fdc_read_fifo ();
	
	// in order to inform the fdc that we are done with the transfer,
	// we need to acknowledge the interrupt
	uint32_t st0, cyl;
	_fdc_sense_interrupt (&st0, &cyl);

}

static void _fdc_write_sector_chs (uint8_t head, uint8_t track, uint8_t sector)
{
	// initialize DMA for write transfer
	_fdc_init_dma (511, true); // 512 bytes (count starts at 511), write mode

	// write sector command with extended bits set, multi-track, double density
	// skip deleted
	_fdc_send_command ( FDC_CMD_WRITE_SECTOR |
						FDC_EXT_DD | FDC_EXT_MT | FDC_EXT_SD );

	/* the write command accepts the following parameters
		- head + drive num (first 3 bits only, top 5 dont care)
		- cylinder
		- head
		- sector
		- sector size (BPS encoding)
		- track length/end of track
		- GAP3
		- data length */
	_fdc_send_command ( (head << 2) | _fdc_current_drive );
	_fdc_send_command ( track );
	_fdc_send_command ( head );
	_fdc_send_command ( sector );
	_fdc_send_command ( FDC_BPS_512 ); // 512 bytes per sector
	// TODO hardcoded, needs update and debugging
	_fdc_send_command ( ((sector + 1) >= 18) ? 18 : (sector + 1) );
	_fdc_send_command ( FDC_GPL_3_5 ); // GAP3 code
	_fdc_send_command ( 0xff );

	_fdc_wait_for_irq (); // wait for the IRQ to be fired

	// read the return data from the fifo (returns 7 bytes)
	for (size_t i = 0; i < 7; i++)
		_fdc_read_fifo ();

	// in order to inform the fdc that we are done with the transfer,
	// we need to acknowledge the interrupt
	uint32_t st0, cyl;
	_fdc_sense_interrupt (&st0, &cyl);

}

static void _fdc_sense_interrupt (uint32_t* st0, uint32_t* cyl)
{
	// send the sense interrupt command
	_fdc_send_command (FDC_CMD_SENSE_INTERRUPT);

	// read the status byte and cylinder number
	if (st0) *st0 = _fdc_read_fifo();
	if (cyl) *cyl = _fdc_read_fifo();
}

static void _fdc_fix_drivedata (uint32_t steprate, uint32_t loadtime,
								uint32_t unloadtime, bool usedma)
{
	// send the fix drive data command
	_fdc_send_command (FDC_CMD_SPECIFY);

	/* the specify command accepts two 1 byte params sequentially:
		- step rate + head unload (4 bits each)
		- head load + NO_DMA? (7 bits + 1 bit) */
	_fdc_send_command ( (uint8_t)
		((steprate & 0x0F) << 4) | ((unloadtime & 0x0F)) );
	_fdc_send_command ( (uint8_t)
		((loadtime & 0x7F) << 1) | (usedma ? 0 : 1) );

	// doesn't return any data	
}

static int32_t _fdc_recalibrate (uint8_t drive)
{
	if (drive > 3)
	{
		LOG_ERROR ("_fdc_recalibrate: invalid drive %d\n", drive);
		return -1;
	}

	// make sure the motor is on first
	_fdc_control_motor (drive, true);

	// we'll do this in a loop
	uint32_t st0, cyl;
	for (size_t i = 0; i < 10; i++)
	{
		_fdc_send_command (FDC_CMD_RECALIBRATE);
		_fdc_send_command (drive);	// accepts only 1 param drive number
		_fdc_wait_for_irq ();
		_fdc_sense_interrupt (&st0, &cyl);
		if (!cyl) {
			_fdc_control_motor (drive, false); // turn the motor off
			return 0;
		}
	}
	
	// failed after several retries
	_fdc_control_motor (drive, false);
	return -1;

}

static int32_t _fdc_seek ( uint8_t cyl, uint8_t head)
{
	uint32_t st0, cyl_returned;
	for (size_t i = 0; i < 10; i++)
	{
		_fdc_send_command (FDC_CMD_SEEK);

		/* accepts two parameters:
			- head + drive
			- cylinder */
		_fdc_send_command ( (head << 2) | _fdc_current_drive );
		_fdc_send_command (cyl);

		_fdc_wait_for_irq (); // wait for the IRQ to be fired
		_fdc_sense_interrupt (&st0, &cyl_returned);
		if (cyl_returned == cyl) {
			LOG_DEBUG ("_fdc_seek: seek to cylinder %d successful\n", cyl);
			return 0; // seek successful
		}
	}

	return -1; // seek failed after several retries
	
}


/* Implementation of interface public functions */

void fdc_init () {

	// setup the irq handler
	register_interrupt_handler (IRQ6_FLOPPY, _fdc_irq_handler);

	// set the current drive to 0
	fdc_set_drive (0);

	// reset the primary controller
	fdc_reset ();

	// register the block device with the kernel
	_fdc_block_device_ops.read  = _blk_read;
	_fdc_block_device_ops.write = _blk_write;
	blkdev_register ("fd0", 512, 2880, &_fdc_block_device_ops, NULL);

}

void fdc_reset ()
{
	_fdc_disable ();
	_fdc_enable ();
	_fdc_wait_for_irq ();

	/* send 4 sense commands for all drives */
	uint32_t st0, cyl;
	for (size_t i = 0; i < 4; i++)
	{
		_fdc_sense_interrupt (&st0, &cyl);
	}
	
	_fdc_write_ccr (FDC_CCR_500KBPS);
	_fdc_fix_drivedata (3, 16, 240, true);
	_fdc_recalibrate (_fdc_current_drive);
}

int32_t fdc_read_sector (uint32_t sectorLBA, uint8_t* buff)
{
	uint32_t cylinder, head, sector;
	fdc_lba_to_chs (sectorLBA, &cylinder, &head, &sector);

	// turn on the motor and seek to the cylinder and head
	_fdc_control_motor (_fdc_current_drive, true);
	if (_fdc_seek(cylinder, head)) {
		LOG_ERROR ("fdc_read_sector: seek failed\n");
		return -1;
	}

	// read the sector from the drive
	_fdc_read_sector_chs ((uint8_t)head, (uint8_t)cylinder, (uint8_t)sector);
	// turn off the motor after reading
	_fdc_control_motor (_fdc_current_drive, false);

	if (buff) {
		// copy the data from the DMA buffer to the provided buffer
		memcpy (buff, (void*)FLOPPY_DMA_BUFFER, 512);
	} else {
		LOG_ERROR ("fdc_read_sector: null buffer passed\n");
		return -1; // null buffer passed
	}

	return 0; // success
}

int32_t fdc_write_sector (uint32_t sectorLBA, const uint8_t* data)
{
	if (!data) {
		LOG_ERROR ("fdc_write_sector: null buffer passed\n");
		return -1; // null buffer passed
	}

	uint32_t cylinder, head, sector;
	fdc_lba_to_chs (sectorLBA, &cylinder, &head, &sector);

	// copy the data to the DMA buffer before writing
	memcpy ((void*)FLOPPY_DMA_BUFFER, data, 512);

	// turn on the motor and seek to the cylinder and head
	_fdc_control_motor (_fdc_current_drive, true);
	if (_fdc_seek(cylinder, head)) {
		LOG_ERROR ("fdc_write_sector: seek failed\n");
		_fdc_control_motor (_fdc_current_drive, false);
		return -1;
	}

	// write the sector to the drive
	_fdc_write_sector_chs ((uint8_t)head, (uint8_t)cylinder, (uint8_t)sector);
	// turn off the motor after writing
	_fdc_control_motor (_fdc_current_drive, false);

	return 0; // success
}

void fdc_lba_to_chs (uint32_t lba, uint32_t* cylinder, 
					 uint32_t* head, uint32_t* sector)
{
	if (!cylinder || !head || !sector) {
		LOG_ERROR ("fdc_lba_to_chs: null pointer passed\n");
		return;
	}

	// calculate the CHS address from the LBA
	*cylinder = (uint8_t)(lba / (18 * 2)); // 18 sectors, 2 heads
	*head = (lba % (2 * 18)) / 18;
	*sector = ((lba % (2 * 18)) % 18) + 1; // sectors are 1-indexed
}

void fdc_set_drive (uint8_t drive)
{
	if (drive > 3)
	{
		LOG_ERROR ("fdc_set_drive: invalid drive %d\n", drive);
		return;
	}

	_fdc_current_drive = drive;
}

uint8_t fdc_get_drive (void)
{
	return _fdc_current_drive;
}
