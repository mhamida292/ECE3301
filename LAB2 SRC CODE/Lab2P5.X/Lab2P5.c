
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <math.h>
#include <p18f4620.h>
#include <usart.h>
#pragma config OSC = INTIO67
#pragma config BOREN = OFF
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config CCP2MX = PORTBE

void Delay_One_Sec()
{
    for(int I=0; I <17000; I++); //for loop to emulate counting to 17000 to simulate 1 second delay
}

void main()
{
char in;
char portCArray[8] = {0xf,0x1b,0x2a,0x39,0x25,0x4,0x10,0x36};
TRISD = 0x00; //init output to portD using 0x00
TRISC = 0x00;//init output to portC using 0x00
ADCON1 = 0x0f;//switching analog to digital


    while (1) //infinite loop
    {
        for (int i = 0; i <= 7; i++) //counter counting to 8 for 8 bits
        {
           PORTD = i; // setting first output to i to iterate through original cases
           PORTC = portCArray[i]; // setting second and third LED to iterate through array of hex values

           Delay_One_Sec(); // delay one second

        }

    }
 }
