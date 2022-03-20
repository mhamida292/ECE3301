#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <math.h>
#include <usart.h>
#include <p18f4620.h>

#pragma config OSC = INTIO67
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config BOREN = OFF
#pragma config CCP2MX = PORTBE 

#define SEG_G PORTEbits.RE1

#define SEC_LED PORTDbits.RD7 
#define MODE_LED PORTBbits.RB7 
//D1
#define NS_RED PORTAbits.RA5
#define NS_GREEN PORTBbits.RB0
//D2
#define EW_RED PORTBbits.RB4
#define EW_GREEN PORTBbits.RB5
//D3
#define NSLT_RED PORTBbits.RB1
#define NSLT_GREEN PORTBbits.RB2
//D4
#define EWLT_RED PORTEbits.RE0
#define EWLT_GREEN PORTEbits.RE2

#define NSPED_SW PORTAbits.RA1
#define NSLT_SW PORTAbits.RA2
#define EWPED_SW PORTAbits.RA3
#define EWLT_SW PORTAbits.RA4

#define OFF     0
#define RED     1 
#define GREEN   2
#define YELLOW  3

void Init_ADC();    
void init_UART();
void Init_Tris(); 
unsigned int get_full_ADC(void); //
void Display_Lower_Digit(char digit);   //
void Display_Upper_Digit(char digit);   //
void Select_ADC_Channel(char channel);  //
void Wait_One_Second(); 
void Wait_Half_Second();
void Wait_N_Seconds(char seconds);
void Set_NS(char color);
void Set_NSLT(char color);
void Set_EW(char color); 
void Set_EWLT(char color); 
void Activate_Buzzer();
void Deactivate_Buzzer(); 
void Wait_One_Second_With_Beep(); 
void PED_Control( char Direction, char Num_Sec); 
void Day_Mode();
void Night_Mode(); 

char array[10] = {0xFF, 0x79, 0x24,0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10}; 

void main(void){
    Init_ADC();
    init_UART();
    Init_Tris();
    
    
    while (1){ 
        Select_ADC_Channel(0);
        int num_step = get_full_ADC();
        float pvoltage_mv = num_step*4.0;
        float Pvoltage = pvoltage_mv; 
        int finalVoltage = (int)Pvoltage; 
        
        if(finalVoltage < 3000){
            MODE_LED = 1; 
            Day_Mode();
        }else{
            MODE_LED = 0; 
            Night_Mode(); 
            
        }
        
       // Display_D3(intVoltage); 
//        for(int i = 0; i < 4; i++)
//        { 
//            Set_NS(i); // Set color for North-South direction 
//            Set_NSLT(i); // Set color for North-South Left-Turn direction 
//            Set_EW(i); // Set color for East-West direction 
//            Set_EWLT(i); // Set color for East-West Left-Turn direction 
//            Wait_N_Seconds(1); // call Wait-N-Second routine to wait for 1 second 
//
//            //PED_Control(0, 8); // Set direction 0 and do for 8 seconds 
//            //PED_Control(1, 8); // Set direction 1 for 6 seconds
//          }
//    if(MODE){
//            MODE_LED = 1; 
//            Day_Mode(); 
//        }else{
//            MODE_LED = 0; 
//            Night_Mode();
//        }
//    }
        
        
           
    }
}

void init_UART(void){
    OpenUSART(USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT 
            & USART_CONT_RX & USART_BRGH_HIGH, 25);
    OSCCON = 0x60;
}

void Init_Tris(){
    TRISA = 0x1F; 
    TRISB = 0x00; 
    TRISC = 0x00; 
    TRISD = 0x00; 
    TRISE = 0x00; 
}

void Init_ADC(void){    
    ADCON1=0x0E;
    ADCON2=0xA9;
}

void Select_ADC_Channel(char channel)
{
    ADCON0 = channel * 4 + 1;
}
unsigned int get_full_ADC(void){
    int result; 
    ADCON0bits.GO=1;
    // Start Conversion
    while(ADCON0bits.DONE==1);
    // wait for conversion to be completed
    result = (ADRESH * 0x100) + ADRESL;
    // combine result of upper byte and
    // lower byte into result
    return result;
    // return the result.
}
void Display_Upper_Digit(char digit){
    PORTC = array[digit] & 0x3F;
    if((array[digit] & 0x40) == 0x00)
        SEG_G = 0;
    else
        SEG_G = 1;
}
void Display_Lower_Digit(char digit){
    PORTD = (PORTD & 0x80) | array[digit]; 
}
void Wait_One_Second() { 
    SEC_LED = 0; // First, turn off the SEC LED 
    Wait_Half_Second(); // Wait for half second (or 500 msec) 
    SEC_LED = 1; // then turn on the SEC LED 
    Wait_Half_Second(); // Wait for half second (or 500 msec) 
} 
void Wait_Half_Second() { 
    T0CON = 0x02; // Timer 0, 16-bit mode, prescaler 1:8 
    TMR0L = 0xDB; // set the lower byte of TMR 
    TMR0H = 0x0B; // set the upper byte of TMR 
    INTCONbits.TMR0IF = 0; // clear the Timer 0 flag 
    T0CONbits.TMR0ON = 1; // Turn on the Timer 0 
    while (INTCONbits.TMR0IF == 0); // wait for the Timer Flag to be 1 for done 
    T0CONbits.TMR0ON = 0; // turn off the Timer 0 
}

