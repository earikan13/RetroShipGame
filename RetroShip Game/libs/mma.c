/***************************************************************************//**
 * @file    mma.c
 * @author  <your name>
 * @date    <date of creation>
 *
 * @brief   <brief description>
 *
 * Here goes a detailed description if required.
 ******************************************************************************/

#include "./mma.h"

/******************************************************************************
 * VARIABLES
 *****************************************************************************/
unsigned char enable4G[] = { REG_XYZ_DATA_CFG, EN_4G };
unsigned char enable2G[] = { REG_XYZ_DATA_CFG, EN_2G };
unsigned char enable8G[] = { REG_XYZ_DATA_CFG, EN_8G };
unsigned char orientation[] = { REG_XYZ_DATA_CFG, 0x40 };
unsigned char reset[] = { REG_CTRL_REG2, 0x40 };
unsigned char enable8Bit[] = { REG_CTRL_REG1, F_READ };
unsigned char enable14Bit[] = { REG_CTRL_REG1, 0x00 };
unsigned char max_rate[] = { REG_CTRL_REG1, 0x01 | 0x04 };
unsigned char self_test[] = { REG_CTRL_REG2, SELF_TEST };
unsigned char de_self_test[] = { REG_CTRL_REG2, 0x00 };
unsigned char high_res[] = { REG_CTRL_REG2, 0x02 };
unsigned char read_x_8Bit[] = { OUT_X_MSB };
unsigned char read_y_8Bit[] = { OUT_Y_MSB };
unsigned char read_z_8Bit[] = { OUT_Z_MSB };
unsigned char whoami[] = { REG_WHOAMI };
unsigned char getrange[] = { REG_XYZ_DATA_CFG };
unsigned char f_stat[] = { REGISTER_F_STATUS };
unsigned char getres[] = { REG_CTRL_REG1 };
unsigned char standby[] = { REG_CTRL_REG1, 0x00 };
unsigned char active[] = { REG_CTRL_REG1, 0x01 };
unsigned char enable_dtap[] = { REG_PULSE_CFG, EN_DTAP };
unsigned char dis_dtap[] = { REG_PULSE_CFG, 0x00 };
unsigned char thsz_32[] = { REG_THSZ, THSZ_TO32 };
unsigned char tmlt[] = { PULSE_TMLT, 0x06 };
unsigned char pulse_ltcy[] = { 0x27, 0x28 };
unsigned char pulse_wind[] = { 0x28, 0x78 };
unsigned char en_int[] = { 0x2D, 0x08 };
unsigned char int1[] = { 0x2E, 0x08 };
unsigned char ipol[] = { 0x2C, 0x02 };
/******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 *****************************************************************************/

/******************************************************************************
 * LOCAL FUNCTION IMPLEMENTATION
 *****************************************************************************/
int mma_get_range(void) // Return the range for measurements eg 2g 4g 8g
{
    unsigned char range[] = { 0 };
    i2c_write(1, getrange, 0);
    i2c_read(1, range);
    return range[0];
}

int mma_get_res(void) // return the resolution eg 8bit or 14 bit
{
    unsigned char res[] = { 0 };
    i2c_write(1, getres, 0);
    i2c_read(1, res);
    if (res[0] & 0b10 == 0) // 14bit
    {
        return 1;
    }
    else // 8bit
    {
        return 0;
    }
}
int new_data(void) // Check if new_data available (probably not working correctly so instead of this i add __delay_cycles())
{
    unsigned char val[] = { 0 };
    i2c_write(1, f_stat, 1);
    i2c_read(1, val);
    if (val[0] & 0b1000 == 0) // no new data
    {
        return 0;
    }
    else // new data ready
    {
        return 1;
    }
}
/******************************************************************************
 * FUNCTION IMPLEMENTATION
 *****************************************************************************/

unsigned char mma_init(void)
{
    unsigned char val[1] = { 0 };
    i2c_init(MMA_ADDR);
    i2c_write(2, reset, 1); // reset to default values
    __delay_cycles(1000000);
    i2c_write(1, whoami, 0); // check if sensor is properly working
    i2c_read(1, val);
    if (val[0] != 0x1A)
        return 1; // if not working properly return 0
    mma_setRange(1);
    return 0;
}

