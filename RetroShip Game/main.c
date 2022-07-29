/***************************************************************************//**
 * @file    main.c
 * @author  Efe Arikan
 *
 * CON5:D4-----CON3:P2.0
 * CON5:D5-----CON3:P2.1
 * CON5:D6-----CON3:P2.2
 * CON5:D7-----CON3:P2.3
 * CON5:RS-----CON4:P3.0
 * CON5:R/W-----CON4:P3.1
 * CON5:E-----CON4:P3.2
 * CON6:I2C_SPI-----CON2:P1.3
 * CON6:XSCL-----CON2:P1.6
 * CON6:XSDA-----CON2:P1.7
 * CON6:UDAC------CON5:BCKL
 * JP2:BKL_ON
 * UPOT-----P1.0
 * BUZZER-----P3.7
 *
 * @note    The project was exported using CCS 8.0.0.
 *
 ******************************************************************************/

#include "libs/templateEMP.h"   // UART disabled, see @note!
#include "libs/mma.h"
#include "libs/lcd.h"
#include "libs/adac.h"

#define POT BIT0 // Potentiometer
#define DIFF_MULT 15 // Difficulty multiplier for the score
#define MIS_SIZE 3
#define METEOR_SIZE 10
#define METEOR_TIMER_LIMIT 10 // Initial meteor timer limit
#define MISSILE_LIMITER 5 // Initial missile speed limiter
#define BUZZER BIT6
#define TOTAL_HP 16
#define MINUS_HP 1

void updatePos(unsigned char adc1, unsigned char adc2, unsigned char adc3,
               int *pos_x, int *pos_y, const char (*rev_ship)[8],
               const char (*ship)[8]);
void checkCollision(char (*missiles)[MIS_SIZE], int (*missile_pos_x)[MIS_SIZE],
                    int (*missile_pos_y)[MIS_SIZE],
                    char (*meteors)[METEOR_SIZE],
                    int (*meteor_pos_x)[METEOR_SIZE],
                    int (*meteor_pos_y)[METEOR_SIZE], int *pos_x, int *pos_y);
void endGame();
int getRNG();
void game_init();
void playSound();
void waitButton();
void meteor_update(char (*meteors)[METEOR_SIZE],
                   int (*meteor_pos_x)[METEOR_SIZE],
                   int (*meteor_pos_y)[METEOR_SIZE], char (*upper_health)[8],
                   char (*lower_health)[8]);
void initTimer();

void missile_update(unsigned char adc3, char (*missiles)[MIS_SIZE],
                    int (*missile_pos_x)[MIS_SIZE],
                    int (*missile_pos_y)[MIS_SIZE], int *pos_x, int *pos_y);
volatile int meteor_timer_cnt = 0; // Counter for meteor limit
volatile int update = 0; // Flag for screen update
volatile int missile_cnt = 0; // Counter for missile limiter
unsigned int level = 3;
unsigned char adc[4] = { 0, 0, 0, 0 }; // [1] for x, [2] for y, [3] for push

