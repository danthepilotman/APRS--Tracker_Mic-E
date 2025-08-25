#ifndef MENU_CTRL_H
#define MENU_CTRL_H


#include<Arduino.h>
#include<Tiny4kOLED.h>
#include"aprs.h"

// Menu Options

enum Operating_Modes { RUN, SETUP, NUM_OF_OP_MODES };  //Enumerated  Operating modes

enum Display_Pages { POSITION, SATS_INFO, DATE_TIME, NUM_OF_DISP_SCREENS };  // Enumerated display modes (pages)

enum Setup_Options { SYMB_AND_TBL, MIC_MSG, TX_DLY, SEND_ALT, NUM_OF_SETUP_SCREENS  };  // Enumerated display modes (pages)

extern uint8_t operating_mode;  // Store the display mode

extern uint8_t disp_mode;  // Store the display mode

extern uint8_t setup_mode;  // Store the setup mode

/************************ ISR variables  ************************/

extern volatile bool select_pressed;  // Store the display mode

extern volatile bool scroll_pressed;  // Store the display mode


/******************** Beacon Operating Parameters ********************/

extern bool send_alt;

extern uint32_t tx_delay;

/**** Function Prototypes ****/

void scroll_Btn();
void select_Btn();
void check_Buttons();
void handleScroll();
void handleSelect();


#endif