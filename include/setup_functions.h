#ifndef SETUP_FUNCTIONS_H
#define SETUP_FUNCTIONS_H


#include"display_OLED.h"
#include <avr/wdt.h>


/******** Function prototypes ***********/

void setup_Peripherals();
void setup_Pins();
void setup_Timers();
void setup_OLED();
void disp_Mode_Btn();

#endif