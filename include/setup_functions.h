#ifndef SETUP_FUNCTIONS_H
#define SETUP_FUNCTIONS_H


#include"display_OLED.h"



#ifdef MCP4725_DAC

Adafruit_MCP4725 dac;

#endif


/******** Function prototypes ***********/


void setup_Pins();
void setup_Timers();
void setup_OLED();
void disp_mode_btn();

#endif