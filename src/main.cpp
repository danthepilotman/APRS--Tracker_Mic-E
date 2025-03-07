#include "aprs.h"  // Header file with definitions and global variables, 


void setup() 
{  
  
  gpsSerial.begin( GPS_BAUD_RATE );  // used for GPS interface
  
  setup_Pins();

  setup_Timers();

  setup_LCD();

}  // End setup()



void loop() 
{

  static unsigned short beacon_period;

  static unsigned short secs_since_beacon;

  static unsigned long last_TX_time;

  byte mic_e_message;
  
  
  if ( gps_data.fix == false )
    lcd.print( F( "Waiting for GPS signal" ) );

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
  
  Serial.print( "corner: " );
  Serial.println( corner_Peg );

#endif

    compute_Mic_E_data( mic_e_message );  // compress data using Mic-E encoding
    
    send_packet();  // send APRS data packet 
    
    last_TX_time = millis();  // update last_TX_time with current time
                                                          
  }

}  // end loop()