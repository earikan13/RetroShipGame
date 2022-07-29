/***************************************************************************//**
 *@file    main.c
 * @author  Efe Arikan, 4963715
 *          efearikan@sabanciuniv.edu
 * @date    03.06.2021
 *
 * @brief   .c file for i2c library
 *
 * Here goes a detailed description if required.
 ******************************************************************************/

#include "./i2c.h"

/******************************************************************************
 * VARIABLES
 *****************************************************************************/

// A variable to be set by your interrupt service routine:
// 1 if all bytes have been sent, 0 if transmission is still ongoing.
unsigned char transferFinished = 0;
unsigned char *tx_data_buffer, *rx_data_buffer;
int tx_data_length, rx_data_length;
int tx_stop_flag, nack_flag;
/******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 *****************************************************************************/

/******************************************************************************
 * LOCAL FUNCTION IMPLEMENTATION
 *****************************************************************************/

/******************************************************************************
 * FUNCTION IMPLEMENTATION
 *****************************************************************************/

// TODO: Implement these functions:
void i2c_init(unsigned char addr)
{
    P1SEL |= (XSCL | XSDA); // SCL and SDA pins
    P1SEL2 |= (XSCL | XSDA);
    UCB0CTL1 |= UCSWRST;    // SW reset enabled
    UCB0CTL0 = (UCMST | UCMODE_3 | UCSYNC); // Master and sync mode
    UCB0CTL1 = UCSSEL_2;    // SMCLK
    UCB0BR0 = 160;  //~100kHz
    UCB0BR1 = 0;
    UCB0I2CSA = addr;
    UCB0CTL1 &= ~UCSWRST;   // Clear SW reset, resume operation
    UCB0I2CIE &= ~(UCSTPIE | UCSTTIE | UCALIE); // Enable RX interrupt
    UCB0I2CIE |= UCNACKIE; //enable NACK interrupt
    nack_flag = 0;
    tx_stop_flag = 0;
}

unsigned char i2c_write(unsigned char length, unsigned char *txData,
                        unsigned char stop)
{
    // Before writing, you should always check if the last STOP-condition has already been sent.
    while (UCB0CTL1 & UCTXSTP)
        ;
    transferFinished = 0; // Transfer will begin
    tx_stop_flag = stop;
    nack_flag = 0; // Reset nack flag
    __disable_interrupt();  // Disable interrupts
    IE2 &= ~UCB0RXIE;   // Disable RX interrupt
    IE2 |= UCB0TXIE;    // Enable TX interrupt
    tx_data_buffer = (unsigned char*) txData; // Pass the value that will be written to tx_data_buffer
    tx_data_length = length; // Pass the length
    UCB0CTL1 |= UCTR + UCTXSTT; // Transmitter + Start condition
    __enable_interrupt(); // Enable interrupts
    // Wait for transfer to be finished.
    // Info: In TI's sample code, low-power mode statements are inserted,
    // also waiting for the transfer to be finished.
    while (!transferFinished)
        ;
    return nack_flag; // 0 if ack, 1 if nack
}

void i2c_read(unsigned char length, unsigned char *rxData)
{
    // Before writing, you should always check if the last STOP-condition has already been sent.
    while (UCB0CTL1 & UCTXSTP)
        ;
    transferFinished = 0;   // Transfer will begin
    nack_flag = 0;  // Reset nack flag
    __disable_interrupt();  // Disable interrupts
    IE2 &= ~UCB0TXIE; // Disable TX interrupt
    IE2 |= UCB0RXIE; // Enable RX interrupt
    rx_data_buffer = rxData; // Pass rxData to buffer
    rx_data_length = length; // Pass the length
    UCB0CTL1 &= ~UCTR; // Receiver

    if (length == 1)
    {
        rx_data_length = 0;
        // Todo: If you only want to receive one byte, you instantly have to write a STOP-condition
        UCB0CTL1 |= UCTXSTT;
        while (UCB0CTL1 & UCTXSTT)
            ;
        UCB0CTL1 |= UCTXSTP;
    }
    else // If more than one byte
    {
        rx_data_length = length - 1;
        UCB0CTL1 |= UCTXSTT;
        while (UCB0CTL1 & UCTXSTT)
            ;
    }
    __enable_interrupt(); // Enable interrupts
    // Wait for transfer to be finished.
    // Info: In TI's sample code, low-power mode statements are inserted,
    // also waiting for the transfer to be finished.
    while (!transferFinished)
        ;
}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
    // TODO: Read RX-Buffer or write TX-Buffer, depending on what you'd like to do.
    if (IFG2 & UCB0RXIFG) // Receive interrupt
    {
        if (rx_data_length == 0) // If no byte, write to buffer and finish the transfer
        {
            *rx_data_buffer = UCB0RXBUF;
            transferFinished = 1;
            IFG2 &= ~UCB0TXIFG;
        }
        else // If more than zero byte
        {
            if (rx_data_length == 1) // If one byte send stop cond
            {
                UCB0CTL1 |= UCTXSTP;
            }
            // Pass to buffer and decrease length
            *rx_data_buffer++ = UCB0RXBUF;
            rx_data_length--;
        }
    }
    else // Transmit interrupt
    {
        if (tx_data_length) // If not zero remaining
        {
            UCB0TXBUF = *tx_data_buffer++; // Write to buffer
            tx_data_length--; // Decrease remaining byte
        }
        else // If zero remaining
        {
            IFG2 &= ~UCB0TXIFG; // Clear flag
            if (tx_stop_flag) // If stop flag with i2c_write()
            {
                UCB0CTL1 |= UCTXSTP; // Send stop cond
                while (UCB0CTL1 & UCTXSTP)
                    // Wait untill received
                    ;
            }
            transferFinished = 1; // Mark as transfer finished
        }
    }
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
    // TODO: Check for NACKs
    if (UCB0STAT & UCNACKIFG)
    {
        nack_flag = 1; // Set nack_flag
        UCB0CTL1 |= UCTXSTP; // If NACK received send stop condition
        UCB0STAT &= ~UCNACKIFG; // Clear NACK Flags
    }
}
