#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <driver/keyboard.h>
#include <testmain.h>
#include <init/tests.h>
#include <driver/vga.h>
#include <utils.h>
#include <init/tty.h>
#include <driver/serial.h>
#include <interrupts.h>
#include <init/syscall.h>


void test_syscall_register(){
    if (get_interrupt_handler (128) == NULL) {
        send_msg ("FAILED: Handler 128 incorrect\n");
        return;
    }
    send_msg ("PASSED");
}

void test_syscall_read(){
    char buf[15];
    read(buf, 15);
    send_msg (buf);
}

void test_syscall_write(){
    char* str = "hello world";
    int row = 4;
    int col = 4;
    terminal_move_cursor(row,col);
    write(str,strlen(str));
    vga_entry_t* screen = vga_get_screen_buffer();
    int len = strlen(str);
    int passed = 1;
    int offset = row * VGA_WIDTH + col; 
    uint8_t colour = terminal_get_colour();
    for (int i = 0; i < len; i++) {
        if (screen[i+offset] != vga_entry(str[i], colour)) {
            passed = 0;
            break;
        }
    }
    if (passed) {
        send_msg("PASSED");
        return;
    }
    send_msg("FAILED");
}

void test_syscall_read_enforced(){
    interrupt_service_t old = get_interrupt_handler(128);
    unregister_interrupt_handler(128);

    char buf[16];
    memset(buf, 0, sizeof(buf));
    read(buf, 15);

    register_interrupt_handler(128, old);

    if (strlen(buf) > 0) {
        send_msg("FAILED");
        return;
    }
    // clean the buffer
    read(buf, 15);
    send_msg("PASSED");
}

void test_syscall_write_enforced(){
    interrupt_service_t old = get_interrupt_handler(128);
    unregister_interrupt_handler(128);

    int row = 6;
    int col = 6;
    terminal_move_cursor(row, col);

    vga_entry_t* screen = vga_get_screen_buffer();
    int offset = row * VGA_WIDTH + col;
    vga_entry_t before = screen[offset];

    char* str = "Z";
    write(str, 1);

    register_interrupt_handler(128, old);

    vga_entry_t after = screen[offset];
    if (after != before) {
        send_msg("FAILED");
        return;
    }
    send_msg("PASSED");
}