void Wait_N_Seconds (char seconds) { 
    char I; 
    for (I = 0; I< seconds; I++) { 
        Wait_One_Second(); 
    } 
}

void Set_NS(char color){
    switch(color){
        case OFF: NS_RED=0; NS_GREEN=0; break; 
        case RED: NS_RED=1; NS_GREEN=0; break; 
        case GREEN: NS_RED=0; NS_GREEN=1; break; 
        case YELLOW: NS_RED=1; NS_GREEN=1; break; 
    }
}
void Set_NSLT(char color){
    switch(color){
        case OFF: NSLT_RED=0; NSLT_GREEN=0; break; 
        case RED: NSLT_RED=1; NSLT_GREEN=0; break; 
        case GREEN: NSLT_RED=0; NSLT_GREEN=1; break; 
        case YELLOW: NSLT_RED=1; NSLT_GREEN=1; break; 
    }
}
void Set_EW(char color){
    switch(color){
        case OFF: EW_RED=0; EW_GREEN=0; break; 
        case RED: EW_RED=1; EW_GREEN=0; break; 
        case GREEN: EW_RED=0; EW_GREEN=1; break; 
        case YELLOW: EW_RED=1; EW_GREEN=1; break; 
    }
}
void Set_EWLT(char color){
    switch(color){
        case OFF: EWLT_RED=0; EWLT_GREEN=0; break; 
        case RED: EWLT_RED=1; EWLT_GREEN=0; break; 
        case GREEN: EWLT_RED=0; EWLT_GREEN=1; break; 
        case YELLOW: EWLT_RED=1; EWLT_GREEN=1; break; 
    }
}
void Activate_Buzzer() { 
    PR2 = 0b11111001 ; 
    T2CON = 0b00000101 ; 
    CCPR2L = 0b01001010 ; 
    CCP2CON = 0b00111100 ; 
} 
void Deactivate_Buzzer() { 
    CCP2CON = 0x0; 
    PORTBbits.RB3 = 0;     
} 
void Wait_One_Second_With_Beep() { 
    SEC_LED = 1; // First, turn on the SEC LED 
    Activate_Buzzer(); // Activate the buzzer
    Wait_Half_Second(); // Wait for half second (or 500 msec) 
    SEC_LED = 0; // then turn off the SEC LED 
    Deactivate_Buzzer(); // Deactivate the buzzer 
    Wait_Half_Second(); // Wait for half second (or 500 msec) 
} 
void PED_Control( char Direction, char Num_Sec){

    Display_Lower_Digit(0x00); 
    Display_Upper_Digit(0x00);
    
    for(char i = Num_Sec - 1; i >0; i--){
        switch(Direction){
            case 0:
                Display_Upper_Digit(i);
                break;
            case 1:
                Display_Lower_Digit(i);
                break; 
        }
        Wait_One_Second_With_Beep();
    }
    Display_Lower_Digit(0x00); 
    Display_Upper_Digit(0x00);
    
    Wait_One_Second_With_Beep();
} 

void Night_Mode(){
    Set_NSLT(RED); 
    Set_EW(RED); 
    Set_EWLT(RED); 
    Set_NS(GREEN); 
    Wait_N_Seconds(8); 
    Set_NS(YELLOW);
    Wait_N_Seconds(3); 
    Set_NS(RED);
    if(EWLT_SW){
        Set_EWLT(GREEN);
        Wait_N_Seconds(6); 
        Set_EWLT(YELLOW);
        Wait_N_Seconds(3); 
        Set_EWLT(RED);
    }else{
        Set_EW(GREEN);
        Wait_N_Seconds(6); 
        Set_EW(YELLOW); 
        Wait_N_Seconds(3);
        Set_EW(RED); 
    }
    if(NSLT_SW){
        Set_NSLT(GREEN);
        Wait_N_Seconds(8);
        Set_NSLT(YELLOW);
        Wait_N_Seconds(3); 
        Set_NSLT(RED);
    }else{
        Set_NSLT(RED);
    }
}
void Day_Mode(){
    Set_NSLT(RED); 
    Set_EW(RED); 
    Set_EWLT(RED); 
    Set_NS(GREEN); 
    
    if(NSPED_SW){
        PED_Control(0,8);
    }
    
    Wait_N_Seconds(7); 
    Set_NS(YELLOW); 
    Wait_N_Seconds(3);
    Set_NS(RED); 
    
    if(EWLT_SW){
        Set_EWLT(GREEN);
        Wait_N_Seconds(8);
        Set_EWLT(YELLOW); 
        Wait_N_Seconds(3); 
        Set_EWLT(RED);  
    }
    //step 9
    Set_EW(GREEN); 
    if(EWPED_SW){
        PED_Control(1,9); 
    }
    
    //step10
    Set_EW(GREEN);
    Wait_N_Seconds(9); 
    Set_EW(YELLOW);
    Wait_N_Seconds(3); 
    Set_EW(RED); 
    
    if(NSLT_SW){
        Set_NSLT(GREEN);
        Wait_N_Seconds(8); 
        Set_NSLT(YELLOW); 
        Wait_N_Seconds(3); 
    }
    Set_NSLT(RED); 
}

