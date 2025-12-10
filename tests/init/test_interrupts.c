#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <interrupts.h>
#include <stdio.h>
#include <testmain.h>


/* internal static vars */

/* this will work fine for now, since no race condition can occur between 
	interrupts at this point. Interrupts should be disabled at the time of 
	entry to an interrupt handler. (Some can choose to reenable interrupts.) */

static uint8_t count[3] = {0, 0, 0}; // each test handler increases its own count

static void _test_isr1 (interrupt_context_t *ctx);
static void _test_isr2 (interrupt_context_t *ctx);
static void _test_isr3 (interrupt_context_t *ctx);


/* Test functions */

void test_intr_reg ()
{
	register_interrupt_handler (0, _test_isr1); //div zero
	register_interrupt_handler (1, _test_isr2); //debug
	register_interrupt_handler (3, _test_isr3); //breakpt

	if (get_interrupt_handler (0) != _test_isr1) {
		send_msg ("FAILED: Handler 0 incorrect\n");
		return;
	} else if (get_interrupt_handler (1) != _test_isr2) {
		send_msg ("FAILED: Handler 1 incorrect\n");
		return;
	} else if (get_interrupt_handler (3) != _test_isr3) {
		send_msg ("FAILED: Handler 3 incorrect\n");
		return;
	}
	
	send_msg ("PASSED");
}


void test_intr_unreg ()
{
	unregister_interrupt_handler (0);
	unregister_interrupt_handler (1);
	unregister_interrupt_handler (3);

	if (get_interrupt_handler (0) != NULL) {
		send_msg ("FAILED: Handler 0 not NULL\n");
		return;
	} else if (get_interrupt_handler (1) != NULL) {
		send_msg ("FAILED: Handler 1 not NULL\n");
		return;
	} else if (get_interrupt_handler (3) != NULL) {
		send_msg ("FAILED: Handler 3 not NULL\n");
		return;
	}

	send_msg ("PASSED");
}

void test_intr_multi ()
{

	register_interrupt_handler(3, _test_isr1); //debug exception
	asm volatile ("int $3");
	unregister_interrupt_handler(3);

	size_t sum = count[0] + count[1] + count[2];
	if (sum != 1) {
		send_msg ("FAILED: registered interrupt handler not called\n");
		return;
	}

	count[0] = 0;
	count[1] = 0;
	count[2] = 0;

	send_msg ("PASSED");

}

static void _test_isr1 (interrupt_context_t *ctx)
{
	count[0]++;
}

static void _test_isr2 (interrupt_context_t *ctx)
{
	count[1]++;
}

static void _test_isr3 (interrupt_context_t *ctx)
{
	count[2]++;
}