int direc = 1; // Direction of the ship
int rand_pos_y = 0;
int diff = 0; // Difficulty
int health = TOTAL_HP;
unsigned long int highscore = 0;
unsigned long int score = 0; // Score
int still_game = 1;
char username[10] = { };
int main(void)
{
    initMSP(); // Initialize with 16MHz
    // I2C
    P1DIR |= BIT3; // 74HCT4066
    P1OUT |= BIT3;
    int x;
    for (x = 0; x < 19; x++) // Try to free the i2c bus, if there was a non finished transfer
    //not sure if it is working correctly
    {
        __delay_cycles(500);
        P1OUT ^= XSCL;
        __delay_cycles(500);
    }
    lcd_init();
    lcd_clear();
    adac_init();
    __delay_cycles(5000);
    game_init();
    // Init ADC
    ADC10CTL0 = ADC10ON + ADC10SHT_0;
    ADC10AE0 |= POT;
    ADC10CTL1 = INCH_0;
    __delay_cycles(1000);
    // Init timers
    initTimer();
    // Buzzer
    TA1CTL = (TASSEL_2 | MC_1); // SMCLK, timer in up-mode
    P3REN &= ~BUZZER;
    P3DIR |= BUZZER;
    P3SEL |= BUZZER;
    __delay_cycles(10000);
    lcd_dispCustom(3);
    lcd_cursorSet(1, 0);
    lcd_dispCustom(4);
    lcd_cursorShow(0); // Dont show&blink cursor
    __delay_cycles(1000);
    int pos_x = 1; // Position of the ship on X
    int pos_y = 0; // Position of the ship on Y
    char missiles[MIS_SIZE] = { 0 }; // To check if a missile exists. 0 for no, 1 for yes
    int missile_pos_x[MIS_SIZE] = { 0 }; // Missile positions on X
    int missile_pos_y[MIS_SIZE] = { 0 }; // Missile positions on Y
    char meteors[METEOR_SIZE] = { 0 }; // To check if a meteor exists. 0 for no, 1 for yes
    int meteor_pos_x[METEOR_SIZE] = { 15 }; // meteor positions on X
    int meteor_pos_y[METEOR_SIZE] = { 0 }; // meteor positions on Y
    char upper_health[8] = { 0b11111, 0b11111, 0b11111, 0b11111, 0b11111,
                             0b11111, 0b11111, 0b11111 };
    char lower_health[8] = { 0b11111, 0b11111, 0b11111, 0b11111, 0b11111,
                             0b11111, 0b11111, 0b11111 };
    const char ship[8] = { 0b00100, 0b00100, 0b00110, 0b01101, 0b01101, 0b00110,
                           0b00100, 0b00100 };
    const char rev_ship[8] = { 0b00100, 0b00100, 0b01100, 0b10110, 0b10110,
                               0b01100, 0b00100, 0b00100 };
    const char meteor[8] = { 0b00000, 0b00000, 0b00000, 0b00100, 0b01110,
                             0b00100, 0b00000, 0b00000 };
    lcd_createCustom(ship, 0);
    lcd_createCustom(meteor, 2);
    lcd_createCustom(upper_health, 3);
    lcd_createCustom(lower_health, 4);
    __delay_cycles(10000);
    while (still_game)
    {
        meteor_update(&meteors, &meteor_pos_x, &meteor_pos_y, &upper_health,
                      &lower_health);
        if (update) // If update is req.
        {
            lcd_clear();
            lcd_dispCustom(3);
            lcd_cursorSet(1, 0);
            lcd_dispCustom(4);
            adac_read(adc); // Read the values and pass them to adc[]
            updatePos(adc[1], adc[2], adc[3], &pos_x, &pos_y, &rev_ship, &ship); // Update the position of the ship based on readings.
            adac_read(adc); // Read the values and pass them to adc[]
            missile_update(adc[3], &missiles, &missile_pos_x, &missile_pos_y,
                           &pos_x, &pos_y);
            lcd_cursorSet(pos_y, pos_x);
            lcd_dispCustom(0);
            unsigned int i = 0;
            for (i = 0; i < METEOR_SIZE; i++) // Update positions of every meteor and every missile
            {
                if (meteors[i]) // If the meteor exists
                {
                    lcd_cursorSet(meteor_pos_y[i], meteor_pos_x[i]);
                    lcd_dispCustom(2);
                    //lcd_putNumber(i);
                }
                if (i < MIS_SIZE)
                {
                    if (missiles[i]) // If the missile exists
                    {
                        lcd_cursorSet(missile_pos_y[i], missile_pos_x[i]);
                        lcd_putChar('-');
                        //lcd_putNumber(i);
                    }
                }
            }
            update = 0;
        }
        checkCollision(&missiles, &missile_pos_x, &missile_pos_y, &meteors,
                       &meteor_pos_x, &meteor_pos_y, &pos_x, &pos_y);
    }
    endGame(); // If game is over
}

// Update position of the ship based on readings.
void updatePos(unsigned char adc1, unsigned char adc2, unsigned char adc3,
               int *pos_x, int *pos_y, const char (*rev_ship)[8],
               const char (*ship)[8])
{
    if (adc2 == 255)
        *pos_y = 1;
    else if (adc2 == 0)
        *pos_y = 0;

    if (adc1 == 255)
    {
        if (*pos_x == 1)
            *pos_x = 1;
        else
            *pos_x -= 1;
        direc = 0;
        lcd_createCustom((*rev_ship), 0);
    }
    else if (adc1 == 0)
    {
        if (*pos_x > 15)
            *pos_x = 15;
        else
            *pos_x += 1;
        direc = 1;
        lcd_createCustom((*ship), 0);
    }
}

