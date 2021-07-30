//
// FILE:   PCF8575.cpp
// AUTHOR:  Rob Tillaart
// DATE:  02-febr-2013
// VERSION: 0.1.02
// PURPOSE: I2C PCF8575 library for Arduino
// 

#include "PCF8575.h"
#include <Wire.h>

// Data    : D7 D6 D5 D4 D3 D2 D1 D0
// Segmenyt: DP  G  F  E  D  C  B  A
static const uint8_t bcd_num_tab_normal[] = 
{
            // 0 : DP G  F  E  D  C  B  A
  0x3F,     // 0 : 0  0  1  1  1  1  1  1
  0x06,     // 1 : 0  0  0  0  0  1  1  0
  0x5B,     // 2 : 0  1  0  1  1  0  1  1
  0x4F,     // 3 : 0  1  0  0  1  1  1  1
  0x66,     // 4 : 0  1  1  0  0  1  1  0
  0x6D,     // 5 : 0  1  1  0  1  1  0  1
  0x7D,     // 6 : 0  1  1  1  1  1  0  1
  0x07,     // 7 : 0  0  0  0  0  1  1  1
  0x7F,     // 8 : 0  1  1  1  1  1  1  1
  0x6F,     // 9 : 0  1  1  0  1  1  1  1
  0x77,     // A : 0  1  1  1  0  1  1  1
  0x7C,     // B : 0  1  1  1  1  1  0  0
  0x39,     // C : 0  0  1  1  1  0  0  1
  0x5E,     // D : 0  1  0  1  1  1  1  0
  0x79,     // E : 0  1  1  1  1  0  0  1
  0x71,     // F : 0  1  1  1  0  0  0  1
};

static const uint8_t bcd_num_tab_normal_dot[] = 
{
            // 0 : DP G  F  E  D  C  B  A
  0xBF,     // 0 : 1  0  1  1  1  1  1  1
  0x86,     // 1 : 1  0  0  0  0  1  1  0
  0xDB,     // 2 : 1  1  0  1  1  0  1  1
  0xCF,     // 3 : 1  1  0  0  1  1  1  1
  0xE6,     // 4 : 1  1  1  0  0  1  1  0
  0xED,     // 5 : 1  1  1  0  1  1  0  1
  0xFD,     // 6 : 1  1  1  1  1  1  0  1
  0x87,     // 7 : 1  0  0  0  0  1  1  1
  0xFF,     // 8 : 1  1  1  1  1  1  1  1
  0xEF,     // 9 : 1  1  1  0  1  1  1  1
};

// Data    : D7 D6 D5 D4 D3 D2 D1 D0
// Segmenyt: DP  G  F  E  D  C  B  A
static const uint8_t bcd_num_tab_rotate[] = 
{
            // 0 : DP G  F  E  D  C  B  A
  0x3F,     // 0 : 0  0  1  1  1  1  1  1
  0x30,     // 1 : 0  0  1  1  0  0  0  0
  0x5B,     // 2 : 0  1  0  1  1  0  1  1
  0x79,     // 3 : 0  1  1  1  1  0  0  1
  0x74,     // 4 : 0  1  1  1  0  1  0  0
  0x6D,     // 5 : 0  1  1  0  1  1  0  1
  0x6F,     // 6 : 0  1  1  0  1  1  1  1
  0x38,     // 7 : 0  0  1  1  1  0  0  0
  0x7F,     // 8 : 0  1  1  1  1  1  1  1
  0x7D,     // 9 : 0  1  1  1  1  1  0  1
  0x7E,     // A : 0  1  1  1  1  1  1  0
  0x67,     // B : 0  1  1  0  0  1  1  1
  0x0F,     // C : 0  0  0  0  1  1  1  1
  0x73,     // D : 0  1  1  1  0  0  1  1
  0x4F,     // E : 0  1  0  0  1  1  1  1  
  0x4E,     // F : 0  1  0  0  1  1  1  0
};


static const uint8_t bcd_num_tab_rotate_dot[] = 
{
            // 0 : DP G  F  E  D  C  B  A
  0xBF,     // 0 : 1  0  1  1  1  1  1  1
  0xB0,     // 1 : 1  0  1  1  0  0  0  0
  0xDB,     // 2 : 1  1  0  1  1  0  1  1
  0xF9,     // 3 : 1  1  1  1  1  0  0  1
  0xF4,     // 4 : 1  1  1  1  0  1  0  0
  0xED,     // 5 : 1  1  1  0  1  1  0  1
  0xEF,     // 6 : 1  1  1  0  1  1  1  1
  0xB8,     // 7 : 1  0  1  1  1  0  0  0
  0xFF,     // 8 : 1  1  1  1  1  1  1  1
  0xFD,     // 9 : 1  1  1  1  1  1  0  1
};

