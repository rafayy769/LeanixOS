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

extern bool run_cmd (char*);


void test_shell_echo(){
    int row = 15;
    int col = 0;
    terminal_move_cursor(col, row);
    char cmd[1024];
    strcat(cmd, "echo");
    char* str = "hello world";
    const char* _cmd = "echo hello world";
    char buf[32]; strncpy(buf, _cmd, 32);
    run_cmd(buf);
    vga_entry_t* screen = vga_get_screen_buffer();
    int offset = (row) * VGA_WIDTH + 0;
    uint8_t colour = terminal_get_colour();
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (screen[i+offset] != vga_entry(str[i], colour)) {
            char c = screen[i + offset] & 0xFF;  // lower 8 bits = character
            (void)c;
            send_msg("FAILED");
            return;
        }
    }
    send_msg ("PASSED");
}

void test_shell_clear(){
    run_cmd("clear");
    vga_entry_t* screen = vga_get_screen_buffer();
    uint8_t color = terminal_get_colour();
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        if (screen[i] != vga_entry(' ', color)) {
            send_msg("FAILED");
            return;
        }
    }
    send_msg ("PASSED");
}

void test_shell_repeat_n(){
    int row = 16;
    int col = 0;
    int n = 10;
    terminal_move_cursor(col, row);
    char cmd[1024];
    strcat(cmd, "repeat 10 hello");
    char* str = "hello world";
    const char* _cmd = "repeat 10 hello world";
    char buf[32]; strncpy(buf, _cmd, 32);
    run_cmd(buf);

    vga_entry_t* screen = vga_get_screen_buffer();
    int offset = (row) * VGA_WIDTH + 0;
    uint8_t colour = terminal_get_colour();
    int len = strlen(str);
    for (int j = 0; j < n;j++){
        for (int i = 0; i < len; i++) {
            if (screen[offset++] != vga_entry(str[i], colour)) {
                send_msg("FAILED");
                return;
            }
        }
        if (screen[offset++] != vga_entry(' ', colour)) {
            send_msg("FAILED");
            return;
        }
    }
    
    send_msg ("PASSED");
}

void test_shell_text_colour() {
    int row = 25;
    int col = 0;
    
    // Move cursor
    terminal_move_cursor(col, row);

    // Set foreground to white
    const char* cmd = "color white";
    char buf[32]; strncpy(buf, cmd, 32);
    run_cmd(buf);

    // Get current VGA color byte
    uint8_t color = terminal_get_colour(); // lower 4 bits = fg, upper 4 bits = bg

    uint8_t fg = color & 0x0F; // extract foreground

    if (fg == 10) {
        send_msg("PASSED");
    } else {
        send_msg("FAILED");
    }
    terminal_setcolor(TTY_DEFAULT_COLOR);
}


void test_shell_bg_colour() {
    int row = 25;
    int col = 0;
    
    // Move cursor
    terminal_move_cursor(col, row);

    // Set foreground to white
    const char* cmd = "bgcolor white";
    char buf[32]; strncpy(buf, cmd, 32);
    run_cmd(buf);

    // Get current VGA color byte
    uint8_t color = terminal_get_colour(); // lower 4 bits = fg, upper 4 bits = bg

    uint8_t bg = (color >> 4) & 0x0F; // extract foreground

    if (bg == 8) {
        send_msg("PASSED");
    } else {
        send_msg("FAILED");
    }
    terminal_setcolor(TTY_DEFAULT_COLOR);
}


