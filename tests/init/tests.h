#ifndef _INIT_TESTS_H
#define _INIT_TESTS_H

/* List of all the exported tests in the module. One can choose to run multiple
	iterations of similar tests in a single function. Each test function
	directly responds with a pass or failure status in these tests. But, that's
	not necessary, and the response can be sent back to the test orchestrator
	to test for custom values too. */

// -- VGA TESTS

extern void test_vga_entry ();
extern void test_vga_cursor ();
extern void test_vga_entry_overwrite ();
extern void test_vga_entry_colors ();
extern void test_vga_entry_boundaries ();
extern void test_vga_entry_hidden ();
extern void test_vga_cursor_hidden ();

// -- INTERRUPT TESTS

extern void test_intr_reg ();
extern void test_intr_unreg ();
extern void test_intr_multi ();

// -- KEYBOARD TESTS

extern void test_kbd_basic ();
extern void test_kbd_multi ();
extern void test_kbd_shift ();
extern void test_kbd_ctrl ();
extern void test_kbd_alt ();
extern void test_kbd_capslock ();
extern void test_kbd_numlock ();
extern void test_kbd_scrolllock ();

// -- TERMINAL TESTS

extern void test_terminal_getc ();
extern void test_terminal_read ();
extern void test_terminal_putc ();
extern void test_terminal_write ();
extern void test_terminal_cursor ();
extern void test_terminal_clear ();
extern void test_terminal_column ();
extern void test_terminal_scroll ();
extern void test_terminal_colour ();
extern void test_terminal_text_color ();
extern void test_terminal_bg_color ();
extern void test_terminal_echo ();

// -- SYSCALL TESTS

extern void test_syscall_register ();
extern void test_syscall_read ();
extern void test_syscall_write ();
extern void test_syscall_read_enforced ();
extern void test_syscall_write_enforced ();

// -- SHELL TESTS

extern void test_shell_echo ();
extern void test_shell_repeat_n ();
extern void test_shell_clear ();
extern void test_shell_text_colour ();
extern void test_shell_bg_colour ();

#endif // _INIT_TESTS_H
