//
// FILE:   PCF8575.H
// AUTHOR:  Rob Tillaart
// DATE:  02-febr-2013
// VERSION: 0.1.02
// PURPOSE: I2C PCF8575 library for Arduino
// 

#ifndef _PCF8575_H
#define _PCF8575_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define PCF8575_LIB_VERSION "0.1.02"

#define MAX_7SEGMENT_DIGIT        4

#define HEX                       16
#define DEC                       10
#define OCT                       8
#define BIN                       2
#define BYTE                      0

class PCF8575
{
public:
  PCF8575(int address); 

  void write_seg(uint8_t msb_value,uint8_t lsb_value); 
  void write_bcd_normal(uint8_t msb_value, 
                        uint8_t lsb_value, 
                        boolean msb_dot = false, 
                        boolean lsb_dot = false); 
  void write_bcd_rotate(uint8_t msb_value, 
                        uint8_t lsb_value, 
                        boolean msb_dot = false, 
                        boolean lsb_dot = false); 
  void write_bcd_normal_dot(uint8_t msb_value, 
                        uint8_t lsb_value, 
                        boolean msb_dot = false, 
                        boolean lsb_dot = false); 
  void write_bcd_rotate_dot(uint8_t msb_value, 
                        uint8_t lsb_value, 
                        boolean msb_dot = false, 
                        boolean lsb_dot = false); 
  size_t write(uint8_t c);
  
  void print(char val,                        // Value 
             int base = BYTE);                // Value Type
             
  void print(unsigned char,                   // Value
             int base = BYTE);                // Value Type
             
  void print(int val,                         // Value
             int base = DEC);                 // Value Type
             
  void print(unsigned int val,                // Value
             int base = DEC);                 // Value Type
              
  void print(long val,                        // Value  
             int base = DEC);                 // Value Type
             
  void print(unsigned long val,               // Value 
             int base = DEC);                 // Value Type
             
  void print(double val,                      // Value
             int base = BIN);                 // Value Type(2)   

  void printNumber(long val,                  // Value
                   uint8_t = BIN);            // Value Type(2)
                   
  void printFloat(double val,                 // Value
                  uint8_t fracDigits = 2,     // frac. Digits(x.yy)
                  uint8_t base = DEC);        // Value Type
                  
  void printError(void);
  
  void writeDigitSegment(uint8_t dig_pos,     // Digit Position For Display
                         uint8_t seg_val);    // 7-Segment Value
                     
  void writeDigitBCD(uint8_t dig_pos,         // Digit Position For Display
                     uint8_t bcd_val,         // BCD[0..F] For Display
                     boolean dot = false);    // Dot 7-Segment ON/OFF Display
                                                                          
private:
  int _address;
};

#endif

//
// END OF FILE
//
