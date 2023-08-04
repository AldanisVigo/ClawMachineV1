#ifndef _SHIFT_REGISTER_H_
#define _SHIFT_REGISTER_H_

//Output Shift Registers
#define outputDataPin  5
#define outputLatchPin  6
#define outputClockPin 7


#define BUTTONLEDBIT 
bool buttonledstatus = false;
uint8_t inputRegisterValues = 0;

uint8_t SHIFTREGISTERSTATUSHIGH;
uint8_t SHIFTREGISTERSTATUSLOW;


//Input Shift Registers
#define inputDataPin 3
#define inputClockPin 2
#define inputLatchPin 12
#define inputClockEnablePin 4

#endif
