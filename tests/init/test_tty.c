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


void test_terminal_getc(){
    char c = terminal_getc();
    serial_putc(c);
    serial_putc('*');
}

void test_terminal_read(){
    char buffer[100];
    terminal_read(buffer, sizeof(buffer));
    send_msg(buffer);
}

void test_terminal_cursor(){
    int col = 5;
    int row = 2;
    terminal_move_cursor(col, row);
    uint16_t pos = 0;

    vga_move_cursor_to (col,row);

    // Select high byte register (0x0E), then read it
    outb(0x0E, VGA_CRTC_INDEX_PORT);
    pos = ((uint16_t) inb(VGA_CRTC_DATA_PORT)) << 8;

    // Select low byte register (0x0F), then read it
    outb(0x0F, VGA_CRTC_INDEX_PORT);
    pos |= inb(VGA_CRTC_DATA_PORT);
    int row_new = pos / VGA_WIDTH;
    int col_new = pos % VGA_WIDTH;
    if(row == row_new && col == col_new){
        send_msg ("PASSED");
        return;
    }
    
    send_msg ("FAILED: failed to move cursor to expected position");
}

void test_terminal_clear(){
    terminal_clear_scr();
    vga_entry_t* screen = vga_get_screen_buffer();
    uint8_t colour = terminal_get_colour();
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        if (screen[i] != vga_entry(' ', colour)) {
            send_msg("FAILED");
            return;
        }
    }
    uint16_t pos = 0;

    // Select high byte register (0x0E), then read it
    outb(0x0E, VGA_CRTC_INDEX_PORT);
    pos = ((uint16_t) inb(VGA_CRTC_DATA_PORT)) << 8;

    // Select low byte register (0x0F), then read it
    outb(0x0F, VGA_CRTC_INDEX_PORT);
    pos |= inb(VGA_CRTC_DATA_PORT);
    int row_new = pos / VGA_WIDTH;
    int col_new = pos % VGA_WIDTH;
    if(0 == row_new && 0 == col_new){
        send_msg ("PASSED");
        return;
    }
    send_msg("FAILED");
}

void test_terminal_putc(){
    char c = 'a';
    int row = 5;
    int col = 3;
    terminal_move_cursor(col,row);
    terminal_putc(c);
    
    vga_entry_t* screen = vga_get_screen_buffer();
    int offset = row * VGA_WIDTH + col;
    uint8_t colour = terminal_get_colour();
    if (screen[offset] == vga_entry(c,colour)) {
        send_msg("PASSED");
        return;
    }
    send_msg("FAILED");
}

void test_terminal_write(){
    char* str = "hello world";
    int row = 4;
    int col = 4;
    terminal_move_cursor(row,col);
    terminal_write(str, strlen(str));
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

void test_terminal_column(){
    char c= 'd';
    int row = 10;
    terminal_move_cursor(VGA_WIDTH-1,row);
    terminal_putc(c);
    uint16_t pos = 0;

    // Select high byte register (0x0E), then read it
    outb(0x0E, VGA_CRTC_INDEX_PORT);
    pos = ((uint16_t)inb(VGA_CRTC_DATA_PORT)) << 8;

    // Select low byte register (0x0F), then read it
    outb(0x0F, VGA_CRTC_INDEX_PORT);
    pos |= inb(VGA_CRTC_DATA_PORT);
    int row_new = pos / VGA_WIDTH;
    int col_new = pos % VGA_WIDTH;
    if(row+1 == row_new && 0 == col_new){
       send_msg("PASSED");
       return;
    }
    send_msg("FAILED");    
}

void test_terminal_scroll(){
    char c= 'd';
    terminal_move_cursor(0,VGA_HEIGHT-1);
    for (int i=0;i<VGA_WIDTH;i++){

        terminal_putc(c);
    }
    terminal_putc(c);
    vga_entry_t* screen = vga_get_screen_buffer();
    uint16_t pos = 0;

    // Select high byte register (0x0E), then read it
    outb(0x0E, VGA_CRTC_INDEX_PORT);
    pos = ((uint16_t)inb(VGA_CRTC_DATA_PORT)) << 8;

    // Select low byte register (0x0F), then read it
    outb(0x0F, VGA_CRTC_INDEX_PORT);
    pos |= inb(VGA_CRTC_DATA_PORT);
    int row_new = pos / VGA_WIDTH;
    int col_new = pos % VGA_WIDTH;

    if(VGA_HEIGHT-1 == row_new && 1 == col_new){
        uint8_t colour = terminal_get_colour();
        int offset = row_new * VGA_WIDTH + col_new; 
        for (int i = 1; i < VGA_HEIGHT; i++) {
            if (screen[i+offset] != vga_entry(' ', colour)) {
                send_msg("FAILED");
                return;
            }
        }
       send_msg("PASSED");
       return;
    }
    send_msg("FAILED");    

}

void test_terminal_colour(){
    
    terminal_setcolor(0xA2);
    uint8_t colour = terminal_get_colour();
    if (colour != 0xA2){
        send_msg("FAILED");
        return;
    }
    terminal_move_cursor(0,0);
    terminal_putc('x');
    terminal_setcolor(0xA4);
    vga_entry_t* screen = vga_get_screen_buffer();
    if (screen[0] == vga_entry('x', colour)) {
        send_msg("PASSED");
        return;
    }
    send_msg("FAILED");
}

void test_terminal_text_color(){
    
    terminal_setcolor(0xA2);
    terminal_move_cursor(0,0);
    for (int i=0;i<VGA_WIDTH*(VGA_HEIGHT);i++){
        int x = i % VGA_WIDTH;
        int y = i / VGA_WIDTH;
        vga_putentry_at(vga_entry('a',0xA2),x,y);
    }
    terminal_settext_color(4);
    uint8_t colour = terminal_get_colour();
    if (colour != 0xA4){
        send_msg("FAILED");
        return;
    }
    vga_entry_t* screen = vga_get_screen_buffer();
    for (int i=0;i<VGA_WIDTH*VGA_HEIGHT;i++){
       if (screen[i] != vga_entry('a', colour)) {
        send_msg("FAILED");
        return;
    }
    }
    
    send_msg("PASSED");
}

void test_terminal_bg_color(){
    
    terminal_setcolor(0xA2);
    terminal_move_cursor(0,0);
    for (int i=0;i<VGA_WIDTH*(VGA_HEIGHT);i++){
        int x = i % VGA_WIDTH;
        int y = i / VGA_WIDTH;
        vga_putentry_at(vga_entry('a',0xA2),x,y);
    }
    terminal_setbg_color(4);
    uint8_t colour = terminal_get_colour();
    if (colour != 0x42){
        send_msg("FAILED");
        return;
    }
    vga_entry_t* screen = vga_get_screen_buffer();
    for (int i=0;i<VGA_WIDTH*VGA_HEIGHT;i++){
       if (screen[i] != vga_entry('a', colour)) {
        send_msg("FAILED");
        return;
    }
    }
    
    send_msg("PASSED");
}

void test_terminal_echo(){
    
    terminal_move_cursor(0,0);
    char buffer[100];
    terminal_read(buffer, sizeof(buffer));
    int i = 0;
    vga_entry_t* screen = vga_get_screen_buffer();
    uint8_t colour = terminal_get_colour();
	while (buffer[i]) {
		if (screen[i] != vga_entry(buffer[i],colour)) {
            send_msg("FAILED");
            return;
        }
        i++;
	}
    send_msg("PASSED");
    
}