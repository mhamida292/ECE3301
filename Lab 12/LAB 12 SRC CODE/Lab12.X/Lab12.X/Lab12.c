#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <math.h>
#include <p18f4620.h>
#include <usart.h>
#include <string.h>

#include "I2C_Soft.h"
#include "I2C_Support.h"
#include "Interrupt.h"
#include "Fan_Support.h"
#include "Main.h"
#include "ST7735_TFT.h"

#pragma config OSC = INTIO67
#pragma config BOREN =OFF
#pragma config WDT=OFF
#pragma config LVP=OFF
#pragma config CCP2MX = PORTBE

#define D3R    0x01
#define D3G    0x02
#define D3B    0x04
#define D3M    0x05
#define D3W    0x07

#define KEY_LED PORTBbits.RB7                                                   // Set KEY PRESSES LED to PORTB7
#define OFF 1
#define ON 0
void Initialize_Screen();
char second = 0x00;
char minute = 0x00;
char hour = 0x00;
char dow = 0x00;
char day = 0x00;
char month = 0x00;
char year = 0x00;

char found;
char tempSecond = 0xff; 
signed int DS1621_tempC, DS1621_tempF;
char setup_second, setup_minute, setup_hour, setup_day, setup_month, setup_year;
char alarm_second, alarm_minute, alarm_hour, alarm_date;
char setup_alarm_second, setup_alarm_minute, setup_alarm_hour;
unsigned char setup_fan_temp = 75;
unsigned char Nec_state = 0;

short nec_ok = 0;
char Nec_code1;
char FAN;
char duty_cycle;
int rps;
int rpm;
int ALARMEN;

char buffer[33]     = " ECE3301L Sp'22 L12\0";
char *nbr;
char *txt;
char tempC[]        = "+25";
char tempF[]        = "+77";
char time[]         = "00:00:00";
char date[]         = "00/00/00";
char alarm_time[]   = "00:00:00";
char Alarm_SW_Txt[] = "OFF";
char Fan_SW_Txt[]   = "OFF";                // text storage for Heater Mode

char array1[21] = { 0xa2,0x62,0xe2,0x22,0x02,0xc2,0xe0,0xa8,0x90,0x68,0x98,0xb0,   // array given from TeraTerm, by pressing all 21 buttons
0x30,0x18,0x7a,0x10,0x38,0x5a,0x42,0x4a,0x52 };
int color[21] = {RD,RD,RD,BU,BU,GR,MA,MA,MA,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK }; // Color of each of the 21 buttons
char D3[21]={D3R,D3R,D3R,D3B,D3B,D3G,D3M,D3M,D3M,D3W,D3W,D3W,D3W,D3W,D3W,D3W,D3W,D3W,D3W,D3W,D3W};

    
char DC_Txt[]       = "000";                // text storage for Duty Cycle value
char RTC_ALARM_Txt[]= "0";                      //
char RPM_Txt[]      = "0000";               // text storage for RPM

char setup_time[]       = "00:00:00";
char setup_date[]       = "01/01/00";
char setup_alarm_time[] = "00:00:00"; 
char setup_fan_text[]   = "075F";


void putch (char c)
{   
    while (!TRMT);       
    TXREG = c;
}

void init_UART()
{
    OpenUSART (USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, 25);
    OSCCON = 0x70;
}

void Do_Init()                      // Initialize the ports 
{ 
    init_UART();                    // Initialize the uart
    OSCCON=0x70;                    // Set oscillator to 8 MHz 
    
    ADCON1=0x0F;
    TRISA = 0x00;
    TRISB = 0x11; //0001 0001
    TRISC = 0x01;
    TRISD = 0x40; //0100 0000
    TRISE = 0x00;


    RBPU=0;
    TMR3L = 0x00;                   
    T3CON = 0x03;
    I2C_Init(100000); 

    DS1621_Init();
    init_INTERRUPT();
    FAN = 0;
}

void main() 
{
    Do_Init();                                                                  // Initialization      
    T3CONbits.TMR3ON=1;                                                         // Turn timer on
    Initialize_Screen();                                                        // Initialize the LED screen
    duty_cycle=50;                                                              // Set duty cycle to an initial value of 50
    do_update_pwm(duty_cycle);                                                  // reprogram the PMW duty cycle
    
    while (1)                                                                   // while loop
    {

        DS3231_Read_Time();                                                     // Function to read the time every second, found in I2C_Support.C file
        if(tempSecond != second)
        {
            tempSecond = second;
            DS1621_tempC = DS1621_Read_Temp();                                  // Read temperature in Celsius 
            DS1621_tempF = (DS1621_tempC * 9 / 5) + 32;                         // Convert temperature to Fahrenheit
            rpm = get_RPM();                                                    // get the value of rpm
            printf ("%02x:%02x:%02x %02x/%02x/%02x",hour,minute,second,month,   // print time onto both TeraTerma and LCD screen
            day,year);
            printf (" Temp = %d C = %d F ", DS1621_tempC, DS1621_tempF);        // print temperature in both C and F for TeraTerm and LCD screen
            printf ("RPM = %d duty_cycle = %d\r\n", rpm, duty_cycle);           // print the duty cycle and rpm on TeraTerm and LCD screen
            Set_DC_RGB(duty_cycle);                                             // Set the color of RBG D1 correctly based on the duty cycle
            Set_RPM_RGB(rpm);                                                   // Set the color of RGB D2 correctly based on the rpm
            Update_Screen();                                                    // Update LCD screen to show everything on TeraTerm to the LCD screen as well
        }
                                              // Update LCD screen to show everything on TeraTerm to the LCD screen as well

       if (nec_ok == 1)
        {
                

            nec_ok = 0;
            printf ("NEC_Code = %x\r\n", Nec_code1);                            // make sure to use Nec_code1

            INTCONbits.INT0IE = 1;                                              // Enable external interrupt
            INTCON2bits.INTEDG0 = 0;                                            // Edge programming for INT0 falling edge

            found = 0xff;
            for (int j=0; j< 21; j++)                                           // For all values of 0-20, check to see if the button pressed matches any value in the array, and set that position value into found
            {
                if (Nec_code1 == array1[j])         
                {
                    found = j;
                    j = 21;
                }
            }
            KEY_LED = ON ;                                                      // Turn on the KEY PRESES LED
            Activate_Buzzer();                                                  // Deactivate the buzzer
            PORTE = D3[found];
            Wait_Half_Second();                                                 // Call Wait_Half_Seconds functions
            KEY_LED = OFF ;                                                      // Turn off the KEY PRESES LED
            Deactivate_Buzzer();                                                // Deactivate the buzzer
            if (found == 0xff) 
            {
                printf ("Cannot find button \r\n", found);                      // TeraTerm will display "Cannot find button" when a button pressed has no command
            }
            else
            {
                Do_Beep();                                                      // 
                printf ("button = %d \r\n", found);
                if (found == 5)                                                 // when "Play/Pause" button is pressed, the fan will turn off and on
                {
                    Toggle_Fan();
                }
                else if (found == 6)                                            // when "-" button is pressed the fan speed will decrement by 5 dc
                {
                    Decrease_Speed();
                }
                else if (found == 7)                                            // when "+" button is pressed the fan speed will increment by 5 dc
                {
                    Increase_Speed();
                }
                else if (found == 8)                                            // when "EQ" button is pressed the set up time will reset back to its initial time set by user 
                {                                                               // in our case 04:20:15
                    DS3231_Setup_Time();
                }
                
            }            
        }
    }
}