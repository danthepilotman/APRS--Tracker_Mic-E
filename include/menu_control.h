#ifndef MENU_CTRL_H
#define MENU_CTRL_H


#include<Arduino.h>
#include<Tiny4kOLED.h>
#include"aprs.h"

// Menu Options

enum Display_Pages { POSITION, SATS_INFO, DATE_TIME, NUM_OF_DISP_SCREENS };  // Enumerated display modes (pages)

extern uint8_t disp_mode;  // Store the display mode

/************************ ISR variables  ************************/

extern volatile bool scroll_pressed;  // Store the display mode

/**** Function Prototypes ****/

void check_Buttons();
void scroll_Btn();
void handleScroll();


#endif