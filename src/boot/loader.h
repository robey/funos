#pragma once

#define export __attribute__((visibility ("default")))

extern char *vga_scrollback_buffer;
extern uint32_t vga_scrollback_size;
extern void vga_set_cursor(uint32_t offset);
extern void vga_show_cursor(void);
extern void vga_hide_cursor(void);

extern void serial_write(uint32_t data);
