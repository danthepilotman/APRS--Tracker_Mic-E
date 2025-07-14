#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

#include"aprs.h"
#include<Tiny4kOLED.h>


/******** Function prototypes ***********/

void display_data( uint16_t beacon_period,  uint16_t secs_since_beacon );
void show_SPLASH_SCRN();

#endif