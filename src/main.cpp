#include<Tiny4kOLED.h>
#include"aprs.h"
#include"display_OLED.h"
#include"setup_functions.h"
#include"gps_string_cap_case.h"
#include"packet_data_compressed_gps.h"
#include"send_packet.h"



void setup()
{

  my_gps.gpsSerial.begin( 9600 );  // Start serial UART and set baud rate

  setup_Pins();  // Pins for buttons and LEDs

  setup_Timers();  // Timers for baud and DAC transmission rate

  
#ifdef USE_OLED

  setup_OLED();  // OLED setup

  show_SPLASH_SCRN( SPLASH_SCRN_DLY );  // Show splash screen message for a certain amount of time

#endif

#ifdef DEBUG

  display_Timers_Setup();  // Show baud and DAC timer settings

#endif

setup_Peripherals();  // Disable unneeded peripherals to save power

}


void loop()
{

  check_Buttons();  // Check if buttons have been pressed

  mic_E_Beacon();  // Send MIC-E beacon packet

}