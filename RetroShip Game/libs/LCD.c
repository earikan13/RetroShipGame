/***************************************************************************//**
 * @file    LCD.c
 * @author  Efe Arikan, 4963715
 *          efearikan@sabanciuniv.edu
 * @date    03.05.2021
 *
 * @brief   .c file for HD44780 LCD library, for exercise sheet 1
 *
 * Here goes a detailed description if required.
 ******************************************************************************/

#include "./LCD.h"

/******************************************************************************
 * VARIABLES
 *****************************************************************************/

/******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 *****************************************************************************/
void toggle_pinE(void);
void check_busy_flag(void);
/******************************************************************************
 * LOCAL FUNCTION IMPLEMENTATION
 *****************************************************************************/
void send_msg(int msg, int type) // Function for 8 bit interface with 4 bits
{
    check_busy_flag(); // Check BF first
    if (type == 0) // If command
    {
        P3OUT &= ~PIN_RS;
    }
    else // If for display
    {
        P3OUT |= PIN_RS;
    }
    P2OUT &= ~DATA_LINES;
    P2OUT |= ((msg & 0xF0) >> 4); // Write upper nibble
    toggle_pinE();
    P2OUT &= ~DATA_LINES;
    P2OUT |= (msg & 0x0F); // Write lower nibble
    toggle_pinE();
    __delay_cycles(640);
}

void check_busy_flag() // Function to check if busy flag is high
{
    P2DIR &= ~DB7; // Set DB7 to input
    P3OUT &= ~PIN_RS; // Set RS to 0
    P3OUT |= PIN_RW; // Set RW to 1
    while (1) // Loop until BF is 0
    {
        P3OUT |= PIN_E;
        if (!(P2IN & DB7)) // If cleared
        {
            break;
        }
        P3OUT &= ~PIN_E;
    }
    P3OUT &= ~PIN_RW; // Set RW back to 1
    P2DIR |= DB7; // Set DB7 to output
}

void toggle_pinE(void) // Pulse for data bus
{
    P3OUT |= PIN_E;
    __delay_cycles(640);
    P3OUT &= ~PIN_E;
    __delay_cycles(640);
}
/******************************************************************************
 * FUNCTION IMPLEMENTATION
 *****************************************************************************/
void lcd_init(void)
{
    // Pin arrangements
    P2SEL &= ~DATA_LINES;
    P2SEL2 &= ~DATA_LINES;
    P2DIR |= DATA_LINES;
    P2OUT &= ~DATA_LINES;
    P3SEL &= ~CTR_LINES;
    P3SEL2 &= ~CTR_LINES;
    P3DIR |= CTR_LINES;
    P3OUT &= ~CTR_LINES;

    // Init. commands
    __delay_cycles(50000);
    send_msg(INIT_MSG, 0);
    __delay_cycles(5000);
    toggle_pinE();
    send_msg(INIT_MSG, 0);
    __delay_cycles(500);
    toggle_pinE();
    send_msg(INIT_MSG, 0);
    toggle_pinE();
    __delay_cycles(500);
    P2OUT = 0x02; // Interface is 4 bit
    toggle_pinE();
    __delay_cycles(500);
    send_msg((FOUR_BIT_MSG | TWO_ROWS_MSG), 0);
    send_msg(DISP_CLR_MSG, 0);
    send_msg(ENTRY_MODE_MSG, 0);
    lcd_clear();
    __delay_cycles(5000);
}

void lcd_enable(unsigned char on)
{
    if (on == 0) // Lcd off
    {
        send_msg(LCD_OFF_MSG, 0);
    }
    else if (on == 1) // Lcd on
    {
        send_msg(LCD_ON_MSG, 0);
    }
}

void lcd_cursorSet(unsigned char y, unsigned char x) // DB7 is 1, rest corresponds to cursor address
{
    int a;
    if (y == 0)
    {
        a = 0;
    }
    else
    {
        a = CUR_SEC_ROW;
    }

    a |= x; // Try to fit in the screen if y exceeds 15

    send_msg((SET_POS_MSG | a), 0);
}

void lcd_cursorShow(unsigned char on)
{
    if (on == 0) // cursor off
    {
        send_msg(CUR_OFF_MSG, 0);
    }
    else if (on == 1) // cursor on, cursor blink on
    {
        send_msg(CUR_ON_MSG, 0);
    }
}

void lcd_cursorBlink(unsigned char on)
{
    if (on == 0) // cursor blink off
    {
        send_msg(CUR_BLINK_OFF_MSG, 0);
    }
    else if (on == 1) // cursor blink on
    {
        send_msg(CUR_BLINK_ON_MSG, 0);
    }
}

void lcd_clear(void)
{
    send_msg(LCD_CLEAR_MSG, 0);
    __delay_cycles(25600);
}

void lcd_putChar(char character)
{
    send_msg(character, 1);
}

void lcd_putText(char *text)
{
    unsigned char i = 0;
    while (text[i] != '\0') // Until the end of the string, lcd_putChar every char
    {
        lcd_putChar(text[i]);
        i++;
    }
}

void lcd_putNumber(int number)
{
    int arr[5]; // Max number of digits
    int i = 0;
    int j, r;
    if (number == 0)
    {
        send_msg(48, 1);
    }
    if (number < 0) // If number is negative, print a '-' first, and act as pos. number afterwards
    {
        number *= -1;
        lcd_putChar('-');
    }
    while (number != 0) // Put every digit into arr[]
    {
        r = number % 10;
        arr[i] = r;
        i++;
        number /= 10;
    }
    for (j = i - 1; j > -1; j--) // Print every digit
    {
        send_msg((arr[j] + 48), 1);
    }
}

void lcd_createCustom(char *custom, char loc)
{
    send_msg((0x40 + (loc * 8)), 0);
    int i;
    for (i = 0; i < 8; i++)
    {
        send_msg(custom[i], 1);
    }
    send_msg(0x80, 0);
}
void lcd_dispCustom(char loc)
{
    send_msg(loc, 1);
}
