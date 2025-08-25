#include"menu_control.h"


uint8_t disp_mode = POSITION;  // Used to store display mode

uint8_t setup_mode = SYMB_AND_TBL;  // Store the setup mode

uint8_t operating_mode = RUN;  // Used to store display mode

volatile bool select_pressed = false;  // Store the display mode

volatile bool scroll_pressed = false;  // Store the display mode

bool send_alt = false;

uint32_t tx_delay = 300;


void scroll_Btn()
{

  uint32_t now = millis();

  static uint32_t lastScrollTime;

  if ( now - lastScrollTime > BTN_DBOUCE_TIME )
  {
    scroll_pressed = true;
    lastScrollTime = now;
  }

}


void select_Btn()
{

  uint32_t now = millis();

  static uint32_t lastSelectTime;

  if ( now - lastSelectTime > BTN_DBOUCE_TIME )
  {
    select_pressed = true;
    lastSelectTime = now;
  }

}


void check_Buttons()
{

  if ( scroll_pressed  )
  {
    scroll_pressed = false;

    handleScroll();
  }

  else if ( select_pressed )
  {
    select_pressed = false;

    handleSelect();
  }

}


void handleScroll()
{

  switch ( operating_mode )
  {

    case RUN:

      disp_mode++;

      if ( disp_mode == NUM_OF_DISP_SCREENS )
        disp_mode = POSITION;

    break;

    case SETUP:

      setup_mode++;

      if( setup_mode == NUM_OF_SETUP_SCREENS )
        setup_mode = SYMB_AND_TBL;

    break;

  }

}


void handleSelect()
{
  switch ( operating_mode )
  {

    case RUN:

      oled.clear();
      operating_mode = SETUP;
      setup_mode = SYMB_AND_TBL;

    break;

    case SETUP:

      switch ( setup_mode )
      {

        case SYMB_AND_TBL:

          oled.clear();
          operating_mode = RUN;

        break;


        case TX_DLY:

          tx_delay +=  100;  // Increase by 100

          if ( tx_delay == 1000 )  // Circular loop from 100 to 10000
            tx_delay = 100;

        break;


        case SEND_ALT:

          send_alt = !send_alt;
        break;

      }

    break;

  }

}