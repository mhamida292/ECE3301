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

#define NS              0 // Number definition of North/South 
#define NSLT            1 // Number definition of North/South Left Turn 
#define EW              2 // Number definition of East/West 
#define EWLT            3 // Number definition of East/West Left Turn

#define Circle_Size 7
#define Circle_Offset   15
#define TS_1            1
#define TS_2            2
#define Count_Offset    10

#define XTXT            30              // X location of Title Text 
#define XRED            40              // X location of Red Circle
#define XYEL            60              // X location of Yellow Circle
#define XGRN            80              // X location of Green Circle
#define XCNT            100             // X location of Sec Count

#define _XTAL_FREQ      8000000         // Set operation for 8 Mhz
#define TMR_CLOCK       _XTAL_FREQ/4    // Timer Clock 2 Mhz

#define TFT_DC          PORTCbits.RC0     		// Location of TFT D/C
#define TFT_CS          PORTCbits.RC2    		// Location of TFT Chip Select
#define TFT_RST         PORTCbits.RC4    		// Location of TFT Reset
// DEFAULT VALUES FOR NSLT, NS, EW, EWLT PHYSICAL PROPERTIES TO SEND TO TFT DISPLAY 
#define NS_Txt_Y        20
#define NS_Cir_Y        NS_Txt_Y + Circle_Offset
#define NS_Count_Y      NS_Txt_Y + Count_Offset
#define NS_Color        ST7735_BLUE 

#define NSLT_Txt_Y      50
#define NSLT_Cir_Y      NSLT_Txt_Y + Circle_Offset
#define NSLT_Count_Y    NSLT_Txt_Y + Count_Offset
#define NSLT_Color      ST7735_MAGENTA

#define EW_Txt_Y        80
#define EW_Cir_Y        EW_Txt_Y + Circle_Offset
#define EW_Count_Y      EW_Txt_Y + Count_Offset
#define EW_Color        ST7735_CYAN

#define EWLT_Txt_Y      110
#define EWLT_Cir_Y      EWLT_Txt_Y + Circle_Offset
#define EWLT_Count_Y    EWLT_Txt_Y + Count_Offset
#define EWLT_Color      ST7735_WHITE

#define PED_NS_Count_Y  30
#define PED_EW_Count_Y  90
#define PED_Count_X     2
#define Switch_Txt_Y    140

//Function PROTOTYPES
void Init_ADC();    // Initialize Analog to digital converter 
void init_UART();   // initialize UART connection 
void Init_Tris();   // Initialize TRIS connection for ports 
unsigned int get_full_ADC(void);        // Get the value from analog port
void Display_Lower_Digit(char digit);   // Display lower digit on 7seg LED
void Display_Upper_Digit(char digit);  // Display upper digit on 7seg LED
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
void update_LCD_color(char direction, char color); // Update the color on LCD display
void Initialize_Screen(); //Initialize the screen method 
void update_LCD_count(char direction, char count); //Update the LCD count based off direction 
void update_LCD_PED_Count(char direction, char count);
void update_LCD_misc();

char array[10] = {0xFF, 0x79, 0x24,0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10}; //Array of numbers to drive 7seg
char buffer[31];                        // general buffer for display purpose
char *nbr;                              // general pointer used for buffer
char *txt;

char NS_Count[]     = "00";             // text storage for NS Count
char NSLT_Count[]   = "00";             // text storage for NS Left Turn Count
char EW_Count[]     = "00";             // text storage for EW Count
char EWLT_Count[]   = "00";             // text storage for EW Left Turn Count

char PED_NS_Count[] = "00";             // text storage for NS Pedestrian Count
char PED_EW_Count[] = "00";             // text storage for EW Pedestrian Count

char SW_NSPED_Txt[] = "0";              // text storage for NS Pedestrian Switch
char SW_NSLT_Txt[]  = "0";              // text storage for NS Left Turn Switch
char SW_EWPED_Txt[] = "0";              // text storage for EW Pedestrian Switch
char SW_EWLT_Txt[]  = "0";              // text storage for EW Left Turn Switch

char SW_MODE_Txt[]  = "D";              // text storage for Mode Light Sensor
    
char Act_Mode_Txt[]  = "D";             // text storage for Actual Mode
char FlashingS_Txt[] = "0";             // text storage for Emergency Status
char FlashingR_Txt[] = "0";             // text storage for Flashing Request

char dir;
char Count;                             // RAM variable for Second Count
char PED_Count;                         // RAM variable for Second Pedestrian Count

