#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <driver/keyboard.h>
#include <testmain.h>
#include <init/tests.h>

void test_kbd_basic ()
{
	// get the last key pressed
	char key = kbd_getlastkey_buf().ascii;
	char buf[2] = {key, '\0'};
	send_msg (buf);

}

void test_kbd_multi ()
{
	// get the last key pressed
	KBD_ENTRY key = {0, 0};
	char buf[32];
	size_t count = 0;

	while (true) {

		key 	   = kbd_getlastkey_buf ();
		if (key.keycode == KEY_RETURN) break;
		if (!key.ascii) continue;
		buf[count++] = key.ascii;
		if (count >= sizeof(buf)-1) break;

	}

	buf[count] = '\0';
	send_msg (buf);
}

void test_kbd_capslock ()
{
	KBD_KEYCODE key = kbd_getlastkey_buf().keycode;
	if (key != KEY_CAPSLOCK) {
		send_msg ("FAIL: CAPSLOCK key not pressed");
		return;
	}

	char ascii = kbd_getlastkey_buf().ascii;
	char buf[2] = {ascii, '\0'};
	send_msg (buf);
}

void test_kbd_shift ()
{
	char ascii;
	do {
		ascii = kbd_getlastkey_buf().ascii;
	} while (!ascii);
	char buf[2] = {ascii, '\0'};
	send_msg (buf);
}

void test_kbd_ctrl ()
{
	KBD_KEYCODE key = kbd_getlastkey_buf().keycode;
	if (key != KEY_LCTRL && key != KEY_RCTRL) {
		send_msg ("FAIL: CTRL key not pressed");
		return;
	}

	char ascii = kbd_getlastkey_buf().ascii;
	char buf[2] = {ascii, '\0'};
	send_msg (buf);
}