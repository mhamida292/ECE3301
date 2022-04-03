#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <math.h>
#include <usart.h>
#include <p18f4620.h> // INITIATING ALL HEADER FILES ASSOCIATED WITH PIC, STANDARD LIBRARY, UART COMMUNICATION

#pragma config OSC = INTIO67    // SET OSCILLATOR TO INTERNAL
#pragma config WDT = OFF        // SET WATCHDOG TIMER TO OFF  
#pragma config LVP = OFF        // SET LOW VOLTAGE PROGRAMMING TO OFF
#pragma config BOREN = OFF      //SET BROWN OUT RESET TO OFF

#define delay 5                     //DEFINE CONSTANT DELAY = 5
        


// DEFINE ALL COLORS TO SPECIFIC BITS DEPENDING ON LED 
#define D1_RED      PORTBbits.RB0        
#define D1_GREEN    PORTBbits.RB1
#define D1_BLUE     PORTBbits.RB2

#define D2_RED      PORTBbits.RB5
#define D2_GREEN    PORTBbits.RB6
#define D2_BLUE     PORTBbits.RB7

#define D3_RED      PORTAbits.RA4
#define D3_GREEN    PORTAbits.RA5


char array[10] = {0x40, 0x79, 0x24,0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10}; // DEFINE ALL 7SEGMENT COMBINATIONS 


void Init_ADC();                        // INITIATE ANALOG TO DIGITAL CONVERTER 
void init_UART();                       // INITIATE UART COMMUNCATION 
void Init_Tris();                       // INITIATE ALL TRISTATE INPUTS AND OUTPUTS 
void WAIT_1_SEC();                      // WAIT ONE SECOND 
unsigned int get_full_ADC(void);        // GET FULL VOLTAGE DATA FROM ANALOG TO DIGITAL 
void Display_Lower_Digit(char digit);   // DISPLAY UPPER DIGIT 7SEG DISPLAY
void Display_Upper_Digit(char digit);   // DISPLAY LOWER DIGIT 7SEG DISPLAY
void Select_ADC_Channel(char channel);  // SELECT APPROPRIATE CHANNEL TO GET ADC DATA 
void Display_D1(int temperature);       // DISPLAY D1 COLOR TO LED DEPENDING ON TEMPERATURE 
void Display_D2(int temperature);       // DISPLAY D2 COLOR TO LED DEPENDING ON TEMPERATURE 
void Display_D3(int voltage);           // DISPLAY D3 DEPENDING ON LIGHT RESISTOR VALUE 


//SET EACH COLOR COMBINATION FOR SPECIFIC LED. 
void D1_None(void);
void D1_Red(void);
void D1_Green(void);
void D1_Blue(void);
void D1_White(void);
void D2_None(void);
void D2_Red(void);
void D2_Green(void);
void D2_Yellow(void);
void D2_Blue(void);
void D2_Purple(void);
void D2_Cyan(void);
void D2_White(void);

void main(void){
    Init_ADC();  // CALL ADC CONVERTER INITIATION 
    init_UART(); // CALL UART INITIATION 
    Init_Tris(); // CALL TRISTATE INITIATION 
    
    
    PORTC = 0xFF; // SET PORT TO INPUT 
    PORTD = 0xFF; // SET PORT TO INPUT 
    PORTB = 0x00; // SET PORT TO OUTPUT 
    
    while(1){   // INFINITE LOOP 
        Select_ADC_Channel(0); //SELECT APPROPRIATE ADC CHANNEL FOR TEMPERATURE SENSOR 
        int num_step = get_full_ADC();  // GET NUMSTEP FROM ADC CHANNEL 
        float voltage_mv = num_step*4.0;
        float temp_c = (1035.0 - voltage_mv)/5.50;  // CALIBRATE CELSIUS DATA 
        float temp_f = (1.80 * temp_c) + 32.0;      // CONVERT TO F
        int intTmpF = (int)temp_f;                  // PARSE AS INT  
        char upper = intTmpF /10;                   // PARSE UPPER DIGIT 
        char lower = intTmpF %10;                   // PARSE LOWER DIGIT
        Display_Upper_Digit(upper);                 // DISPLAY UPPER DIGIT 
        Display_Lower_Digit(lower);                 // DISPLAY LOWER DIGIT
        
        Display_D1(intTmpF);                        // SEND TEMP DATA TO DISPLAY LED D1
        Display_D2(intTmpF);                        // SEND TEMP DATA TO DISPALY LED D2
        
        Select_ADC_Channel(2);                      // CHANGE ADC CHANNEL TO PHOTORESISTOR 
        int num_step = get_full_ADC();              // GET LIGHT RESISTOR DATA FROM ADC
        float pvoltage_mv = num_step*4.0;
        float Pvoltage = pvoltage_mv;               // SET NEW VOLTAGE 
        int intVoltage = (int) Pvoltage;            // PARSE AS INT 
        Display_D3(intVoltage);                     // DISPLAY ON D3 LED DEPENDING ON VOLTAGE
        //DISPLAY ALL DATA
        printf("Steps = %d \r\n", num_step);        
        printf("Voltage = %f \r\n", pvoltage_mv);
        printf("Temperature = %f/ F \r\n\n", temp_f);
        printf ("Light Volt = %f mV \r\n\n", intVoltage);
        WAIT_1_SEC(); //WAIT ONE SECOND 
    }
}


