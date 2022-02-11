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
char in; // Use variable ?in? as char 
    TRISA = 0xFF; // This is to initialize A for input 
    TRISB = 0x00; // This is to initialize B for output
    ADCON1 = 0x0F; // This is to make it go from analog to digital
 
    while (1) 
    { 
        in = PORTA; // Saves input data into "in"                
        in = in & 0x0F; // Mask out the unused upper four bits 
        PORTB = in; // Output the data to PORTB 
    } 
 } 