PCF8575::PCF8575(int address) 
{
  _address = address;
  Wire.begin();
}

//=====================================================================
// Write 7-Segment Display Value
//=====================================================================
// Data    : D7 D6 D5 D4 D3 D2 D1 D0
// Segmenyt: DP  G  F  E  D  C  B  A
//=====================================================================
void PCF8575::write_seg(uint8_t msb_value, uint8_t lsb_value)
{
  Wire.beginTransmission(_address);                                   // Begin I2C
  Wire.write(lsb_value);                                              // LSB Side(Right Digit)
  Wire.write(msb_value);                                              // MSB Side(Left Digit)
  Wire.endTransmission();                                             // End I2C
}

//=====================================================================
// Write BCD Display Normal Value
//=====================================================================
// Data MSB : Left Digit Display
// Data LSB : Right Digit Display
//=====================================================================
void PCF8575::write_bcd_normal(uint8_t msb_value,                     // Left Side Digit Segment
                               uint8_t lsb_value,                     // Right Sid Digit Segment
                               boolean msb_dot = false,               // Left Side Digit Dot
                               boolean lsb_dot = false)               // Right Side Digit Dot
{
  Wire.beginTransmission(_address);                                   // Begin I2C
  Wire.write(bcd_num_tab_normal[lsb_value] | (lsb_dot << 7));         // LSB Side(Right Digit)
  Wire.write(bcd_num_tab_normal[msb_value] | (msb_dot << 7));         // MSB Side(Left Digit)
  Wire.endTransmission();                                             // End I2C
}


void PCF8575::write_bcd_normal_dot(uint8_t msb_value,                     // Left Side Digit Segment
                               uint8_t lsb_value,                     // Right Sid Digit Segment
                               boolean msb_dot = false,               // Left Side Digit Dot
                               boolean lsb_dot = false)               // Right Side Digit Dot
{
  Wire.beginTransmission(_address);                                   // Begin I2C
  Wire.write(bcd_num_tab_normal_dot[lsb_value] | (lsb_dot << 7));         // LSB Side(Right Digit)
  Wire.write(bcd_num_tab_normal_dot[msb_value] | (msb_dot << 7));         // MSB Side(Left Digit)
  Wire.endTransmission();                                             // End I2C
}

//=====================================================================
// Write BCD Display Rotate Value
//=====================================================================
// Data MSB : Left Digit Display
// Data LSB : Right Digit Display
//=====================================================================
void PCF8575::write_bcd_rotate(uint8_t msb_value,                     // Left Side Digit Segment
                               uint8_t lsb_value,                     // Right Sid Digit Segment
                               boolean msb_dot = false,               // Left Side Digit Dot
                               boolean lsb_dot = false)               // Right Side Digit Dot
{
  Wire.beginTransmission(_address);                                   // Begin I2C
  Wire.write(bcd_num_tab_rotate[lsb_value] | (lsb_dot << 7));         // LSB Side(Right Digit)
  Wire.write(bcd_num_tab_rotate[msb_value] | (msb_dot << 7));         // MSB Side(Left Digit)
  Wire.endTransmission();                                             // End I2C
}


void PCF8575::write_bcd_rotate_dot(uint8_t msb_value,                     // Left Side Digit Segment
                               uint8_t lsb_value,                     // Right Sid Digit Segment
                               boolean msb_dot = false,               // Left Side Digit Dot
                               boolean lsb_dot = false)               // Right Side Digit Dot
{
  Wire.beginTransmission(_address);                                   // Begin I2C
  Wire.write(bcd_num_tab_rotate_dot[lsb_value] | (lsb_dot << 7));         // LSB Side(Right Digit)
  Wire.write(bcd_num_tab_rotate_dot[msb_value] | (msb_dot << 7));         // MSB Side(Left Digit)
  Wire.endTransmission();                                             // End I2C
}
//==========================================================================================
//==========================================================================================
void PCF8575::print(char val,                                                 // Value 
                                int base)                                                 // Value Type
{
  print((long) val, base);
}
//==========================================================================================

//==========================================================================================
//==========================================================================================
void PCF8575::print(unsigned char val,                                        // Value 
                                int base)                                                 // Value Type
{
  print((unsigned long) val, base);
}
//==========================================================================================