void init_UART(void){ // INIT UART CONNECTION 
    OpenUSART(USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT 
            & USART_CONT_RX & USART_BRGH_HIGH, 25);
    OSCCON = 0x60;
}
void Select_ADC_Channel(char channel) // CONNECT APPROPRIATE ADC CHANNEL 
{
    ADCON0 = channel * 4 + 1;
}
void Init_Tris(){ // INITIATE TRISTATE CONNECTIONS 
    TRISA = 0x0F; 
    TRISB = 0x00; 
    TRISC = 0x00; 
    TRISD = 0x00; 
    TRISE = 0x00; 
}
void Init_ADC(void){    // INITIAILIZE ANALOG TO DIGITAL CONVERSION 
    //ADCON0=0x01;
    // select channel AN0, and turn on the ADC subsystem
    ADCON1=0x1B;
    // select pins AN0 through AN3 as analog signal, VDD-VSS as
    // reference voltage
    ADCON2=0xA9;
    // right justify the result. Set the bit conversion time (TAD) and
    // acquisition time
}
unsigned int get_full_ADC(void){    // GET DATA FROM ADC CHANNEL 
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
void putch (char c){
    while (!TRMT);
    TXREG = c;
}

void WAIT_1_SEC(){ // WAIT ONE SECOND 
    for(int j = 0; j<17000; j++); 
}
void Display_Upper_Digit(char digit){ // DISPLAY UPPER DIGIT TO 7SEG
    PORTC = array[digit] & 0x3F;
    if((array[digit] & 0x40) == 0x00)
        PORTE = 0;
    else
        PORTE = 1;
}

void Display_Lower_Digit(char digit){// DISPLAY LOWER DIGIT TO 7SEG
    PORTD = array[digit]; 
    //PORTD = 0x00; 
}

void Display_D1(int temperature){ // CHECK TEMP AND DISPLAY COLOR TO D1 LED 
    if(temperature <= 45) D1_None();
    else if(temperature >= 46 && temperature<= 55) D1_Red(); 
    else if(temperature >= 56 && temperature<= 65) D1_Green(); 
    else if(temperature >= 66 && temperature <= 75) D1_Blue(); 
    else if(temperature >= 76) D1_White();
}

void Display_D2(int temperature){// CHECK TEMP AND DISPLAY COLOR TO D2 LED 
    if(temperature <= 10) D2_None();
    else if(temperature >= 10 && temperature<= 19) D2_Red(); 
    else if(temperature >= 20 && temperature<= 29) D2_Green(); 
    else if(temperature >= 30 && temperature<= 39) D2_Yellow(); 
    else if(temperature >= 40 &&temperature <= 49) D2_Blue();
    else if(temperature >= 50 &&temperature <= 59) D2_Purple();
    else if(temperature >= 60 &&temperature <= 69) D2_Cyan();
    else if(temperature >= 70 ) D2_White();
}

void Display_D3(int voltage){// CHECK VOLTAGE FROM PHOTORESISTOR AND DISPLAY COLOR TO D3 LED 
    if(voltage < 2500)
        PORTA = 0x01<<4;
    else if (voltage >=2500 && voltage < 3400)
        PORTA = 0x02<<4;
    else
        PORTA = 0x03<<4;
}
// COLOR COMBINATIONS FOR LEDS D1 - D2
void D1_None(void)
{
    D1_RED = 0;
    D1_GREEN = 0;
    D1_BLUE = 0;
}

void D1_Red(void)
{
    D1_RED = 1;
    D1_GREEN = 0;
    D1_BLUE = 0;
}

void D1_Green(void)
{
    D1_RED = 0;
    D1_GREEN = 1;
    D1_BLUE = 0;
}

void D1_Blue(void)
{
    D1_RED = 0;
    D1_GREEN = 0;
    D1_BLUE = 1;
}

void D1_White(void)
{
    D1_RED = 1;
    D1_GREEN = 1;
    D1_BLUE = 1;
}

void D2_None(void)
{
    D2_RED = 0;
    D2_GREEN = 0;
    D2_BLUE = 0;
}

void D2_Red(void)
{
    D2_RED = 1;
    D2_GREEN = 0;
    D2_BLUE = 0;
}

void D2_Green(void)
{
    D2_RED = 0;
    D2_GREEN = 1;
    D2_BLUE = 0;
}
void D2_Yellow(void)
{
    D2_RED = 1;
    D2_GREEN = 1;
    D2_BLUE = 0;
}

void D2_Blue(void)
{
    D2_RED = 0;
    D2_GREEN = 0;
    D2_BLUE = 1;
}

void D2_Purple(void)
{
    D2_RED = 1;
    D2_GREEN = 0;
    D2_BLUE = 1;
}

void D2_Cyan(void)
{
    D2_RED = 0;
    D2_GREEN = 1;
    D2_BLUE = 1;
}

void D2_White(void)
{
    D2_RED = 1;
    D2_GREEN = 1;
    D2_BLUE = 1;
}



  