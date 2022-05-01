#include <p18f4620.h>
#include "Main.h"
#include "Fan_Support.h"
#include "stdio.h"
#define OFF 0x00 // Define colors for RGB D1  
#define RED 0x01                                                                                                    
#define GRN 0x02
#define YLW 0x03
#define BLU 0x04
#define PRP 0x05
#define CYA 0x06
#define WHT 0x07



char D1[8]={ OFF, RED, GRN, YLW, BLU, PRP, CYA, WHT} ;         // Array for RGB D1
char D2[7]={ RED, YLW, GRN, BLU, PRP, CYA, WHT};                                      // Array for RGB D2

extern char FAN;
extern char duty_cycle;


int get_RPM()
{
int RPS = TMR3L / 2;                                                           // read the count. Since there are 2 pulses per rev
                                                                                // then RPS = count /2
 TMR3L = 0;                                                                     // clear out the count
 return (RPS * 60);                                                             // return RPM = 60 * RPS
}

void Toggle_Fan()               // Function used to turn fan on and off using the remote control
{
 if (FAN==0)                                                                 
    {
        Turn_On_Fan();
    }
    else
    {
        Turn_Off_Fan();
    }
}

void Turn_Off_Fan()             // Function used to turn fan off
{
    FAN=0;
    FAN_EN=0;
}

void Turn_On_Fan()              // Function used to turn fan on
{
    FAN=1;
    do_update_pwm(duty_cycle);
    FAN_EN=1;
}

void Increase_Speed()           // Function used to Increase speed of fan using remote control
{
    if(duty_cycle == 100)                                                       // if duty cycle is 100 
    {
        Do_Beep();                                                              // Do a double deep to identify that the maximum speed has been met 
        printf("Duty Cycle is at 100 \r\n");                                    // Print on TeraTerm that this speed has been met
        do_update_pwm(duty_cycle);                                              // reprogram the PMW duty cycle
    }    
    else                                                                        // if duty cycle is not 100
    {
        duty_cycle = duty_cycle + 5;                                            // increment duty cycle for 5 
        do_update_pwm(duty_cycle);                                              // change the PMW duty cycle
    }
}

void Decrease_Speed()          // Function used to Decrease speed of fan using remote control
{
    if (duty_cycle == 0)                                                        // if duty cycle is 100
    {
        Do_Beep();                                                              // Do a double deep to identify that the minimum speed has been met
        printf("Duty Cycle is at 0 \r\n");                                      // Print of TeraTerm that this speed as been met
        do_update_pwm(duty_cycle);                                              // reprogram the PMW duty cycle
    }
    else                                                                        // if duty cycle is not 0
    {
        duty_cycle = duty_cycle - 5;                                            // decrement duty cycle by 5
        do_update_pwm(duty_cycle);                                              //// change the PMW duty cycle
    } 
}

void do_update_pwm(char duty_cycle)         // Function that uses PMW to vary the speed using the duty cycle
{ 
 float dc_f;
 int dc_I;
 PR2 = 0b00000100 ;                                                             // set the frequency for 25 Khz
 T2CON = 0b00000111 ;                                                           // Set T3CON to 0x03 to turn timer on
 dc_f = ( 4.0 * duty_cycle / 20.0) ;                                            // calculate factor of duty cycle versus a 25 Khz
                                                                                // signal
 dc_I = (int) dc_f;                                                             // get the integer part
 if (dc_I > duty_cycle) dc_I++;                                                 // round up function
 CCP1CON = ((dc_I & 0x03) << 4) | 0b00001100;
 CCPR1L = (dc_I) >> 2;
}

void Set_DC_RGB(int duty_cycle)
{
    unsigned char d1color_I;
    unsigned char d1color_C = 0x00;
    d1color_I = duty_cycle / 10;
    if (d1color_I >7) d1color_I =7;

    d1color_C = (char) d1color_I;
    PORTA = (d1color_C << 1);
}

void Set_RPM_RGB(int rpm)
{
    unsigned char mask = 0xC7; 
    unsigned char maskedValue; 
    maskedValue = PORTD & mask;   
    int RPM= (rpm/500);
    if (rpm < 1){
        PORTD = maskedValue;
    }   
    else{
        PORTD = maskedValue | (D2[RPM] << 3);
    }
        
}