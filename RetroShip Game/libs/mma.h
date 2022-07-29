/***************************************************************************//**
 * @file    mma.h
 * @author  <your name>
 * @date    <date of creation>
 *
 * @brief   <brief description>
 *
 * Here goes a detailed description if required.
 *
 * @ note   The X/Y-direction printed on the board are wrong:
 //          X is the real Y and Y is the real X. Sorry for the confusion. :-(
 ******************************************************************************/

#ifndef EXERCISE_3_LIBS_MMA_H_
#define EXERCISE_3_LIBS_MMA_H_

/******************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "./i2c.h"

/******************************************************************************
 * CONSTANTS
 *****************************************************************************/
#define MMA_ADDR 0x1D
#define OUT_X_MSB 0x01 //!< Read-only device output register
#define OUT_Y_MSB 0x03 //!< Read-only device output register
#define OUT_Z_MSB 0x05 //!< Read-only device output register
#define OUT_X_LSB 0x02 //!< Read-only device output register
#define OUT_Y_LSB 0x04 //!< Read-only device output register
#define OUT_Z_LSB 0x06 //!< Read-only device output register
#define REG_XYZ_DATA_CFG 0x0E //!< XYZ_DATA_CFG register, sets dynamic range and high-pass filter for
#define REG_PL_STATUS 0x10 //!< PL_STATUS portrait/landscape status register
#define REG_PL_CFG 0x11 //!< Portrait/landscape configuration register
#define REG_CTRL_REG1 0x2A //!< CTRL_REG1 system control 1 register
#define REG_CTRL_REG2 0x2B //!< CTRL_REG2 system control 2 register
#define REG_CTRL_REG4 0x2D //!< CTRL_REG4 system control 4 register
#define REG_CTRL_REG5 0x2E //!< CTRL_REG5 system control 5 register
#define EN_4G 0x01
#define EN_2G 0x00
#define EN_8G 0x02
#define F_READ 0x03
#define SELF_TEST 0x80
#define MAX_8BIT_SIGNED 0xFF
#define MAX_8BIT_VALUE 0x7F
#define REG_WHOAMI 0x0D
#define REGISTER_F_STATUS 0x00
#define REG_PULSE_CFG 0x21
#define EN_DTAP 0x6A
#define REG_THSZ 0x25
#define THSZ_TO32 0x20
#define PULSE_TMLT 0x26
/******************************************************************************
 * VARIABLES
 *****************************************************************************/

/******************************************************************************
 * FUNCTION PROTOTYPES
 *****************************************************************************/

// All configuration functions return 0 if everything went fine
// and anything but 0 if not (they are the ones with a unsigned char return type).
// Initialize the MMA with 8 bit resolution and 4G measurement range (1 pt.)
unsigned char mma_init(void);

// Change the measurement range. (0: 2g, 1: 4g, >1: 8g) (0.5 pt.)
unsigned char mma_setRange(unsigned char range);
// Change the resolution (0: 8 Bit, >= 1: 14 Bit) (0.5 pt.)
unsigned char mma_setResolution(unsigned char resolution);

// Run a self-test on the MMA, verifying that all three axis and all three
// measurement ranges are working. (1 pt.)
/* HINT:
 * The idea of the self test is that you measure the current acceleration values,
 * then enable the on-chip self-test and then read the values again.
 * The values without selftest enabled and those with selftest enabled
 * should now feature a predefined difference (see the datasheet).
 */
unsigned char mma_selftest(void);

// Set up the double tap interrupt on the MMA (do not set up the interrupt on
// the MSP in this function!). This means that the MMA should change the INT1-
// pin whenever a double tap is detected. You may freely choose the axis on
// which the tap has to be received. (You should put a comment in your code,
// which axis you chose, though). (1 pt.)
/* HINT:
 * As the datasheet for the MMA is a bit stingy when it comes to the double
 * tap stuff, so here's (roughly) what you should do:
 *
 *  1) Go to standby (as you can only change the registers when in standby)
 *  2) Write MMA_PULSE_CFG  to enable the z-axis for double tap
 *  3) Write MMA_PULSE_THSZ to set the tap threshold (e.g. to 2g)
 *  4) Write MMA_PULSE_TMLT to set the pulse time limit (e.g. to 100 ms)
 *  5) Write MMA_PULSE_LTCY to set the pulse latency timer (e.g. to 200 ms)
 *  6) Write MMA_PULSE_WIND to set the time window for the second tap
 *  7) Write MMA_CTRL_REG4  to set the pulse interrupt
 *  8) Write MMA_CTRL_REG5  to activate the interrupt on INT1
 *  9) Write MMA_CTRL_REG3  to set the interrupt polarity
 * 10) Return to active mode
 */
unsigned char mma_enableTapInterrupt(void);
// Disable the double-tap-interrupt on the MMA. (0.5 pt.)
unsigned char mma_disableTapInterrupt(void);

// Read the values of all three axis from the chip and store the values
// internally. Take the requested resolution into account. (1 pt.)
unsigned char mma_read(void);

/* Get Functions (1 pt. total): */

// Return the appropriate 8 bit values
// If the resolution during mma_read was 14 bit, translate the data to 8 bit
signed char mma_get8X(void);
signed char mma_get8Y(void);
signed char mma_get8Z(void);

// Return the appropriate 14 bit values
// If the resolution during mma_read was 8 bit, translate the data to 14 bit
int mma_get14X(void);
int mma_get14Y(void);
int mma_get14Z(void);

// Return the appropriate values in m*s^-2.
double mma_getRealX(void);
double mma_getRealY(void);
double mma_getRealZ(void);

#endif /* EXERCISE_3_LIBS_MMA_H_ */
