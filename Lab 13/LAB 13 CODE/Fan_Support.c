
#include <p18f4620.h>
#include "Main.h"
#include "Fan_Support.h"
#include "stdio.h"

extern char HEATER;
extern char duty_cycle;
extern char heater_set_temp;
extern signed int DS1621_tempF;

char D1[8]={ OFF, RED, GRN, YLW, BLU, PRP, CYA, WHT};          // Array for RGB D1
char D2[7]={ RED, YLW, GRN, BLU, PRP, CYA, WHT};               // Array for RGB D2

int get_duty_cycle(signed int temp, int set_temp){
if(temp >= set_temp) duty_cycle = 0;     
    else if (temp < set_temp){
        int diff = set_temp - temp;
        if(diff > 50) duty_cycle = 100;         // check if dc is greater than 100. If so, set it to 100
        else if((diff >= 25) && (diff < 50)) duty_cycle = diff * 2;
        else if ((diff >= 10) && (diff < 25)) duty_cycle = diff * (3/2);
        else if ((diff >= 0) && (diff < 10)) duty_cycle = diff;
    }
    return duty_cycle;      // return dc
}

void Monitor_Heater()
{
    duty_cycle = get_duty_cycle(DS1621_tempF, heater_set_temp);
    do_update_pwm(duty_cycle);
    
    if (HEATER == 1) if(DS1621_tempF < heater_set_temp) Turn_On_Fan();
    else Turn_Off_Fan();
}

void Toggle_Heater()
{
    if(FAN_EN==0)  Turn_On_Fan(); 
    else Turn_Off_Fan();
}

int get_RPM()
{
    int RPS = TMR3L / 2;                                                           // read the count. Since there are 2 pulses per rev
                                                                                    // then RPS = count /2
    TMR3L = 0;                                                                     // clear out the count
    return (RPS * 60);                                                             // return RPM = 60 * RPS
}

void Turn_Off_Fan()             // Function used to turn fan off
{
    HEATER = 0; 
    FAN_LED = 0; 
    FAN_EN=0;
}

void Turn_On_Fan()              // Function used to turn fan on
{
    HEATER = 1; 
    FAN_LED = 1; 
    do_update_pwm(duty_cycle);
    FAN_EN=1;
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
void Increase_Speed()
{
    if (duty_cycle == 100)
    {
        Do_Beep();
        Do_Beep();
    }
    else 
    {
        duty_cycle = duty_cycle + 5;
    }
}

void Decrease_Speed()
{
    if (duty_cycle == 0)
    {
        Do_Beep();
        Do_Beep();
        do_update_pwm(duty_cycle);
    }
    else 
    {
        duty_cycle = duty_cycle - 5;
    }
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

