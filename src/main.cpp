#include<Tiny4kOLED.h>
#include"aprs.h"
#include"display_OLED.h"
#include"setup_functions.h"
#include"gps_string_cap_case.h"
#include"packet_data_compressed_gps.h"
#include"send_packet.h"



void setup() 
{  
  
  setup_Peripherals();  // Disable unneeded peripherals to save power

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


}


void loop() 
{

  static uint16_t beacon_period = SLOW_RATE;  // Time in seconds until next packet transmission

  static uint16_t secs_since_beacon;  // Time in seconds since last packet transmission

  static uint32_t last_TX_time;  // Timestamp in seconds of last packet transmission  

  uint8_t mic_e_message;  // Contents of Mic-e message (En-route, Off Duty, Emergency, etc)
  

#ifdef USE_GPS  

  if ( my_gps.gps_data.fix == false )
    oled.print( F( "Waiting for GPS signal" ) );

  my_gps.get_GPS_Data();  // Get data from GPS unit

#endif


#ifdef DEBUG

    //print_GPS_Data();

#endif


  secs_since_beacon = uint16_t( ( millis() - last_TX_time ) / 1000 );  // Compute seconds since last packet transmission


#ifdef USE_OLED

  display_Data( beacon_period, secs_since_beacon );  // Displays captured GPS data to LCD 

#else

  display_Beacon_Timing( beacon_period, secs_since_beacon );  // Print beaconing timing data

#endif

/* ---------------------- Compress data for transmission and send packet  ----------------------- */


  if ( smart_Beaconing( beacon_period, secs_since_beacon, mic_e_message ) ) 
  {

    compute_Mic_E_Data( mic_e_message );  // Compress data using Mic-E encoding
    
    send_Packet();  // Send APRS data packet 
    
    last_TX_time = millis();  // Update last_TX_time with current time
                                                          
  }

}