void checkCollision(char (*missiles)[MIS_SIZE], int (*missile_pos_x)[MIS_SIZE],
                    int (*missile_pos_y)[MIS_SIZE],
                    char (*meteors)[METEOR_SIZE],
                    int (*meteor_pos_x)[METEOR_SIZE],
                    int (*meteor_pos_y)[METEOR_SIZE], int *pos_x, int *pos_y) // Function to check collisions
{
    unsigned int i;
    for (i = 0; i < METEOR_SIZE; i++)
    {
        if ((*meteors)[i])
        {
            unsigned int j;
            for (j = 0; j < MIS_SIZE; j++)
            {
                // Missile & meteor collision. Clear meteor and missile
                if (((*missiles)[j])
                        && ((*missile_pos_y)[j] == (*meteor_pos_y)[i])
                        && ((*missile_pos_x)[j] == (*meteor_pos_x)[i]))
                {
                    (*missiles)[j] = 0;
                    (*meteors)[i] = 0;
                    score += 1;
                    //playSound();
                    if ((score > DIFF_MULT) && (diff < 6)) // Increase difficulty based on the score.
                    {
                        diff += 1;
                        score = 0;
                    }
                }
            }
            if ((*pos_x == (*meteor_pos_x)[i])
                    && (*pos_y == (*meteor_pos_y)[i])) // Ship & meteor collision. End the game
                still_game = 0;
        }
    }
}
void initTimer()
{
    TA0CCTL0 = CCIE + OUTMOD_0;
    TA0CTL = TASSEL_2 + MC_3 + ID_3;
    TA0CCR0 = 60000;
}
void endGame() // End game screen and memory
{
    TA0CTL = MC_0;
    lcd_clear();
    lcd_putText("Game Over,");
    lcd_putText(username);
    lcd_cursorSet(1, 0);
    lcd_putText("Score:");
    score = score + DIFF_MULT * diff;
    lcd_putNumber(score);
    waitButton();
    lcd_clear();
    lcd_putText("Highscore was:");
    lcd_cursorSet(1, 0);
    lcd_putNumber(highscore);
    waitButton();
    WDTCTL = 0; // Reset
}

int getRNG() // Use the noise last byte of the ADC to generate pseudo-randomnes
{
    ADC10CTL0 |= ENC + ADC10SC;

    return ADC10MEM & 0x01;
}
void game_init()
{
    char chars[] = "abcdefghijklmnopqrstuvwxyz";
    const int size = sizeof(chars) / sizeof(chars[0]);
    unsigned int curr = 0;
    unsigned int name_cnt = 0;
    lcd_putText("Enter your name");
    lcd_cursorSet(1, name_cnt);
    lcd_putChar(chars[curr]);
    lcd_cursorBlink(1);
    char flag = 1;
    unsigned int init_case = 0;
    int prev1 = 1000;
    int prev2 = 1000;
    while (flag)
    {
        adac_read(adc);
        while ((prev1 == adc[1]) || (prev2 == adc[3]))
        {
            adac_read(adc);
        }
        prev1 = adc[1];
        prev2 = 1000;
        switch (init_case)
        {
        case 0: // Choose characters, if button pressed go to case 2
            if (adc[1] == 255)
            {
                if (curr == 0)
                    curr = size - 2;
                else
                    curr -= 1;
                lcd_clear();
                lcd_putText("Enter your name");
                lcd_cursorSet(1, 0);
                lcd_putText(username);
                lcd_putChar(chars[curr]);
                prev1 = 255;
            }
            else if (adc[1] == 0)
            {
                if (curr == size - 2)
                    curr = 0;
                else
                    curr += 1;
                lcd_clear();
                lcd_putText("Enter your name");
                lcd_cursorSet(1, 0);
                lcd_putText(username);
                lcd_putChar(chars[curr]);
                prev1 = 0;
            }
            if (adc[3] == 255)
            {
                init_case = 1;
                username[name_cnt] = chars[curr];
                name_cnt += 1;
                prev2 = 255;
                lcd_cursorBlink(0);
            }
            break;
        case 1:
            if (adc[3] == 255) // Is naming done?
            {
                lcd_cursorBlink(0);
                flag = 0;
            }

            else if (adc[1] == 0) // Continue to the next char
            {
                init_case = 0;
                prev1 = 0;
                curr = 0;
                lcd_cursorSet(1, name_cnt);
                lcd_putChar(chars[curr]);
                lcd_cursorBlink(1);
            }
            else if (adc[1] == 255) // Need to delete a character
            {
                init_case = 2;
                prev1 = 255;
                lcd_cursorBlink(1);
            }
            break;
        case 2: // Delete character
            if (adc[3] == 255)
            {
                curr = 0;
                name_cnt -= 1;
                username[name_cnt] = 0;
                lcd_clear();
                lcd_putText("Enter your name");
                lcd_cursorSet(1, 0);
                lcd_putText(username);
                init_case = 1;
                lcd_cursorBlink(0);
                prev2 = 255;
            }
            break;
        }
    }
    waitButton();
    lcd_clear();
    lcd_cursorBlink(1);
    lcd_putText("Choose a level");
    lcd_cursorSet(1, 0);
    lcd_putNumber(1);
    lcd_cursorSet(1, 15);
    lcd_putNumber(2);
    lcd_cursorSet(1, 7);
    prev1 = 1000;
    prev2 = 1000;
    while (1)
    {
        adac_read(adc);
        while ((prev1 == adc[1]) || (prev2 == adc[3]))
        {
            adac_read(adc);
            __delay_cycles(15000);
        }
        prev1 = adc[1];
        if (adc[1] == 0)
        {
            level = 2;
            lcd_cursorSet(1, 15);
            prev1 = 0;
        }

        else if (adc[1] == 255)
        {
            level = 1;
            lcd_cursorSet(1, 0);
            prev1 = 255;
        }
        if ((adc[3] == 255) && (level != 3))
        {
            lcd_clear();
            lcd_putText("Defend the");
            lcd_cursorSet(1, 0);
            lcd_putText("planet");
            waitButton();
            break;
        }
    }
    waitButton();
}
void playSound()
{
    TA1CTL = (TASSEL_2 | MC_1);
    TA1CCTL1 = OUTMOD_3;
    int count;
    for (count = 0; count < 125; count++)
    {
        TA1CCR0 = 1000;
        TA1CCR1 = 500;
        __delay_cycles(1000);
    }
    TA1CTL |= MC_0;
    TA1CCR0 = 0;
}

