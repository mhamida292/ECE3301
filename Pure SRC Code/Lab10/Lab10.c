#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <math.h>
#include <p18f4620.h>
#include <usart.h>
#include <string.h>

#pragma config OSC = INTIO67
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config BOREN = OFF
#pragma config CCP2MX = PORTBE

#include "ST7735.h"
#define _XTAL_FREQ  8000000             // Set operation for 8 Mhz


void TIMER1_isr(void);
void INT0_isr(void);
void Initialize_Screen();
void Wait_Half_Sec();
void Activate_Buzzer();
void Deactivate_Buzzer();

unsigned char Nec_state = 0;
unsigned char i,bit_count;
short nec_ok = 0;
unsigned long long Nec_code;
char Nec_code1;
unsigned int Time_Elapsed;

#define KEY_PRESSED     PORTBbits.RB7
#define D1R     0x02
#define D1G     0x04
#define D1B     0x08  
#define D1M     0x0A

#define D2M     0x28
#define D2W     0x38

#define D3W     0x07

//#define keyPressLED PORTBbits.RB7

// colors
#define RD               ST7735_RED
#define BU               ST7735_BLUE
#define GR               ST7735_GREEN
#define MA               ST7735_MAGENTA
#define BK               ST7735_BLACK

#define Circle_Size     20              // Size of Circle for Light
#define Circle_X        60              // Location of Circle
#define Circle_Y        80              // Location of Circle
#define Text_X          52
#define Text_Y          77
#define TS_1            1               // Size of Normal Text
#define TS_2            2               // Size of Big Text


char buffer[31];                        // general buffer for display purpose
char *nbr;                              // general pointer used for buffer
char *txt;

char array1[21]={0xa2, 0x62, 0xe2, 0x22, 0x02, 0xc2, 0xe0, 0xa8, 0x90, 0x68, 0x98, 0xb0, 0x30, 0x18, 0x7a, 0x10, 0x38, 0x5a, 0x42, 0x4a, 0x52};
char txt1[21][4] = {"CH-\0", "CH \0", "CH+\0", "PRV\0", "NXT\0", "P/P\0","VL-\0", "VL+\0", "EQ \0", "#00\0","100\0", "200\0", "#01\0", "#02\0", "#03\0", "#04\0", "#05\0", "#06\0", "#07\0", "#08\0", "#09\0"};
int color[21]={RD, RD, RD, BU, BU, GR, MA, MA, MA, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK};

char D1[21] = {D1R, D1R, D1R, D1B, D1B, D1G, D1M, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char D2[21] = {0, 0, 0, 0, 0, 0,0, D2M, D2M, D2W, D2W, D2W, D2W, D2W, 0, 0, 0, 0, 0, 0, 0};
char D3[21] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, D3W, D3W, D3W, D3W, D3W, D3W, D3W};

void putch (char c)
{
    while (!TRMT);
    TXREG = c;
}
void init_UART()
{
    OpenUSART (USART_TX_INT_OFF & USART_RX_INT_OFF &
    USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX &
    USART_BRGH_HIGH, 25);
    OSCCON = 0x60;
}

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

void INT0_isr(void)
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

void Wait_Half_Second()
{
    T0CON = 0x03;                               // Timer 0, 16-bit mode, prescaler 1:16
    TMR0L = 0xDB;                               // set the lower byte of TMR
    TMR0H = 0x0B;                               // set the upper byte of TMR
    INTCONbits.TMR0IF = 0;                      // clear the Timer 0 flag
    T0CONbits.TMR0ON = 1;                       // Turn on the Timer 0
    while (INTCONbits.TMR0IF == 0);             // wait for the Timer Flag to be 1 for done
    T0CONbits.TMR0ON = 0;                       // turn off the Timer 0
}

void Activate_Buzzer()
{
    PR2 = 0b11111001;
    T2CON = 0b00000101;
    CCPR2L = 0b01001010;
    CCP2CON = 0b00111100;
}

void Deactivate_Buzzer()
{
    CCP2CON = 0x0;
    PORTBbits.RB3 = 0;
}

void main()
{
    init_UART();
    OSCCON = 0x70;                          // 8 Mhz
    nRBPU = 0;                              // Enable PORTB internal pull up resistor
    TRISA = 0x00;
    TRISB = 0x01;
    TRISC = 0x00;                           // PORTC as output
    TRISD = 0x00;
    TRISE = 0x00;
    ADCON1 = 0x0F;                          //
    Initialize_Screen();
    INTCONbits.INT0IF = 0;                  // Clear external interrupt
    INTCON2bits.INTEDG0 = 0;                // Edge programming for INT0 falling edge H to L
    INTCONbits.INT0IE = 1;                  // Enable external interrupt
    TMR1H = 0;                              // Reset Timer1
    TMR1L = 0;                              //
    PIR1bits.TMR1IF = 0;                    // Clear timer 1 interrupt flag
    PIE1bits.TMR1IE = 1;                    // Enable Timer 1 interrupt
    INTCONbits.PEIE = 1;                    // Enable Peripheral interrupt
    INTCONbits.GIE = 1;                     // Enable global interrupts
    nec_ok = 0;                             // Clear flag
    Nec_code = 0x0;                         // Clear code
    
    while(1)
    {
        if (nec_ok == 1)
        {
            nec_ok = 0;

            Nec_code1 = (char) ((Nec_code >> 8));
            INTCONbits.INT0IE = 1;          // Enable external interrupt
            INTCON2bits.INTEDG0 = 0;        // Edge programming for INT0 falling edge
            
            char found = 0xff;
            
            
            // add code here to look for code
            for(int i = 0;i < 21;i++)
            {
                if(array1[i] == Nec_code1)
                {
                    found = i;
                    break;
                }
            }
            
            printf ("NEC_Code = %08lx %x ", Nec_code, Nec_code1);
            printf ("Found = %d\r\n", found);
            if (found != 0xff) 
            {
                PORTA = D1[found];
                PORTD = D2[found];
                PORTE = D3[found];
                
                fillCircle(Circle_X, Circle_Y, Circle_Size, color[found]); 
                drawCircle(Circle_X, Circle_Y, Circle_Size, ST7735_WHITE);  
                drawtext(Text_X, Text_Y, txt1[found], ST7735_WHITE, ST7735_BLACK,TS_1); 
                
                KEY_PRESSED = 1;
                //
                Activate_Buzzer();
                Wait_Half_Second();
                KEY_PRESSED = 0;
                //
                Deactivate_Buzzer();
            }
        }
    }
}


void Initialize_Screen()
{
    LCD_Reset();
    TFT_GreenTab_Initialize();
    fillScreen(ST7735_BLACK);
  
    /* TOP HEADER FIELD */
    txt = buffer;
    strcpy(txt, "ECE3301L Sp 22-S3");  
    drawtext(2, 2, txt, ST7735_WHITE, ST7735_BLACK, TS_1);

    strcpy(txt, "LAB 10 ");  
    drawtext(50, 10, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
}

