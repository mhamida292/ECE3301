#include <p18f4620.h>
#include "Main.h"
#include "Utils.h"
#include "stdio.h"
//#define kp               PORTBbits.RB7                                          // Define Key Pressed LED to PORTE1

#define OFF 1                                                                   // Define 0 as OFF
#define ON  0                                                                   // Define 1 as ON

void Do_Beep()
{
    Activate_Buzzer();  
    Wait_Half_Second();
    Activate_Buzzer();
    Wait_Half_Second();
    Deactivate_Buzzer();
}

void Wait_One_Sec()
{
   Wait_Half_Second();
   Wait_Half_Second();
}

void Wait_Half_Second()
{
    T0CON = 0x02;                                                               // Timer 0, 16-bit mode, prescaler 1:8
    TMR0L = 0xDB;                                                               // set the lower byte of TMR
    TMR0H = 0x0B;                                                               // set the upper byte of TMR
    INTCONbits.TMR0IF = 0;                                                      // clear the Timer 0 flag
    T0CONbits.TMR0ON = 1;                                                       // Turn on the Timer 0
    while (INTCONbits.TMR0IF == 0);                                             // wait for the Timer Flag to be 1 for done
        T0CONbits.TMR0ON = 0;                                                   // turn off the Timer 0
}

void Activate_Buzzer()
{
    PR2 = 0b11111001 ;
    T2CON = 0b00000101 ;
    CCPR2L = 0b01001010 ;
    CCP2CON = 0b00111100 ;
}

void Deactivate_Buzzer()
{
    CCP2CON = 0x0;
    PORTBbits.RB3 = 0;                                                          // Set PORTB3 to buzzer
}