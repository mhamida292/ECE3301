#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <math.h>
#include <usart.h>
#include <p18f4620.h>


//System defaults for PIC18F
#pragma config OSC = INTIO67
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config BOREN = OFF
#pragma config CCP2MX = PORTBE 


//DEFINE all inputs and outputs to their corresponding leds and switches 
#define SEG_G PORTEbits.RE1

#define SEC_LED PORTDbits.RD7 
#define MODE_LED PORTBbits.RB7 

//Define LED Colors for the D1 LED
#define NS_RED PORTAbits.RA5
#define NS_GREEN PORTBbits.RB0

//Define LED Colors for the D2 LED
#define EW_RED PORTBbits.RB4
#define EW_GREEN PORTBbits.RB5

//Define LED Colors for the D3 LED
#define NSLT_RED PORTBbits.RB1
#define NSLT_GREEN PORTBbits.RB2
//Define LED Colors for the D4 LED
#define EWLT_RED PORTEbits.RE0
#define EWLT_GREEN PORTEbits.RE2

//Define all switches on dip switch
#define NSPED_SW PORTAbits.RA1
#define NSLT_SW PORTAbits.RA2
#define EWPED_SW PORTAbits.RA3
#define EWLT_SW PORTAbits.RA4


//Make life easier with simple 
#define OFF     0
#define RED     1 
#define GREEN   2
#define YELLOW  3


//Function PROTOTYPES
void Init_ADC();    // Initialize Analog to digital converter 
void init_UART();   // initialize UART connection 
void Init_Tris();   // Initialize TRIS connection for ports 
unsigned int get_full_ADC(void);        // Get the value from analog port
void Display_Lower_Digit(char digit);   // Display lower digit on 7seg LED
void Display_Upper_Digit(char digit);   // Display uppder digit on 7seg LED
void Select_ADC_Channel(char channel);  // Select correct ADC channel for conversion 
void Wait_One_Second();     // Wait one second 
void Wait_Half_Second();    // Wait half second 
void Wait_N_Seconds(char seconds);  // Wait N seconds by calling wait one second 
void Set_NS(char color);    // Set the north south color 
void Set_NSLT(char color);  // Set the north south left right signal color 
void Set_EW(char color);    // Set the north south left right signal color 
void Set_EWLT(char color);  // Set the east west left right signal color 
void Activate_Buzzer();     // Activate buzzer noise 
void Deactivate_Buzzer();   // Deactivate buzzer noise 
void Wait_One_Second_With_Beep();  //Wait one second, buzz
void PED_Control( char Direction, char Num_Sec);  //Control the Pedestrian controls 
void Day_Mode(); //Initiate day mode sequence 
void Night_Mode();  //Initiate night mode sequence 

char array[10] = {0xFF, 0x79, 0x24,0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10}; //Array of numbers to drive 7seg