signed char mma_get8X(void)
{
    mma_setResolution(0); // Set resolution to 8bit
    unsigned char val[] = { 0 };
    i2c_write(1, read_x_8Bit, 0);
    i2c_read(1, val);
    int valx = (int) ((val[0] << 8) >> 6);
    return valx;
}
signed char mma_get8Y(void)
{
    mma_setResolution(0);
    unsigned char val[] = { 0 };
    i2c_write(1, read_y_8Bit, 0);
    i2c_read(1, val);
    char valy = (char) val[0];
    return valy;
}
signed char mma_get8Z(void)
{
    mma_setResolution(0);
    unsigned char val[] = { 0 };
    i2c_write(1, read_z_8Bit, 0);
    i2c_read(1, val);
    char valz = (char) val[0];
    return valz;
}
int mma_get14X(void)
{
    mma_setResolution(1); // Set resolution to 14bit
    unsigned char val[2] = { 0, 0 };
    i2c_write(1, read_x_8Bit, 0);
    i2c_read(2, val);
    int val_x;
    val_x = ((signed int) (val[0] << 8 | val[1])) >> 2;
    return val_x;
}
int mma_get14Y(void)
{
    mma_setResolution(1);
    unsigned char val[2] = { 0, 0 };
    i2c_write(1, read_y_8Bit, 0);
    i2c_read(2, val);
    int val_y;
    val_y = ((signed int) (val[0] << 8 | val[1])) >> 2;
    return val_y;
}
int mma_get14Z(void)
{
    mma_setResolution(1);
    unsigned char val[2] = { 0, 0 };
    i2c_write(1, read_z_8Bit, 0);
    i2c_read(2, val);
    int val_z;
    val_z = ((signed int) (val[0] << 8 | val[1])) >> 2;
    return val_z;
}
unsigned char mma_read(void)
{
    int res = mma_get_range;
    if (res)
    {
        int x = mma_get14X();
        int y = mma_get14Y();
        int z = mma_get14Z();
    }
    else
    {
        int x = mma_get8X();
        int y = mma_get8Y();
        int z = mma_get8Z();
    }
}
double mma_getRealX(void)
{
    double val_x = mma_get14X();
    unsigned char range = mma_get_range();
    // Get range and divide the raw value accordingly
    if (range == 0x00)
    {
        val_x /= 1024.0;
    }
    else if (range == 0x01)
    {
        val_x /= 2048.0;
    }
    else if (range == 0x10)
    {
        val_x /= 4096.0;
    }
    return val_x;

}
double mma_getRealY(void)
{
    double val_y = mma_get14Y();
    unsigned char range = mma_get_range();
    if (range == 0x00)
    {
        val_y /= 1024.0;
    }
    else if (range == 0x01)
    {
        val_y /= 2048.0;
    }
    else if (range == 0x10)
    {
        val_y /= 4096.0;
    }
    return val_y;
}
double mma_getRealZ(void)
{
    double val_z = mma_get14Z();
    unsigned char range = mma_get_range();
    if (range == 0x00)
    {
        val_z /= 1024.0;
    }
    else if (range == 0x01)
    {
        val_z /= 2048.0;
    }
    else if (range == 0x10)
    {
        val_z /= 4096.0;
    }
    return val_z;
}
unsigned char mma_selftest(void)
{
    int valx = 0;
    valx = mma_get14X();
    int valy = 0;
    valy = mma_get14Y();
    int valz = 0;
    valz = mma_get14Z();

    i2c_write(2, standby, 1);
    i2c_write(2, self_test, 1);
    i2c_write(2, active, 1);

    __delay_cycles(1000000); // wait certain amount of time for new values

    int test_valx = mma_get14X();
    int test_valy = mma_get14Y();
    int test_valz = mma_get14Z();

    int diffx = test_valx - valx;
    int diffy = test_valy - valy;
    int diffz = test_valz - valz;

    i2c_write(2, standby, 1);
    i2c_write(2, de_self_test, 1);
    i2c_write(2, active, 1);

    if (((diffx >= 130) || (diffx <= 210)) && ((diffy >= 200) || (diffy <= 280))
            && ((diffz >= 1300) || (diffz <= 1600))) // If the values are OK, return 0
        return 0;

    else // Values are not OK
        return 1;
}

unsigned char mma_setRange(unsigned char range)
{
    i2c_write(2, standby, 1);

    if (range == 0)
    {
        i2c_write(2, enable2G, 1);
    }
    else if (range == 1)
    {
        i2c_write(2, enable4G, 1);
    }
    else
    {
        i2c_write(2, enable8G, 1);
    }
    i2c_write(2, active, 1);
}

unsigned char mma_setResolution(unsigned char resolution)
{
    i2c_write(2, standby, 1);
    if (resolution == 0)
    {
        i2c_write(2, enable8Bit, 1);
    }
    else
    {
        i2c_write(2, enable14Bit, 1);
    }
    i2c_write(2, active, 1);
}

unsigned char mma_enableTapInterrupt(void)
{
    i2c_write(2, standby, 1); // Enter to standby
    i2c_write(2, enable_dtap, 1); // Double tap
    i2c_write(2, thsz_32, 1); // 32 counts
    i2c_write(2, tmlt, 1); // 30ms at 200HZ
    i2c_write(2, pulse_ltcy, 1); // 200ms latency
    i2c_write(2, pulse_wind, 1); // 300ms time window for second tap
    i2c_write(2, en_int, 1); // Enable interrupt
    i2c_write(2, int1, 1); // Interrupt on INT1
    i2c_write(2, ipol, 1); // Polarity
    i2c_write(2, active, 1); // Active mode
}
unsigned char mma_disableTapInterrupt(void)
{
    i2c_write(2, standby, 1);
    i2c_write(2, dis_dtap, 1); // Disable interrupt
    i2c_write(2, active, 1);
}
