#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

#define VGA_BLACK 			 0
#define VGA_BLUE 			 1
#define VGA_GREEN 			 2
#define VGA_CYAN			 3
#define VGA_RED				 4
#define VGA_MAGENTA			 5
#define VGA_BROWN			 6
#define VGA_LIGHTGRAY		 7
#define VGA_DARKGRAY		 8
#define VGA_LIGHTBLUE		 9
#define VGA_LIGHTGREEN		10
#define VGA_LIGHTCYAN		11
#define VGA_LIGHTRED		12
#define VGA_LIGHTMAGENTA	13
#define VGA_LIGHTBROWN		14
#define VGA_WHITE			15

#define VGA_WIDTH	80
#define VGA_HEIGHT	25

#define make_color(fg, bg) (fg | bg << 4)
#define make_entry(c, col) (c | col << 8)

void term_init();
void term_putc(char c);
void term_puts(const char* str);
void term_setcol(uint8_t fg, uint8_t bg);
void term_putdec(uint32_t val);
void term_puthex(uint32_t val);

#endif