void main(void){
    //Initiation methods!
    Init_ADC();
    init_UART();
    Init_Tris();
    
    while (1){  //Infinite loop
        Select_ADC_Channel(0); //choose correct ADC channel for the light resistor 
        int num_step = get_full_ADC(); // Get the correct numstep from get_full_adc
        float pvoltage_mv = num_step*4.0; // multiple numstep by 4
        float Pvoltage = pvoltage_mv;  
        int finalVoltage = (int)Pvoltage; // parse to int  
        
        if(finalVoltage < 3000){ // if output from light resistor is over 3000, run day mode
            MODE_LED = 1; 
            Day_Mode();
        }else{// if output from light resistor is less than 3000, run night mode
            MODE_LED = 0; 
            Night_Mode(); 
            
        }
        //TESTING VARIOUS PARTS
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

void init_UART(void){ //initiate UART communication 
    OpenUSART(USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT 
            & USART_CONT_RX & USART_BRGH_HIGH, 25);
    OSCCON = 0x60;
}

void Init_Tris(){ //set ports to input or output 
    TRISA = 0x1F; 
    TRISB = 0x00; 
    TRISC = 0x00; 
    TRISD = 0x00; 
    TRISE = 0x00; 
}

void Init_ADC(void){    //initialize analog to digital converter 
    ADCON1=0x0E;
    ADCON2=0xA9;
}

void Select_ADC_Channel(char channel) // Method for selecting correct channel for light resistor  
{
    ADCON0 = channel * 4 + 1;
}
unsigned int get_full_ADC(void){ // get result from light resistor 
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
void Display_Upper_Digit(char digit){ // display upper digit to 7seg led 
    PORTC = array[digit] & 0x3F; 
    if((array[digit] & 0x40) == 0x00)
        SEG_G = 0;
    else
        SEG_G = 1;
}
void Display_Lower_Digit(char digit){ //display lower digit from 7seg led 
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

void Wait_N_Seconds (char seconds) { //wait N amount of seconds by calling the wait one second an N amount of times 
    char I; 
    for (I = 0; I< seconds; I++) { 
        Wait_One_Second(); 
    } 
}

void Set_NS(char color){ //set NORTH SOUTH to various color combos 
    switch(color){
        case OFF: NS_RED=0; NS_GREEN=0; break; 
        case RED: NS_RED=1; NS_GREEN=0; break; 
        case GREEN: NS_RED=0; NS_GREEN=1; break; 
        case YELLOW: NS_RED=1; NS_GREEN=1; break; 
    }
}
void Set_NSLT(char color){ //set NORTH SOUTH left right signal to various color combos 
    switch(color){
        case OFF: NSLT_RED=0; NSLT_GREEN=0; break; 
        case RED: NSLT_RED=1; NSLT_GREEN=0; break; 
        case GREEN: NSLT_RED=0; NSLT_GREEN=1; break; 
        case YELLOW: NSLT_RED=1; NSLT_GREEN=1; break; 
    }
}
void Set_EW(char color){ //set EAST WEST to various color combos 
    switch(color){
        case OFF: EW_RED=0; EW_GREEN=0; break; 
        case RED: EW_RED=1; EW_GREEN=0; break; 
        case GREEN: EW_RED=0; EW_GREEN=1; break; 
        case YELLOW: EW_RED=1; EW_GREEN=1; break; 
    }
}
void Set_EWLT(char color){//set EAST WEST left right signal to various color combos 
    switch(color){
        case OFF: EWLT_RED=0; EWLT_GREEN=0; break; 
        case RED: EWLT_RED=1; EWLT_GREEN=0; break; 
        case GREEN: EWLT_RED=0; EWLT_GREEN=1; break; 
        case YELLOW: EWLT_RED=1; EWLT_GREEN=1; break; 
    }
}
void Activate_Buzzer() { //Activate buzzer
    PR2 = 0b11111001 ; 
    T2CON = 0b00000101 ; 
    CCPR2L = 0b01001010 ; 
    CCP2CON = 0b00111100 ; 
} 
void Deactivate_Buzzer() { //deactivate buzzer 
    CCP2CON = 0x0; 
    PORTBbits.RB3 = 0;     
} 
void Wait_One_Second_With_Beep() { // wait one second and beep 
    SEC_LED = 1; // First, turn on the SEC LED 
    Activate_Buzzer(); // Activate the buzzer
    Wait_Half_Second(); // Wait for half second (or 500 msec) 
    SEC_LED = 0; // then turn off the SEC LED 
    Deactivate_Buzzer(); // Deactivate the buzzer 
    Wait_Half_Second(); // Wait for half second (or 500 msec) 
} 
void PED_Control( char Direction, char Num_Sec){ //control pedestrian 

    Display_Lower_Digit(0x00); // turn off lower digit 
    Display_Upper_Digit(0x00); // turn on lower digit 
    
    for(char i = Num_Sec - 1; i >0; i--){
        switch(Direction){ //choose direction 
            case 0:
                Display_Upper_Digit(i); //display current number with countdown 
                break;
            case 1:
                Display_Lower_Digit(i); //display current number with countdown 
                break; 
        }
        Wait_One_Second_With_Beep(); //wait a second with beep 
    }
    Display_Lower_Digit(0x00);  // turn off lower digit 
    Display_Upper_Digit(0x00);  // turn off upper digit 
    
    Wait_One_Second_With_Beep(); //wait second with beep 
} 

void Night_Mode(){ //initiate night mode sequence 
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
void Day_Mode(){    //initiate day mode sequence 
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