char SW_NSPED;                          // RAM variable for NS Pedestrian Switch
char SW_NSLT;                           // RAM variable for NS Left Turn Switch
char SW_EWPED;                          // RAM variable for EW Pedestrian Switch
char SW_EWLT;                           // RAM variable for EW Left Turn Switch
char SW_MODE;                           // RAM variable for Mode Light Sensor
char FLASHING;
char FLASHING_REQUEST;
int MODE;
char direction;
float volt;
#include "ST7735_TFT.c" //Include TFT display screen

void main(void){
    //Initiation methods!
    Init_ADC();
    init_UART();
    Init_Tris();            // INITIALIZE TRISTATE CONNECTIONS 
        
    OSCCON = 0x70; 
    RBPU = 0;    
    Initialize_Screen();                        // Initialize the TFT screen
    while (1){  //Infinite loop
        Select_ADC_Channel(0); //choose correct ADC channel for the light resistor 
        int num_step = get_full_ADC(); // Get the correct numstep from get_full_adc
        float pvoltage_mv = num_step*4.0; // multiple numstep by 4
        float Pvoltage = pvoltage_mv;  
        int finalVoltage = (int)Pvoltage; // parse to int  
        
        if(finalVoltage < 2250){ // if output from light resistor is over 3000, run day mode
            MODE_LED = 1; 
            Day_Mode();
        }else{// if output from light resistor is less than 3000, run night mode
            MODE_LED = 0; 
            Night_Mode(); 
        }
    }
}
void putch (char c){
    while (!TRMT);
    TXREG = c;
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
void Wait_One_Second()							//creates one second delay and blinking asterisk
{
    SEC_LED = 1;
    strcpy(txt,"*");
    drawtext(120,10,txt,ST7735_WHITE,ST7735_BLACK,TS_1);
    Wait_Half_Second();                         // Wait for half second (or 500 msec)

    SEC_LED = 0;
    strcpy(txt," ");
    drawtext(120,10,txt,ST7735_WHITE,ST7735_BLACK,TS_1);
    Wait_Half_Second();                         // Wait for half second (or 500 msec)
    update_LCD_misc();
}
void Wait_Half_Second()                         // WAIT HALF A SECOND 
{
    T0CON = 0x03;                               // Timer 0, 16-bit mode, prescaler 1:16
    TMR0L = 0xDB;                               // set the lower byte of TMR
    TMR0H = 0x0B;                               // set the upper byte of TMR
    INTCONbits.TMR0IF = 0;                      // clear the Timer 0 flag
    T0CONbits.TMR0ON = 1;                       // Turn on the Timer 0
    while (INTCONbits.TMR0IF == 0);             // wait for the Timer Flag to be 1 for done
    T0CONbits.TMR0ON = 0;                       // turn off the Timer 0
}

void Wait_N_Seconds (char seconds)              // WAIT N AMOUNT OF SECONDS BY CALLING THE WAIT ONE SECOND FUNCTION N TIMES
{
    char I;
    for (I = seconds; I> 0; I--)
    {
        update_LCD_count(direction, I);  
        Wait_One_Second();                      // calls Wait_One_Second for x number of times
      
    }
    update_LCD_count(direction, 0);   
}


void Set_NS(char color){ //set NORTH SOUTH to various color combos 
    direction = NS; 
    update_LCD_color(direction, color);
    switch(color){
        case OFF: NS_RED=0; NS_GREEN=0; break; 
        case RED: NS_RED=1; NS_GREEN=0; break; 
        case GREEN: NS_RED=0; NS_GREEN=1; break; 
        case YELLOW: NS_RED=1; NS_GREEN=1; break; 
    }
}
void Set_NSLT(char color){ //set NORTH SOUTH left right signal to various color combos 
    direction = NSLT;
    update_LCD_color(direction, color);
    switch(color){
        case OFF: NSLT_RED=0; NSLT_GREEN=0; break; 
        case RED: NSLT_RED=1; NSLT_GREEN=0; break; 
        case GREEN: NSLT_RED=0; NSLT_GREEN=1; break; 
        case YELLOW: NSLT_RED=1; NSLT_GREEN=1; break; 
    }
}
void Set_EW(char color){ //set EAST WEST to various color combos 
    direction = EW;
    update_LCD_color(direction, color);
    switch(color){
        case OFF: EW_RED=0; EW_GREEN=0; break; 
        case RED: EW_RED=1; EW_GREEN=0; break; 
        case GREEN: EW_RED=0; EW_GREEN=1; break; 
        case YELLOW: EW_RED=1; EW_GREEN=1; break; 
    }
}
void Set_EWLT(char color){//set EAST WEST left right signal to various color combos 
    direction = EWLT;
    update_LCD_color(direction, color);
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
void Wait_One_Second_With_Beep()				// creates one second delay as well as sound buzzer
{
    SEC_LED = 1;
    strcpy(txt,"*");
    drawtext(120,10,txt,ST7735_WHITE,ST7735_BLACK,TS_1);
    Activate_Buzzer();
    Wait_Half_Second();                                                                             // Wait for half second (or 500 msec)

    SEC_LED = 0;
    strcpy(txt," ");
    drawtext(120,10,txt,ST7735_WHITE,ST7735_BLACK,TS_1);
    Deactivate_Buzzer();
    Wait_Half_Second();                                                                             // Wait for half second (or 500 msec)
    update_LCD_misc();                                                                              // Refreshes our LCD
}
void PED_Control( char Direction, char Num_Sec){ //control pedestrian     
    for(char i = Num_Sec - 1; i >0 ; i--){
        update_LCD_PED_Count(Direction, i);
        Wait_One_Second_With_Beep(); //wait a second with beep 
    } 
    update_LCD_PED_Count(direction, 0); 
    update_LCD_PED_Count(1, 0); 
    Wait_One_Second_With_Beep();                // leaves the 7-Segment off for 1 second
} 

void Night_Mode(){ //initiate night mode sequence 
    MODE = 0;                                   // turns on the MODE_LED
    MODE_LED = 0;
    Act_Mode_Txt[0] = 'N';
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
    MODE = 1;                                   // turns on the MODE_LED
    MODE_LED = 1;
    Act_Mode_Txt[0] = 'D';
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
    
void update_LCD_color (char direction, char color){ // UPDATE LCD COLORS 
    char Circle_Y; 
    Circle_Y = NS_Cir_Y + direction * 30; 

    if (color == OFF){
        //Fills
        fillCircle(XRED, Circle_Y, Circle_Size, ST7735_BLACK);
        fillCircle(XYEL, Circle_Y, Circle_Size, ST7735_BLACK);
        fillCircle(XGRN, Circle_Y, Circle_Size, ST7735_BLACK); 
        //Outlines
        drawCircle(XRED, Circle_Y, Circle_Size, ST7735_RED);            
        drawCircle(XYEL, Circle_Y, Circle_Size, ST7735_YELLOW);
        drawCircle(XGRN, Circle_Y, Circle_Size, ST7735_GREEN);                       
    }  
    
    if(color == RED){ //only fill red with red 
        //Fills
        fillCircle(XRED, Circle_Y, Circle_Size, ST7735_RED);
        fillCircle(XYEL, Circle_Y, Circle_Size, ST7735_BLACK);
        fillCircle(XGRN, Circle_Y, Circle_Size, ST7735_BLACK); 
        //Outlines
        drawCircle(XRED, Circle_Y, Circle_Size, ST7735_RED);            
        drawCircle(XYEL, Circle_Y, Circle_Size, ST7735_YELLOW);
        drawCircle(XGRN, Circle_Y, Circle_Size, ST7735_GREEN);   
    }
    
     if(color == YELLOW){ //only fill yellow with yellow 
        //Fills
        fillCircle(XRED, Circle_Y, Circle_Size, ST7735_BLACK);
        fillCircle(XYEL, Circle_Y, Circle_Size, ST7735_YELLOW);
        fillCircle(XGRN, Circle_Y, Circle_Size, ST7735_BLACK); 
        //Outlines
        drawCircle(XRED, Circle_Y, Circle_Size, ST7735_RED);            
        drawCircle(XYEL, Circle_Y, Circle_Size, ST7735_YELLOW);
        drawCircle(XGRN, Circle_Y, Circle_Size, ST7735_GREEN);   
    }
    if(color == GREEN){ //only fill green with green 
        //Fills
        fillCircle(XRED, Circle_Y, Circle_Size, ST7735_BLACK);
        fillCircle(XYEL, Circle_Y, Circle_Size, ST7735_BLACK);
        fillCircle(XGRN, Circle_Y, Circle_Size, ST7735_GREEN); 
        //Outlines
        drawCircle(XRED, Circle_Y, Circle_Size, ST7735_RED);            
        drawCircle(XYEL, Circle_Y, Circle_Size, ST7735_YELLOW);
        drawCircle(XGRN, Circle_Y, Circle_Size, ST7735_GREEN);   
    }
}

void Initialize_Screen()    // INITIALIZE SCREEN WITH DEFAULT VALUES 
{
  LCD_Reset();
  TFT_GreenTab_Initialize();
  fillScreen(ST7735_BLACK);
  
  /* TOP HEADER FIELD */
  txt = buffer;
  strcpy(txt, "ECE3301L SPRING 22-S3");  
  drawtext(2, 2, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  
  /* MODE FIELD */
  strcpy(txt, "Mode:");
  drawtext(2, 10, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(35,10, Act_Mode_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);

  /* FLASHING REQUEST FIELD */
  strcpy(txt, "FR:");
  drawtext(50, 10, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(70, 10, FlashingR_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  
  /* FLASHING STATUS FIELD */
  strcpy(txt, "FS:");
  drawtext(80, 10, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(100, 10, FlashingR_Txt, ST7735_WHITE, ST7735_BLACK, TS_1); 

  /* SECOND UPDATE FIELD */
  strcpy(txt, "*");
  drawtext(120, 10, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
      
  /* NORTH/SOUTH UPDATE FIELD */
  strcpy(txt, "NORTH/SOUTH");
  drawtext  (XTXT, NS_Txt_Y  , txt, NS_Color, ST7735_BLACK, TS_1);
  drawRect  (XTXT, NS_Cir_Y-8, 60, 18, NS_Color);
  drawCircle(XRED, NS_Cir_Y  , Circle_Size, ST7735_RED);
  drawCircle(XYEL, NS_Cir_Y  , Circle_Size, ST7735_YELLOW);
  fillCircle(XGRN, NS_Cir_Y  , Circle_Size, ST7735_GREEN);
  drawtext  (XCNT, NS_Count_Y, NS_Count, NS_Color, ST7735_BLACK, TS_2);
    
  /* NORTH/SOUTH LEFT TURN UPDATE FIELD */
  strcpy(txt, "N/S LT");
  drawtext  (XTXT, NSLT_Txt_Y, txt, NSLT_Color, ST7735_BLACK, TS_1);
  drawRect  (XTXT, NSLT_Cir_Y-8, 60, 18, NSLT_Color);
  fillCircle(XRED, NSLT_Cir_Y, Circle_Size, ST7735_RED);
  drawCircle(XYEL, NSLT_Cir_Y, Circle_Size, ST7735_YELLOW);
  drawCircle(XGRN, NSLT_Cir_Y, Circle_Size, ST7735_GREEN);  
  drawtext  (XCNT, NSLT_Count_Y, NSLT_Count, NSLT_Color, ST7735_BLACK, TS_2);
  
  /* EAST/WEST UPDATE FIELD */
  strcpy(txt, "EAST/WEST");
  drawtext  (XTXT, EW_Txt_Y, txt, EW_Color, ST7735_BLACK, TS_1);
  drawRect  (XTXT, EW_Cir_Y-8, 60, 18, EW_Color);
  fillCircle(XRED, EW_Cir_Y, Circle_Size, ST7735_RED);
  drawCircle(XYEL, EW_Cir_Y, Circle_Size, ST7735_YELLOW);
  drawCircle(XGRN, EW_Cir_Y, Circle_Size, ST7735_GREEN);  
  drawtext  (XCNT, EW_Count_Y, EW_Count, EW_Color, ST7735_BLACK, TS_2);

  /* EAST/WEST LEFT TURN UPDATE FIELD */
  strcpy(txt, "E/W LT");
  drawtext  (XTXT, EWLT_Txt_Y, txt, EWLT_Color, ST7735_BLACK, TS_1);
  drawRect  (XTXT, EWLT_Cir_Y-8, 60, 18, EWLT_Color);  
  fillCircle(XRED, EWLT_Cir_Y, Circle_Size, ST7735_RED);
  drawCircle(XYEL, EWLT_Cir_Y, Circle_Size, ST7735_YELLOW);
  drawCircle(XGRN, EWLT_Cir_Y, Circle_Size, ST7735_GREEN);   
  drawtext  (XCNT, EWLT_Count_Y, EWLT_Count, EWLT_Color, ST7735_BLACK, TS_2);

  /* NORTH/SOUTH PEDESTRIAM UPDATE FIELD */
  strcpy(txt, "PNS");  
  drawtext(3, NS_Txt_Y, txt, NS_Color, ST7735_BLACK, TS_1);
  drawtext(2, PED_NS_Count_Y, PED_NS_Count, NS_Color, ST7735_BLACK, TS_2);
  
  /* EAST/WEST PEDESTRIAM UPDATE FIELD */  
  drawtext(2, PED_EW_Count_Y, PED_EW_Count, EW_Color, ST7735_BLACK, TS_2);
  strcpy(txt, "PEW");  
  drawtext(3, EW_Txt_Y, txt, EW_Color, ST7735_BLACK, TS_1);
      
  /* MISCELLANEOUS UPDATE FIELD */  
  strcpy(txt, "NSP NSLT EWP EWLT MR");
  drawtext(1,  Switch_Txt_Y, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(6,  Switch_Txt_Y+9, SW_NSPED_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(32, Switch_Txt_Y+9, SW_NSLT_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(58, Switch_Txt_Y+9, SW_EWPED_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(87, Switch_Txt_Y+9, SW_EWLT_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);  
  drawtext(112,Switch_Txt_Y+9, SW_MODE_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
}

void update_LCD_count(char direction, char count){  // UPDATE LCD COUNTER 
   switch (direction)                       
   {
      case NS:       
        NS_Count[0] = count/10  + '0';
        NS_Count[1] = count%10  + '0';
        drawtext(XCNT, NS_Count_Y, NS_Count, NS_Color, ST7735_BLACK, TS_2); 
        break;
      
      case NSLT:       
        NSLT_Count[0] = count/10  + '0';
        NSLT_Count[1] = count%10  + '0';
        drawtext(XCNT, NSLT_Count_Y, NSLT_Count, NSLT_Color, ST7735_BLACK, TS_2); 
        break;
        
      case EW:       
        EW_Count[0] = count/10  + '0';
        EW_Count[1] = count%10  + '0';
        drawtext(XCNT, EW_Count_Y, EW_Count, EW_Color, ST7735_BLACK, TS_2); 
        break;
        
      case EWLT:       
        EWLT_Count[0] = count/10  + '0';
        EWLT_Count[1] = count%10  + '0';
        drawtext(XCNT, EWLT_Count_Y, EWLT_Count, EWLT_Color, ST7735_BLACK, TS_2); 
        break;
    }  
}
void update_LCD_PED_Count(char direction, char count)   // UPDATE PEDESTRIAN LCD COUNTER 
{
   switch (direction)
   {
      case 1:       
        PED_EW_Count[0] = count/10  + '0';          // PED count upper digit
        PED_EW_Count[1] = count%10  + '0';          // PED Lower
        drawtext(PED_Count_X, PED_EW_Count_Y, PED_EW_Count, EW_Color, ST7735_BLACK, TS_2);        
        break;
      case 0:       
        PED_NS_Count[0] = count/10  + '0';          // PED count upper digit
        PED_NS_Count[1] = count%10  + '0';          // PED Lower
        drawtext(PED_Count_X, PED_NS_Count_Y, PED_NS_Count, NS_Color, ST7735_BLACK, TS_2);        
        break;
   }
}

void update_LCD_misc()
{
    int nStep = get_full_ADC();         // calculates the # of steps for analog conversion
    volt = nStep * 5 /1024.0;           // gets the voltage in Volts, using 5V as reference s instead of 4, also divide by 1024 
    SW_MODE = volt < 2.25 ? 1:0;         // Mode = 1, Day_mode, Mode = 0 Night_mode
    printf("Voltage = %f \r\n", volt);
    SW_EWPED = EWPED_SW;
    SW_EWLT = EWLT_SW;    
    SW_NSPED = NSPED_SW;
    SW_NSLT = NSLT_SW;
    
    if (SW_MODE == 0) SW_MODE_Txt[0]= 'N'; else SW_MODE_Txt[0] = 'D';
    if (SW_EWPED == 0) SW_EWPED_Txt[0] = '0'; else SW_EWPED_Txt[0] = '1';      // Set Text at bottom of screen to switch states
    if (SW_EWLT == 0) SW_EWLT_Txt[0] = '0'; else SW_EWLT_Txt[0] = '1';         // Set Text at bottom of screen to switch states
    if (SW_NSPED == 0) SW_NSPED_Txt[0] = '0'; else SW_NSPED_Txt[0] = '1';      // Set Text at bottom of screen to switch states
    if (SW_NSLT == 0) SW_NSLT_Txt[0] = '0'; else SW_NSLT_Txt[0] = '1';         // Set Text at bottom of screen to switch states
   
    drawtext(35,10, Act_Mode_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);    
    drawtext(6,   Switch_Txt_Y+9, SW_EWPED_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
    drawtext(32,  Switch_Txt_Y+9, SW_EWLT_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
    drawtext(58,  Switch_Txt_Y+9, SW_NSPED_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);       // Show switch and sensor states at bottom of the screen
    drawtext(87,  Switch_Txt_Y+9, SW_NSLT_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
    drawtext(112,  Switch_Txt_Y+9, SW_MODE_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
}