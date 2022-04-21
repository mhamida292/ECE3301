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
 
}

void TIMER1_isr(void)
{

}

void force_nec_state0()
{

}

void INT0_ISR() 
{    
                              
} 



