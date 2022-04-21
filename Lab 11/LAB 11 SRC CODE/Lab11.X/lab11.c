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
#include "ST7735.h"

#define Circle_Size     20              // Size of Circle for Light
#define Circle_X        60              // Location of Circle
#define Circle_Y        80              // Location of Circle
#define Text_X          52
#define Text_Y          77
#define TS_1            1               // Size of Normal Text
#define TS_2            2               // Size of Big Text


// colors
#define RD               ST7735_RED
#define BU               ST7735_BLUE
#define GR               ST7735_GREEN
#define MA               ST7735_MAGENTA
#define BK               ST7735_BLACK

#define kp               PORTEbits.RE1     

#define Circle_Size     20              // Size of Circle for Light
#define Circle_X        60              // Location of Circle
#define Circle_Y        80              // Location of Circle
#define Text_X          52
#define Text_Y          77
#define TS_1            1               // Size of Normal Text
#define TS_2            2               // Size of Big Text

#define RD               ST7735_RED
#define BU               ST7735_BLUE
#define GR               ST7735_GREEN
#define MA               ST7735_MAGENTA
#define BK               ST7735_BLACK

unsigned char Nec_state = 0;
unsigned char i,bit_count;
short nec_ok = 0;
unsigned long long Nec_code;
unsigned int Time_Elapsed;

extern char Nec_code1;

char tempSecond = 0xff; 
char second = 0x00;
char minute = 0x00;
char hour = 0x00;
char dow = 0x00;
char day = 0x00;
char month = 0x00;
char year = 0x00;
char setup_second, setup_minute, setup_hour, setup_day, setup_month, setup_year;
char alarm_second, alarm_minute, alarm_hour, alarm_date;
char setup_alarm_second, setup_alarm_minute, setup_alarm_hour;

char array1[21]={0xa2};
char txt1[21][4] ={"CH-\0"};
int color[21]={RD};
#define D1R     0x02
#define D1G     0x04
#define D1B     0x08  
#define D1M     0x0A

#define D2M     0x28
#define D2W     0x38

#define D3W     0x07
char D1[21] = {D1R, D1R, D1R, D1B, D1B, D1G, D1M, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char D2[21] = {0, 0, 0, 0, 0, 0,0, D2M, D2M, D2W, D2W, D2W, D2W, D2W, 0, 0, 0, 0, 0, 0, 0};
char D3[21] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, D3W, D3W, D3W, D3W, D3W, D3W, D3W};

char TempSecond;
#define KEY_PRESSED     PORTBbits.RB7
char buffer[31];                        // general buffer for display purpose
char *nbr;                              // general pointer used for buffer
char *txt;

char Nec_code1;
char nec_code; 
char found;

void putch(char c); 
void init_UART(); 
void Do_Init();
void Deactivate_Buzzer();
void Activate_Buzzer();
void Initialize_Screen();
void Wait_One_Sec();
void Wait_Half_Second(); 
void main() 
{ 
    Do_Init();                                                  // Initialization  
    //Initialize_Screen(); 
    DS3231_Setup_Time(); 
    while (1)							// This is for the DS1621 testing. Comment this section
    {								// out to move on to the next section
        char tempC = DS1621_Read_Temp();
        char tempF = (tempC * 9 / 5) + 32;
        printf(" Temperature = %d degreesC = %d degreesF\r\n", tempC, tempF);
        char previousSecond = second; 
        DS3231_Read_Time();
        if(second != previousSecond){
            printf("%02x:%02x:%02x %02x/%02x/%02x",hour,minute,second,month,day,year);   
            tempC = DS1621_Read_Temp();
            tempF = (tempC * 9 / 5) + 32;
            printf (" Temperature = %d degreesC = %d degreesF\r\n", tempC, tempF);
        }
        if(nec_ok == 1)
        {
            nec_ok = 0;
            Nec_code1 = (char)((Nec_code >> 8));
            INTCONbits.INT0IE = 1;          // Enable external interrupt
            INTCON2bits.INTEDG0 = 0;        // Edge programming for INT0 falling edge
            
            found = 0xff;
            
            
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
    
//      while (1)						// This is for the DS3231 testing. Comment this section
//      {							// out to move on to the next section
//
//        DS3231_Read_Time();
//        if(tempSecond != second)
//        {
//            tempSecond = second;
//            char tempC = DS1621_Read_Temp();
//            char tempF = (tempC * 9 / 5) + 32;
//            printf ("%02x:%02x:%02x %02x/%02x/%02x",hour,minute,second,month,day,year);
//            printf (" Temperature = %d degreesC = %d degreesF\r\n", tempC, tempF);
//        }
    
//        if (nec_ok == 1)					// This is for the final section
//        {
    
    
    
//        }
//      }
}

void Do_Init()                      // Initialize the ports 
{ 
    init_UART();                    // Initialize the uart
    //init_INTERRUPT();
    OSCCON=0x70;                    // Set oscillator to 8 MHz 
    DS1621_Init();                  //Initialize the DS1621 begin process
    
    ADCON1= 0x0F;		    // Fill out values
    TRISA = 0x00;
    TRISB = 0x11;
    TRISC = 0x00;                   
    TRISD = 0x40;
    TRISE = 0x00;
    RBPU=0;
    I2C_Init(100000); 
    //init_INTERRUPT(); 
}
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


void Wait_One_Sec()
{
    Wait_Half_Second();
    Wait_Half_Second();
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

void Initialize_Screen()
{
    LCD_Reset();
    TFT_GreenTab_Initialize();
    fillScreen(ST7735_BLACK);
  
    /* TOP HEADER FIELD */
    txt = buffer;
    strcpy(txt, "ECE3301L Spring 22-S3");  
    drawtext(2, 2, txt, ST7735_WHITE, ST7735_BLACK, TS_1);

    strcpy(txt, "LAB 11 ");  
    drawtext(50, 10, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
}