void waitButton()
{
    while (1)
    {
        adac_read(adc);
        if (adc[3] == 255)
            break;
    }
    while (1)
    {
        adac_read(adc);
        if (adc[3] == 0)
            break;
    }
}
void missile_update(unsigned char adc3, char (*missiles)[MIS_SIZE],
                    int (*missile_pos_x)[MIS_SIZE],
                    int (*missile_pos_y)[MIS_SIZE], int *pos_x, int *pos_y)
{
    unsigned int i;
    for (i = 0; i < MIS_SIZE; i++) // Update missile positions
    {
        if ((*missiles)[i])
        {
            (*missile_pos_x)[i] += 1;
            if ((*missile_pos_x)[i] == 16)
                (*missiles)[i] = 0;
        }
    }
    if (missile_cnt >= MISSILE_LIMITER) // Limit the speed of reloading the missiles.
    {
        if ((adc3 == 255) && (direc == 1)) // If facing right and button pressed, fire missile
        {
            unsigned int i;
            for (i = 0; i < MIS_SIZE; i++)
            {
                if (!(*missiles)[i]) // Find the first non-launched missile and launch it
                {
                    (*missiles)[i] = 1;
                    (*missile_pos_x)[i] = *pos_x + 1;
                    (*missile_pos_y)[i] = *pos_y;
                    missile_cnt = 0;
                    break;
                }
            }
        }
    }
}
void meteor_update(char (*meteors)[METEOR_SIZE],
                   int (*meteor_pos_x)[METEOR_SIZE],
                   int (*meteor_pos_y)[METEOR_SIZE], char (*upper_health)[8],
                   char (*lower_health)[8])
{
    unsigned int i;
    char create_meteor = 1;
    if ((meteor_timer_cnt >= METEOR_TIMER_LIMIT - diff)) // Check if it is time to update meteors. Basically determines the speed of meteors
    {
        for (i = 0; i < METEOR_SIZE; i++)
        {
            if ((*meteors)[i] == 1) // If the meteor exists, shift one to the left
            {
                (*meteor_pos_x)[i] -= 1;
                if ((*meteor_pos_x)[i] == 0) // If meteor hits the most left, reset the meteor
                {
                    (*meteors)[i] = 0;
                    health -= 1;
                    if (level == 2)
                    {
                        if (health > 7)
                        {
                            (*upper_health)[TOTAL_HP - health - 1] = 0;
                            lcd_createCustom((*upper_health), 3);
                        }
                        else
                        {
                            (*lower_health)[TOTAL_HP - health - 9] = 0;
                            lcd_createCustom((*lower_health), 4);
                        }
                        if (!health)
                            still_game = 0;
                    }
                }
            }
            else if (!(*meteors)[i] && (create_meteor))
            {
                create_meteor = 0;
                if (getRNG()) // 50% possibility of getting a new meteor
                {
                    (*meteors)[i] = 1;
                    (*meteor_pos_x)[i] = 15;
                    (*meteor_pos_y)[i] = rand_pos_y; // Randomly select the horizontal position of the meteor
                    rand_pos_y = 1;
                }
                else
                    rand_pos_y = 0;
            }
        }
        meteor_timer_cnt = 0; // Reset the meteor timer counter
    }
}
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMERA0(void)
{
    missile_cnt += 1; // Increase missile counter
    meteor_timer_cnt += 1; // Increase meteor counter
    update = 1; // Update req.
}
