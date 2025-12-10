#include <driver/dma.h>
#include <utils.h>

#define LOG_MOD_NAME 	"DMA"
#define LOG_MOD_ENABLE  1
#include <log.h>

void dma_set_address (uint8_t channel, uint16_t address) {

	uint16_t port = 0;
	switch (channel)
	{

	case 0: { port = DMAC0_CHAN0_ADDR; break;}
	case 1: { port = DMAC0_CHAN1_ADDR; break;}
	case 2: { port = DMAC0_CHAN2_ADDR; break;}
	case 3: { port = DMAC0_CHAN3_ADDR; break;}
	case 4: { port = DMAC1_CHAN4_ADDR; break;}
	case 5: { port = DMAC1_CHAN5_ADDR; break;}
	case 6: { port = DMAC1_CHAN6_ADDR; break;}
	case 7: { port = DMAC1_CHAN7_ADDR; break;}
	
	default:
		LOG_ERROR ("dma_set_address: invalid channel %d\n", channel);
		return;
	}

	// send the low and high byte of the address to the DMA controller
	outb (BYTE_LOW(address), port);
	outb (BYTE_HIGH(address), port);

}

void dma_set_count (uint8_t channel, uint16_t count) {

	uint16_t port = 0;
	switch (channel)
	{

	case 0: { port = DMAC0_CHAN0_COUNT; break;}
	case 1: { port = DMAC0_CHAN1_COUNT; break;}
	case 2: { port = DMAC0_CHAN2_COUNT; break;}
	case 3: { port = DMAC0_CHAN3_COUNT; break;}
	case 4: { port = DMAC1_CHAN4_COUNT; break;}
	case 5: { port = DMAC1_CHAN5_COUNT; break;}
	case 6: { port = DMAC1_CHAN6_COUNT; break;}
	case 7: { port = DMAC1_CHAN7_COUNT; break;}
	
	default:
		LOG_ERROR ("dma_set_count: invalid channel %d\n", channel);
		return;
	}

	// send the low and high byte of the count to the DMA controller
	outb (BYTE_LOW(count), port);
	outb (BYTE_HIGH(count), port);

}

void dma_set_external_pagereg (uint8_t channel, uint8_t page)
{

	uint16_t port = 0;
	switch (channel)
	{

	case 0: { port = DMA_CHAN0_PAGE; break;}
	case 1: { port = DMA_CHAN1_PAGE; break;}
	case 2: { port = DMA_CHAN2_PAGE; break;}
	case 3: { port = DMA_CHAN3_PAGE; break;}
	/* channel 4 page register is never written to */
	case 5: { port = DMA_CHAN5_PAGE; break;}
	case 6: { port = DMA_CHAN6_PAGE; break;}
	case 7: { port = DMA_CHAN7_PAGE; break;}
	
	default:
		LOG_ERROR ("dma_set_external_pagereg: invalid channel %d\n", channel);
		return;
	}

	// send the page register value to the controller
	outb (page, port);

}

void dma_set_mode (uint8_t channel, uint8_t mode)
{
	if (channel > 7) {
		LOG_ERROR ("dma_set_mode: invalid channel %d\n", channel);
		return;
	}

	uint8_t dma_no   = ( channel < 4 ) ? 0 : 1;  // the DMA controller number
	uint8_t chan_no  = ( dma_no==0 ) ? channel : (channel - 4);
	uint8_t mode_reg = ( dma_no==0 ) ? DMAC0_REG_MODE : DMAC1_REG_MODE;

	dma_mask_channel (channel);
	outb ((mode | chan_no), mode_reg);
	dma_unmask_all ();

}

void dma_setup_read (uint8_t channel)
{
	dma_set_mode (channel,
				  DMA_MODE_READ | DMA_MODE_SINGLE );
}

void dma_setup_write (uint8_t channel)
{
	dma_set_mode (channel,
				  DMA_MODE_WRITE | DMA_MODE_SINGLE );
}

void dma_mask_channel (uint8_t channel)
{

	if (channel > 7) {
		LOG_ERROR ("dma_mask_channel: invalid channel %d\n", channel);
		return;
	}

	uint16_t port = (channel < 4) ? DMAC0_REG_CHANMASK : DMAC1_REG_CHANMASK;
	uint8_t  chan = (channel < 4) ? channel : (channel - 4);

	outb (0x04 | chan, port);  // set the mask bit for the channel

}

void dma_unmask_channel (uint8_t channel)
{

	if (channel > 7) {
		LOG_ERROR ("dma_unmask_channel: invalid channel %d\n", channel);
		return;
	}

	uint16_t port = (channel < 4) ? DMAC0_REG_CHANMASK : DMAC1_REG_CHANMASK;
	uint8_t  chan = (channel < 4) ? channel : (channel - 4);

	outb (0x00 | chan, port);  // clear the mask bit for the channel
}

void dma_unmask_all ()
{
	outb (0x00, DMAC0_REG_CLEARMASK); /* only need to clear master */
}

void dma_reset_flipflop (uint8_t dmac)
{
	// doesn't matter what value is written
	outb ( 0xff, (dmac == 0) ? DMAC0_REG_CLEARFLIPFLOP :
							   DMAC1_REG_CLEARFLIPFLOP );
}

void dma_reset (uint8_t dmac)
{
	outb ( 0xff, (dmac == 0) ? DMAC0_REG_MASTERCLEAR : 
							   DMAC1_REG_MASTERCLEAR );
}

void dma_disable (uint8_t dmac)
{
	outb ( DMA_CMD_DISABLE, (dmac == 0) ? DMAC0_REG_COMMAND :
							   			  DMAC1_REG_COMMAND );
}