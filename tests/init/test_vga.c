#include <stdbool.h>
#include <stddef.h>
#include <driver/vga.h>
#include <stdint.h>
#include <testmain.h>
#include <utils.h>

void test_vga_entry()
{

    int row = 10;
    int col = 20;
    char c = 'x';

    vga_entry_t entry = vga_entry (c, vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));

    vga_putentry_at (entry, col, row);
    vga_entry_t* screen = vga_get_screen_buffer();

    uint8_t correct_color = VGA_COLOR_LIGHT_GREEN | (VGA_COLOR_BLACK << 4);
    if (entry != ((uint16_t) c | (uint16_t) (correct_color << 8))) {
        send_msg ("FAILED: entry encoding incorrect");
        return;
    }

    if (screen[row * VGA_WIDTH + col] == entry) {
        send_msg ("PASSED");
        return;
    }
    
	send_msg ("FAILED: correct entry not found at expected position");
}

void test_vga_cursor()
{

    int      row = 10;
    int      col = 20;
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

void test_vga_entry_overwrite()
{

    int row = 5, col = 5;
    vga_entry_t* screen = vga_get_screen_buffer();

    vga_putentry_at(vga_entry('A', vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK)), col, row);
    vga_putentry_at(vga_entry('B', vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK)), col, row);

    if (screen[row * VGA_WIDTH + col] == vga_entry('B', vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK))) {
        send_msg ("PASSED");
    } else {
        send_msg ("FAILED: entry not overwritten correctly");
    }

}

void test_vga_entry_colors()
{
    int row = 6, col = 10;
    uint8_t color = vga_entry_color(VGA_COLOR_RED, VGA_COLOR_GREEN);
    vga_entry_t entry = vga_entry('C', color);

    vga_putentry_at(entry, col, row);
    vga_entry_t* screen = vga_get_screen_buffer();

    if (screen[row * VGA_WIDTH + col] == entry) {
        send_msg("PASSED");
    } else {
        send_msg("FAILED: entry color incorrect");
    }
}

void test_vga_entry_boundaries()
{
    vga_entry_t* screen = vga_get_screen_buffer();
    vga_entry_t  entry1 = vga_entry('X', vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    vga_entry_t  entry2 = vga_entry('Y', vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

    // top-left
    vga_putentry_at(entry1, 0, 0);
    // bottom-right
    vga_putentry_at(entry2, VGA_WIDTH - 1, VGA_HEIGHT - 1);

    if (screen[0] == entry1 && screen[(VGA_HEIGHT-1) * VGA_WIDTH + (VGA_WIDTH-1)] == entry2) {
        send_msg("PASSED");
    } else {
        send_msg("FAILED: entry not found at expected boundary positions");
    }
}

void test_vga_entry_hidden()
{
    int row = 40;
    int col = 20;
    char c = 'x';

    vga_entry_t entry = vga_entry (c, vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));

    vga_putentry_at (entry, col, row);
    vga_entry_t* screen = vga_get_screen_buffer();

    // uint8_t correct_color = VGA_COLOR_LIGHT_GREEN | (VGA_COLOR_BLACK << 4);

    if (screen[row * VGA_WIDTH + col] != entry) {
        send_msg ("PASSED");
        return;
    }
    
	send_msg ("FAILED: correct entry not found at expected position");
}


void test_vga_cursor_hidden()
{

    int      row = 40;
    int      col = 20;
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
        send_msg ("FAILED");
        return;
    }
    
    send_msg ("PASSED");
}