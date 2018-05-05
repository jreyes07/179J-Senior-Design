/*
 * nokia5110.h
 *
 * Created: 11/20/2017 3:13:18 PM
 *  Author: User
 */ 


#ifndef NOKIA5110_H_
#define NOKIA5110_H_

#include <avr/pgmspace.h>
#include <stdint.h>

#define PORT_LCD PORTB
#define DDR_LCD DDRB

/*
 * LCD's pins
 */
#define LCD_SCE PB4
#define LCD_RST PB0
#define LCD_DC PB1
#define LCD_DIN PB5
#define LCD_CLK PB7

#define LCD_CONTRAST 0x40

/*
 * Must be called once before any other function, initializes display
 */
void nokia_lcd_init(void);

/*
 * Clear screen
 */
void nokia_lcd_clear(void);

/**
 * Power of display
 * @lcd: lcd nokia struct
 * @on: 1 - on; 0 - off;
 */
void nokia_lcd_power(uint8_t on);

/**
 * Set single pixel
 * @x: horizontal pozition
 * @y: vertical position
 * @value: show/hide pixel
 */
void nokia_lcd_set_pixel(uint8_t x, uint8_t y, uint8_t value);

/**
 * Draw single char with 1-6 scale
 * @code: char code
 * @scale: size of char
 */
void nokia_lcd_write_char(char code, uint8_t scale);

/**
 * Draw string. Example: writeString("abc",3);
 * @str: sending string
 * @scale: size of text
 */
void nokia_lcd_write_string(const char *str, uint8_t scale);

/**
 * Set cursor position
 * @x: horizontal position
 * @y: vertical position
 */
void nokia_lcd_set_cursor(uint8_t x, uint8_t y);

/*
 * Render screen to display
 */
void nokia_lcd_render(void);




#endif /* NOKIA5110_H_ */