#include <stdint.h>
#include <stdarg.h>
#include "terminal.h"
#include "common.h"
#include "task.h"

uint8_t term_row;
uint8_t term_col;
uint16_t term_vgacol;
uint16_t* term_buf;

void move_cursor()
{
	uint32_t temp = (term_row * VGA_WIDTH) + term_col;
	outb(0x3D4, 14);
	outb(0x3D5, (uint8_t)(temp >> 8));
	outb(0x3D4, 15);
	outb(0x3D5, (uint8_t)temp);
}

void term_init()
{
	// Set constants
	term_row = 0;
	term_col = 0;
	term_vgacol = make_color(VGA_LIGHTGRAY, VGA_BLACK);
	term_buf = (uint16_t*)0xB8000;
	
	// Clear the screen
	for (uint8_t y = 0; y < VGA_HEIGHT; y++)
		for (uint8_t x = 0; x < VGA_WIDTH; x++)
			term_buf[(y * VGA_WIDTH) + x] = make_entry(' ', term_vgacol);
}

void term_setcol(uint8_t fg, uint8_t bg)
{
	term_vgacol = make_color(fg, bg);
}

#define TAB_WIDTH 4
void term_putc(char c)
{
	#ifdef DEBUG
	bochs_putc(c);
	#endif
	switch (c)
	{
	case '\n':
		term_row++;
		term_col = 0;
		break;
		
	case '\b':
		if (term_col > 0)
			term_col--;
		break;
	
	case '\t':
		term_col += TAB_WIDTH - (term_col % TAB_WIDTH);
		break;
		
	default:
		term_buf[(term_row * VGA_WIDTH) + term_col] = make_entry(c, term_vgacol);
		term_col++;
		break;
	}
	
	if (term_col >= VGA_WIDTH)
	{
		term_col = 0;
		term_row++;
	}
	
	if (term_row >= VGA_HEIGHT)
	{
		for (uint16_t i = VGA_WIDTH; i < VGA_WIDTH * VGA_HEIGHT; i++)
		{
			term_buf[i - VGA_WIDTH] = term_buf[i];
		}
		for (uint16_t i = (VGA_WIDTH * VGA_HEIGHT) - VGA_WIDTH; i < VGA_WIDTH * VGA_HEIGHT; i++)
			term_buf[i] = make_entry(' ', term_vgacol);
		term_row--;
	}
	move_cursor();
}

void term_puts(const char* str)
{
	uint32_t i = 0;
	while (str[i] != 0)
	{
		term_putc(str[i]);
		i++;
	}
}

void term_putdec(uint32_t val)
{
	char buffer[12];
	int8_t bufpos = 0;
	if (val == 0)
	{
		term_putc('0');
		return;
	}
	while (val != 0)
	{
		buffer[bufpos] = '0' + (val % 10);
		val /= 10;
		bufpos++;
	}
	for (bufpos--; bufpos >= 0; bufpos--)
		term_putc(buffer[bufpos]);
}

void term_puthex(uint32_t val)
{
	if (val == 0)
	{
		term_puts("00000000");
		return;
	}
	uint8_t byte;
	for (int i = 0; i < 8; i++)
	{
		byte = ((val & 0xF0000000) >> 28);
		if (byte < 10)
			term_putc('0' + byte);
		else
			term_putc('A' + byte - 10);
		val <<= 4;
	}
}

void kprintf(const char* format, ...)
{
	
	
	va_list lst;
	va_start(lst, format);
	
	uint32_t i = 0;
	
	uint32_t val;
	char* str;
	
	while (format[i] != 0)
	{
		if (format[i] == '%')
		{
			i++;
			switch (format[i])
			{
				case 'x':
					val = va_arg(lst, uint32_t);
					term_puthex(val);
					break;
				case 'i':
					val = va_arg(lst, uint32_t);
					term_putdec(val);
					break;
				case 's':
					str = va_arg(lst, char*);
					term_puts(str);
					break;
				case 'b':
					val = va_arg(lst, uint32_t);
					if (val)
						term_puts("true");
					else
						term_puts("false");
					break;
					
					
				case '%':
					term_putc('%');
					break;
			}
		} else term_putc(format[i]);
		i++;
	}
	
	va_end(lst);
}
