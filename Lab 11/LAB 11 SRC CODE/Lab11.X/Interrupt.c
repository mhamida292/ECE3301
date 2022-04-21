#include <xc.h>
#include <p18f4620.h>
#include "Interrupt.h"


unsigned char Nec_state = 0;
unsigned char i,bit_count;
short nec_ok = 0;
unsigned long long Nec_code;
unsigned int Time_Elapsed;

extern char Nec_code1;

void interrupt high_priority chkisr()
{
    if (PIR1bits.TMR1IF == 1) TIMER1_isr();
    if (INTCONbits.INT0IF == 1) INT0_isr();
}

void TIMER1_isr(void)
{
    Nec_state = 0;                          // Reset decoding process
    INTCON2bits.INTEDG0 = 0;                // Edge programming for INT0 falling edge
    T1CONbits.TMR1ON = 0;                   // Disable T1 Timer
    PIR1bits.TMR1IF = 0;                    // Clear interrupt flag
}

void force_nec_state0()
{
    Nec_state=0;
    T1CONbits.TMR1ON = 0;
}

void INT0_ISR() 
{    
    INTCONbits.INT0IF = 0;                  // Clear external interrupt
    if (Nec_state != 0)
    {
        Time_Elapsed = (TMR1H << 8) | TMR1L;       // Store Timer1 value
        TMR1H = 0;                          // Reset Timer1
        TMR1L = 0;
    }
    
    switch(Nec_state)
    {
        case 0 :
        {
                                            // Clear Timer 1
            TMR1H = 0;                      // Reset Timer1
            TMR1L = 0;                      //
            PIR1bits.TMR1IF = 0;            //
            T1CON= 0x90;                    // Program Timer1 mode with count = 1usec using System clock running at 8Mhz
            T1CONbits.TMR1ON = 1;           // Enable Timer 1
            bit_count = 0;                  // Force bit count (bit_count) to 0
            Nec_code = 0;                   // Set Nec_code = 0
            Nec_state = 1;                  // Set Nec_State to state 1
            INTCON2bits.INTEDG0 = 1;        // Change Edge interrupt of INT0 to Low to High            
            return;
        }
        
        case 1 :
        {
            if(Time_Elapsed > 8500 && Time_Elapsed < 9500)
            {
                Nec_state = 2;              // Set Nec_State to state 1
            }
            else
            {
                force_nec_state0();         // Set Nec_State to state 0
            }
            INTCON2bits.INTEDG0 = 0;        // Change Edge interrupt of INT0 to high to low 
            return;
        }
        
        case 2 :                            // Add your code here
        {
            if(Time_Elapsed > 4000 && Time_Elapsed < 5000)
            {
                Nec_state = 3;              // Set Nec_State to state 3
            }
            else
            {
                force_nec_state0();         // Set Nec_State to state 0
            }
            INTCON2bits.INTEDG0 = 1;        // Change Edge interrupt of INT0 to low to high 
            return;
        }
        
        case 3 :                            // Add your code here
        {
            if(Time_Elapsed > 400 && Time_Elapsed < 700)
            {
                Nec_state = 4;              // Set Nec_State to state 4
            }
            else
            {
                force_nec_state0();         // Set Nec_State to state 0
            }
            INTCON2bits.INTEDG0 = 0;        // Change Edge interrupt of INT0 to high to low 
            return;
        }
        
        case 4 :                            // Add your code here
        {
            if(Time_Elapsed > 400 && Time_Elapsed < 1800)
            {
                Nec_code = Nec_code << 1;   // Left shift nec_code by 1
                if(Time_Elapsed > 1000)
                {
                    Nec_code++;             // Increment Nec_State by 1
                }
                bit_count++;                // Increment bit_count by 1
                if(bit_count > 31)
                {
                    nec_ok = 1;             // Set Nec_ok to 1
                    Nec_code1 = (char)((Nec_code >> 8));
                    INT0IE = 0;             
                    Nec_state = 0;          // Set Nec_State to state 0
                }
                Nec_state = 3;              // Set Nec_State to state 3
            }
            else
            {
                force_nec_state0();         // Set Nec_State to state 0
            }
            INTCON2bits.INTEDG0 = 1;        // Change Edge interrupt of INT0 to low to high 
            return;
        }
    }     
} 



