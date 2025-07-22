#include<Tiny4kOLED.h>
#include"aprs.h"
#include"display_OLED.h"
#include"setup_functions.h"
#include"gps_string_cap_case.h"
#include"packet_data_compressed_gps.h"
#include"send_packet.h"


void setup() 
{  
  
  gpsSerial.begin( GPS_BAUD_RATE );  // Used for GPS interface

  disable_Peripherals();  // Disable unneeded peripherals to save power
  
  setup_Pins();  // Pins for buttons and LEDs

  setup_Timers();  // Timers for baud and DAC transmission rate

  setup_OLED();  // OLED setup

  show_SPLASH_SCRN( SPLASH_SCRN_DLY );  // Show splash screen message for a certain amount of time

}


void loop() 
{

  static uint16_t beacon_period;  // Time in seconds until next packet transmission

  static uint16_t secs_since_beacon;  // Time in seconds since last packet transmission

  static uint32_t last_TX_time;  // Timestamp in seconds of last packet transmission  

  uint8_t mic_e_message;  // Contents of Mic-e message (En-route, Off Duty, Emergency, etc)
  

  if ( gps_data.fix == false )
    oled.print( F( "Waiting for GPS signal" ) );


#if DEBUG == false    
  
  get_GPS_data();  // Get data from GPS unit

#endif


  secs_since_beacon = uint16_t( ( millis() - last_TX_time ) / 1000 );  // Compute seconds since last packet transmission

  display_data( beacon_period, secs_since_beacon );  // Displays captured GPS data to LCD 


/* ---------------------- Compress data for transmission and send packet  ----------------------- */

  if ( smart_Beaconing( beacon_period, secs_since_beacon, mic_e_message ) ) 
  {

#if DEBUG

  Serial.print( "\r\nSince bkn: " );
  Serial.println( secs_since_beacon );
  
  Serial.print( "bkn rate: " );
  Serial.println( beacon_period );
  
#endif

    compute_Mic_E_Data( mic_e_message );  // Compress data using Mic-E encoding
    
    send_Packet();  // Send APRS data packet 
    
    last_TX_time = millis();  // Update last_TX_time with current time
                                                          
  }

}