//==========================================================================================
//==========================================================================================
void PCF8575::print(int val,                                                  // Value 
                                int base)                                                 // Value Type
{
  print((long) val, base);
}
//==========================================================================================

//==========================================================================================
//==========================================================================================
void PCF8575::print(unsigned int val,                                         // Value 
                                int base)                                                 // Value Type
{
  print((unsigned long) val, base);
}
//==========================================================================================

//==========================================================================================
//==========================================================================================
void PCF8575::print(long val,                                                 // Value 
                                int base)                                                 // Value Type
{
  printNumber(val, base);
}
//==========================================================================================

//==========================================================================================
//==========================================================================================
void PCF8575::print(unsigned long val,                                        // Value 
                                int base)                                                 // Value Type
{
  if(base == BYTE) 
  {
    write(val);
  }
  else 
  {
    printNumber(val, base);
  }
}
//==========================================================================================

//==========================================================================================
//==========================================================================================
void PCF8575::print(double val,                                               // Value 
                                int base)                                                 // Value Type(2:BIN)
{
  printFloat(val, base);                                                                  // x.yy(DEC)
}
//==========================================================================================

//==========================================================================================
//==========================================================================================
void PCF8575::printNumber(long val,                                           // Value 
                                      uint8_t base)                                       // Value Type
{
  printFloat(val, 0, base);                                                               // 0 fracDigits Display
}
//==========================================================================================

//==========================================================================================
// print Float value to 7-Segment Display
// val          : Value of float 
// fracDigits   : number of digit frac. Display(Default : 2) : x.yy
// base         : Type of Value(Default : DEC)
//==========================================================================================
void PCF8575::printFloat(double val,                                          // Value 
                         uint8_t fracDigits,                                  // frac. Digits(x.yy)
                         uint8_t base)                                        // Value Type(DEC)
{ 
  uint8_t numericDigits = 4;                                                              // available digits on display
  boolean isNegative = false;                                                             // true if the number is negative
  
  // is the number negative?
  if(val < 0) 
  {
    isNegative = true;                                                                    // Enable Display Negative Sign[-]
    --numericDigits;                                                                      // Reserve 1 Digit For Display Sign[-]
    val *= -1;                                                                            // Convert Negative Value to Postive Value
  }
  
  // calculate the factor required to shift all fractional digits
  // into the integer part of the number
  double toIntFactor = 1.0;
  for(int i = 0; i < fracDigits; ++i) 
  {
    toIntFactor *= base;
  }
  
  // create integer containing digits to display by applying
  // shifting factor and rounding adjustment
  uint32_t displayNumber = val * toIntFactor + 0.5;
  
  // calculate upper bound on displayNumber given
  // available digits on display
  uint32_t tooBig = 1;
  for(int i = 0; i < numericDigits; ++i) 
  {
    tooBig *= base;
  }
  
  // if displayNumber is too large, try fewer fractional digits
  while(displayNumber >= tooBig) 
  {
    --fracDigits;
    toIntFactor /= base;
    displayNumber = val * toIntFactor + 0.5;
  }
  
  // did toIntFactor shift the decimal off the display?
  if (toIntFactor < 1) 
  {
    printError();
  } 
  else 
  {
    // otherwise, display the number
    //int8_t displayPos = 4;
    int8_t displayPos = 3;
    if (displayNumber)  //if displayNumber is not 0
    {
      for(uint8_t i = 0; displayNumber || i <= fracDigits; ++i) 
      {
        boolean displayDecimal = (fracDigits != 0 && i == fracDigits);
        writeDigitBCD(displayPos--, displayNumber % base, displayDecimal);                // Display Value
        displayNumber /= base;
      }
    }
    else 
    {
      writeDigitBCD(displayPos--, 0, false);                                              // Display : [0]
    }
  
    // display negative sign if negative
    if(isNegative) writeDigitSegment(displayPos--, 0x40);                                 // Display Negative Sign : [-]
  
    // clear remaining display positions
    while(displayPos >= 0) writeDigitSegment(displayPos--, 0x00);                         // Display Space : [ ]
  }
}
//==========================================================================================

//==========================================================================================
// Display [-][-][-][-]
//==========================================================================================
void PCF8575::printError(void) 
{
  for(uint8_t i = 0; i < MAX_7SEGMENT_DIGIT; ++i) 
  {
    writeDigitSegment(i, 0x40);                                                           // [-]:(0100 0000)
  }
}
//==========================================================================================

//
// END OF FILE
//
