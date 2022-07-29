/***************************************************************************//**
 * @file    LCD.h
 * @author  Efe Arikan, 4963715
 *          efearikan@sabanciuniv.edu
 * @date    03.05.2021
 *
 * @brief   .h file for HD44780 LCD library, for exercise sheet 1
 *
 * Here goes a detailed description if required.
 ******************************************************************************/

#ifndef LIBS_LCD_H_
#define LIBS_LCD_H_

/******************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <msp430g2553.h>

/******************************************************************************
 * CONSTANTS
 *****************************************************************************/
#define DB4 BIT0
#define DB5 BIT1
#define DB6 BIT2
#define DB7 BIT3
#define PIN_RS BIT0
#define PIN_RW BIT1
#define PIN_E BIT2

#define INIT_MSG 0x30
#define FOUR_BIT_MSG 0x20
#define TWO_ROWS_MSG 0x08
#define DISP_CLR_MSG 0x01
#define ENTRY_MODE_MSG 0x06
#define LCD_OFF_MSG 0x08
#define LCD_ON_MSG 0x0C
#define SET_POS_MSG 0x80
#define CUR_SEC_ROW 0x40
#define CUR_OFF_MSG 0x0C
#define CUR_ON_MSG 0x0F
#define CUR_BLINK_OFF_MSG 0x0E
#define CUR_BLINK_ON_MSG 0x0F
#define LCD_CLEAR_MSG 0x01

#define DATA_LINES (DB4 | DB5 | DB6 | DB7)
#define CTR_LINES (PIN_RS | PIN_RW | PIN_E)

/******************************************************************************
 * VARIABLES
 *****************************************************************************/

/******************************************************************************
 * FUNCTION PROTOTYPES
 *****************************************************************************/

/** Initialization */

// Initialization of the LCD; set all pin directions,
// basic setup of the LCD, etc. (1 pt.)
void lcd_init(void);

/** Control functions */

// Enable (1) or disable (0) the display (i.e. hide all text) (0.5 pts.)
void lcd_enable(unsigned char on);

// Set the cursor to a certain x/y-position (0.5 pts.)
void lcd_cursorSet(unsigned char x, unsigned char y);

// Show (1) or hide (0) the cursor (0.5 pts.)
void lcd_cursorShow(unsigned char on);

// Blink (1) or don't blink (0) the cursor (0.5 pts.)
void lcd_cursorBlink(unsigned char on);

/** Data manipulation */

// Delete everything on the LCD (1 pt.)
void lcd_clear(void);

// Put a single character on the display at the cursor's current position (1 pt.)
void lcd_putChar(char character);

// Show a given string on the display. If the text is too long to display,
// don't show the rest (i.e. don't break into the next line) (1 pt.).
void lcd_putText(char *text);

// Show a given number at the cursor's current position.
// Note that this is a signed variable! (1 pt.)
void lcd_putNumber(int number);
void send_msg(int msg, int type);

void lcd_createCustom(char *custom, char loc);
void lcd_dispCustom(char loc);


#endif /* LIBS_LCD_H_ */
