#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

#include"aprs.h"
#include<Tiny4kOLED.h>
#include"gps_string_cap_case.h"


/******** Function prototypes ***********/

void display_Data( uint16_t beacon_period,  uint16_t secs_since_beacon );
void show_SPLASH_SCRN( uint32_t splash_screen_delay );
void display_Timers_Setup(); 
void display_Beacon_Timing( uint16_t beacon_period,  uint16_t secs_since_beacon ); 
void print_GPS_Data();

#endif