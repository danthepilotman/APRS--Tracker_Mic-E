#include"menu_control.h"


uint8_t disp_mode = POSITION;  // Used to store display mode

volatile bool scroll_pressed = false;  // Store the display mode



void check_Buttons()
{

  if ( scroll_pressed  )  // Check scroll button pressed 
  {
    scroll_pressed = false;  // Reset flag

    handleScroll();  // Call helper function
  }

}


void scroll_Btn()  // Scroll button press ISR called fuction
{

  uint32_t now = millis();  // Get current timestamp

  static uint32_t last_Scroll_Time;  // Storage for previous timestamp

  if ( now - last_Scroll_Time > BTN_DBOUCE_TIME )  // Check for debounce time elapsed
  {
    scroll_pressed = true;  // Set flag to true
    last_Scroll_Time = now;  // Store timestamp of latest button press
  }

}


void handleScroll()
{

    disp_mode++;  // Increment display mode

    if ( disp_mode == NUM_OF_DISP_SCREENS )  // Handle wrap around
      disp_mode = POSITION;

}