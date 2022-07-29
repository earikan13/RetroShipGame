/***************************************************************************//**
 * @file    main.c
 * @author  Efe Arikan, 4963715
 *          efearikan@sabanciuniv.edu
 * @date    03.06.2021
 *
 * @brief   .c file of PCF8591T i2c library
 *
 * Here goes a detailed description if required.
 ******************************************************************************/

#include "./adac.h"

/******************************************************************************
 * VARIABLES
 *****************************************************************************/
/******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 *****************************************************************************/

/******************************************************************************
 * LOCAL FUNCTION IMPLEMENTATION
 *****************************************************************************/

/******************************************************************************
 * FUNCTION IMPLEMENTATION
 *****************************************************************************/
unsigned char adac_init() // Initialize ADAC
{
    i2c_init(ADAC_ADDR);
    __delay_cycles(1000);
    return 0;
}

unsigned char adac_read(unsigned char *values)
{
    unsigned char val1[] = { CTRL_read };
    if (!(i2c_write(1, val1, 1))) // First write, read command. If ack continue and return 0
    {
        i2c_read(4, values++); // Read D/A and pass to values
        return 0;
    }
    else
        // If nack return 1
        return 1;
}

unsigned char adac_write(unsigned char value)
{
    unsigned char val2[2] = { CTRL_write, value };
    if (!(i2c_write(2, val2, 1))) // Write, write command and the A/D value respectively. If ack return 0
        return 0;
    else
        // If nack
        return 1;
}
