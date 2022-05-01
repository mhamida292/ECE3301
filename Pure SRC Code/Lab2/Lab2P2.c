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

void main() 
{  
    char in;            // Use variable ?in? as char 
    TRISA = 0xff;       // fill out the TRISA with the FF values to use as input 
    TRISD = 0x00;       // fill out the TRISD with the 00 values to use as output
    ADCON1 = 0x0f;      // fill out the 0f to make it from analog to digital 
    
    while (1) 
    { 
        in  = PORTA;    // read data from PORTA and save it 
                        // into ?in?    
        in = in & 0x0F; // Mask out the unused upper four bits 
        PORTD = in;     // Output the data to PORTD
    } 
 } 