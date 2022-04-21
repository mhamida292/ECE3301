#include <stdio.h>

#include <p18f4620.h>
#include "I2C_Support.h"
#include "I2C_Soft.h"

#define ACCESS_CFG      0xAC
#define START_CONV      0xEE
#define READ_TEMP       0xAA
#define CONT_CONV       0x02
#define ACK     1
#define NAK     0


extern unsigned char second, minute, hour, dow, day, month, year;
extern unsigned char setup_second, setup_minute, setup_hour, setup_day, setup_month, setup_year;
extern unsigned char alarm_second, alarm_minute, alarm_hour, alarm_date;
extern unsigned char setup_alarm_second, setup_alarm_minute, setup_alarm_hour;


void DS1621_Init()
{
    char Device = 0x48; 
    I2C_Write_Cmd_Write_Data (Device, ACCESS_CFG, CONT_CONV);
    I2C_Write_Cmd_Only(Device, START_CONV);                 
}


int DS1621_Read_Temp()
{
    char Device = 0x48; 
    char Cmd = READ_TEMP; 
    char Data_Ret;    
    I2C_Start();                      // Start I2C protocol
    I2C_Write((Device << 1) | 0);     // Device address
    I2C_Write(Cmd);                   // Send register address
    I2C_ReStart();                    // Restart I2C
    I2C_Write((Device << 1) | 1);     // Initialize data read
    Data_Ret = I2C_Read(NAK);         //
    I2C_Stop(); 
    return Data_Ret;       
}

void DS3231_Read_Time()
{
          
}

void DS3231_Setup_Time()
{

}

