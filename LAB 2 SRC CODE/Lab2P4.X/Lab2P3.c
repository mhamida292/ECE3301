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
    for(int I=0; I <17000; I++);
}

void main() 
{  
    char in;            // Use variable ?in? as char 
    TRISA = 0xff;       // fill out the ?? with the proper values 
    TRISD = 0x00; 
    ADCON1 = 0x0f;      // fill out the ?? with the proper values 
    
    while (1) 
    { 
        for(char i = 0; i < 8; i++){
            in  = i;    // read data from PORTA and save it 
                        // into ?in?    
            in = in & 0x0F; // Mask out the unused upper four bits 
            PORTD = in;     // Output the data to PORTB 
            Delay_One_Sec();
        }
    } 
 } 