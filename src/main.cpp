#include "aprs.h"  // Header file with definitions and global variables, 


void setup() 
{  
  
  gpsSerial.begin( GPS_BAUD_RATE );  // Used for GPS interface
  
  setup_Pins();

  setup_Timers();

  setup_OLED();

#ifdef MCP4725_DAC

  dac.begin(0x62);

#endif

  oled.print( F( "APRS Tracker\r\nV1.0" ) );

  delay( SPLASH_SCRN_DLY );

  oled.clear();

}



void loop() 
{

  static uint16_t beacon_period;

  static uint16_t secs_since_beacon;

  static uint32_t last_TX_time;

  uint8_t mic_e_message;
  
  
  if ( gps_data.fix == false )
    oled.print( F( "Waiting for GPS signal" ) );

  get_GPS_data();  // Get data from GPS unit

  secs_since_beacon = ( unsigned short ) ( ( millis() - last_TX_time ) / 1000 );  // Compute seconds since last packet transmission

  display_data( beacon_period, secs_since_beacon );  // Displays captured GPS data to LCD 


/* ---------------------- Compress data for transmission and send packet  ----------------------- */

  if ( smart_beaconing( beacon_period, secs_since_beacon, mic_e_message ) ) 
  {

#if DEBUG

  Serial.print( "\r\nSince bkn: " );
  Serial.println( secs_since_beacon );
  
  Serial.print( "bkn rate: " );
  Serial.println( beacon_period );
  
#endif

    compute_Mic_E_data( mic_e_message );  // Compress data using Mic-E encoding
    
    send_packet();  // Send APRS data packet 
    
    last_TX_time = millis();  // Update last_TX_time with current time
                                                          
  }

}