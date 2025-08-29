#include"display_OLED.h"


#if defined( USE_OLED )


void display_Data( uint16_t beacon_period,  uint16_t secs_since_beacon )
{

  char oled_row[ OLED_COLS + 1 ];  // Used to create character array for display on OLED

  static uint8_t current_disp_mode;
  
  if ( disp_mode != current_disp_mode )
  {
  
    oled.clear();  // Erase all content on the display

    current_disp_mode = disp_mode;

  }


/*----- Display relevant data to OLED screen based on page selection ------- */

  switch ( disp_mode )
  {

    case POSITION:

      oled.setCursor( 0, FIRST_ROW ); // Latitude
      sprintf( oled_row, " %u%u %u%u.%u%u%u%u %c",
      my_gps.gps_data.lat_DD_10, my_gps.gps_data.lat_DD_01,
      my_gps.gps_data.lat_MM_10, my_gps.gps_data.lat_MM_01,
      my_gps.gps_data.lat_mm_1, my_gps.gps_data.lat_mm_01,
      my_gps.gps_data.lat_mm_001, my_gps.gps_data.lat_mm_0001,
      my_gps.gps_data.NorS );

      oled.print ( oled_row );
      //oled.bitmap( 16, FIRST_ROW, 8 + 16, FIRST_ROW + 2, degree_symbol_bitmap );
      oled.clearToEOL();


      oled.setCursor( 0, SECOND_ROW ); // Longitude
      sprintf( oled_row, "%03u %02u.%02u%u%u %c",
      my_gps.gps_data.lon_DD,
      my_gps.gps_data.lon_MM,
      my_gps.gps_data.lon_mm,
      my_gps.gps_data.lon_mm_001, my_gps.gps_data.lon_mm_0001,
      my_gps.gps_data.EorW );
      
      oled.print( oled_row );
      //oled.bitmap( 24, SECOND_ROW, 8 + 24, SECOND_ROW + 2, degree_symbol_bitmap );
      oled.clearToEOL();


      oled.setCursor( 0, THIRD_ROW );  // Course
      sprintf( oled_row, "%03u  ", my_gps.gps_data.course );
      oled.print( oled_row );
      oled.print( KTS_to_MPH * float( my_gps.gps_data.speed ), 0 );
      oled.print( " mph");
      oled.clearToEOL();

      
      oled.setCursor( 0, FOURTH_ROW );  // Altitude
      oled.print( M_to_FT * float( my_gps.gps_data.altitude ), 0 );
      oled.print( " ft" );
      oled.clearToEOL();

    break;
      

    case SATS_INFO:

      
      oled.setCursor( 0, FIRST_ROW );  // Fix quality
      oled.print( my_gps.pos_fix[my_gps.gps_data.fixquality] );
      oled.clearToEOL();


      oled.setCursor( 0, SECOND_ROW ); // Fix type
      sprintf( oled_row, "Fix type: %uD", my_gps.gps_data.fixquality_3d );
      oled.print( oled_row );
      oled.clearToEOL();
      

      oled.setCursor( 0, THIRD_ROW ); // Numer of satellites being tracked
      sprintf( oled_row, "Sats: %u", my_gps.gps_data.satellites );
      oled.print( oled_row );
      oled.clearToEOL();

      oled.clearToEOP();
        
    break;


    case DATE_TIME:
    
      
      int16_t next_tx = beacon_period - secs_since_beacon;

      if ( next_tx < 0 )
        next_tx = 0;
      
      uint8_t bkn_min = beacon_period / 60;
      uint8_t bkn_sec = beacon_period % 60;

      uint8_t nxt_min = next_tx / 60;
      uint8_t nxt_sec = next_tx % 60;

      oled.setCursor( 0, FIRST_ROW );  // Beacon period
      sprintf( oled_row, "Bkn prd: %02u:%02u", bkn_min, bkn_sec );
      oled.print ( oled_row );
      oled.clearToEOL();

    
      oled.setCursor( 0, SECOND_ROW ); // Time until next transmission
      sprintf( oled_row, "Next TX: %02u:%02u", nxt_min, nxt_sec  );
      oled.print ( oled_row );
      oled.clearToEOL();


      oled.setCursor( 0, THIRD_ROW );  // Time
      sprintf( oled_row, "Time: %0u:%02u:%02u", my_gps.gps_data.hour, my_gps.gps_data.minute, my_gps.gps_data.seconds );
      oled.print ( oled_row );
      oled.clearToEOL();


      oled.setCursor( 0, FOURTH_ROW );  // Date
      sprintf( oled_row, "Date: %02u-%02u-%02u", my_gps.gps_data.month, my_gps.gps_data.day, my_gps.gps_data.year );
      oled.print ( oled_row );
      oled.clearToEOL();

    

    break;

  } // end switch

}


void show_SPLASH_SCRN( uint32_t SPLASH_SCREEN_DELAY )
{
  oled.print( F( "APRS Tracker\r\nV1.0" ) );

  delay( SPLASH_SCREEN_DELAY );

  oled.clear();
}


#endif


#if defined( DEBUG )


void display_Beacon_Timing( uint16_t beacon_period,  uint16_t secs_since_beacon )
{
  int16_t next_tx = beacon_period - secs_since_beacon; // Compute time until next beacon

  static int16_t prev_sec = 0;  // Remember previous timestamp form when beacon data was printed

  if ( next_tx < 0 )  // Handle possible negative values
    next_tx = 0;

  uint8_t nxt_sec = next_tx % 60; // Compute seconds until next beacon.

  if ( nxt_sec != prev_sec )  // Check if one seconds has passed since last beacon timing printing
  {

    my_gps.gpsSerial.print( F("Next Tx:" ) );  // Print label

    my_gps.gpsSerial.println( nxt_sec );  // Print number of seconds until next beacon

    prev_sec = nxt_sec;  // Update previous printing timestamp

  }

}


void display_Timers_Setup()
{

  my_gps.gpsSerial.println( F("\r\n-----WAVE_GEN_TMR Settings-----") );
  my_gps.gpsSerial.print( F("WAVE_ARRY_SIZE: ") );
  my_gps.gpsSerial.println( WAVE_ARRY_SIZE );
  my_gps.gpsSerial.print( F("WAVE_GEN_TIMER_CMP: ") );
  my_gps.gpsSerial.println(TONE_TIMER_CMP );
  my_gps.gpsSerial.print( F("WAVE_GEN_TMR_TCCRA = ") );
  my_gps.gpsSerial.println( WAVE_GEN_TMR_TCCRA, BIN );
  my_gps.gpsSerial.print( F("WAVE_GEN_TMR_TCCRB = ") );
  my_gps.gpsSerial.println( WAVE_GEN_TMR_TCCRB, BIN );
  my_gps.gpsSerial.print( F("WAVE_GEN_TMR_OCRA = ") );
  my_gps.gpsSerial.println( WAVE_GEN_TMR_OCRA, BIN );
  my_gps.gpsSerial.print( F("WAVE_GEN_TMR_TIMSK = ") );
  my_gps.gpsSerial.println( WAVE_GEN_TMR_TIMSK, BIN );


  my_gps.gpsSerial.println( F("\r\n-----BAUD_TMR Settings-----") );
  my_gps.gpsSerial.print( F("BAUD_TIMER_CMP: ") );
  my_gps.gpsSerial.println( BAUD_TIMER_CMP );
  my_gps.gpsSerial.print( F("BAUD_TMR_TCCRA = ") );
  my_gps.gpsSerial.println( BAUD_TMR_TCCRA, BIN );
  my_gps.gpsSerial.print( F("BAUD_TMR_TCCRB = ") );
  my_gps.gpsSerial.println( BAUD_TMR_TCCRB, BIN );
  my_gps.gpsSerial.print( F("BAUD_TMR_OCRA = ") );
  my_gps.gpsSerial.println( BAUD_TMR_OCRA, BIN );
  my_gps.gpsSerial.print( F("BAUD_TMR_TIMSK = ") );
  my_gps.gpsSerial.println( BAUD_TMR_TIMSK, BIN );
  my_gps.gpsSerial.println( "" );

}  // End display_timers_setup()


void print_GPS_Data()
{

  char gps_str[32];  // Character array to store formatted data for printing

  static uint32_t prev_timestamp = 0;  // Store timestamp when previous printing occurred

  uint32_t current_timestamp = millis();  // Get the current timestamp


  if ( current_timestamp - prev_timestamp > 1000 )  // Check if enough time has elapsed in order to print again
  {

    prev_timestamp = current_timestamp;  // Remember when printing started the last time

    sprintf( gps_str,"%02d:%02d:%02d UTC %d/%02d/%02d",
    my_gps.gps_data.hour, my_gps.gps_data.minute, my_gps.gps_data.seconds,
    my_gps.gps_data.month, my_gps.gps_data.day, my_gps.gps_data.year );

    my_gps.gpsSerial.println( F( "\r\n---------GPS Data--------" ) );
    my_gps.gpsSerial.print( F( "Time: " ) );
    my_gps.gpsSerial.println( gps_str );

    my_gps.gpsSerial.print( F( "Pos: " ) );
    sprintf( gps_str,"%c %c%c\xC2\xB0 %c%c.%c%c%c%c %c %c%c%c\xC2\xB0 %c%c.%c%c%c%c",
    my_gps.gps_data.NorS, 
    my_gps.gps_data.lat_DD_10, my_gps.gps_data.lat_DD_01,
    my_gps.gps_data.lat_MM_10, my_gps.gps_data.lat_MM_01,
    my_gps.gps_data.lat_mm_1, my_gps.gps_data.lat_mm_01,
    my_gps.gps_data.lat_mm_001, my_gps.gps_data.lat_mm_0001,
    my_gps.gps_data.EorW,
    my_gps.gps_data.lon_DD_100, my_gps.gps_data.lon_DD_10, my_gps.gps_data.lon_DD_01,
    my_gps.gps_data.lon_MM_10, my_gps.gps_data.lon_MM_01,
    my_gps.gps_data.lon_mm_1, my_gps.gps_data.lon_mm_01,
    my_gps.gps_data.lon_mm_001, my_gps.gps_data.lon_mm_0001 );

    my_gps.gpsSerial.println( gps_str );
  
    my_gps.gpsSerial.print( F ("Speed: ") );
    my_gps.gpsSerial.print( KTS_to_MPH *my_gps.gps_data.speed, 0 );
    my_gps.gpsSerial.println( F( " mph" ) );

    my_gps.gpsSerial.print( F( "Course: " ) );
    my_gps.gpsSerial.print( my_gps.gps_data.course );
    my_gps.gpsSerial.println( F( "\xC2\xB0" ) );

    my_gps.gpsSerial.print( F( "Altitude: " ) );
    my_gps.gpsSerial.print( M_to_FT *my_gps.gps_data.altitude, 0 );
    my_gps.gpsSerial.println( F( " ft" ) );

    my_gps.gpsSerial.print( F( "Sats in use: " ) );
    my_gps.gpsSerial.println( my_gps.gps_data.satellites );

    my_gps.gpsSerial.print( F( "Pos Fix Indicator: " ) );
    my_gps.gpsSerial.println( my_gps.pos_fix[my_gps.gps_data.fixquality] );

    my_gps.gpsSerial.print( F( "Fix type [none/2D/3D]: " ) );
    my_gps.gpsSerial.println( my_gps.gps_data.fixquality_3d );

  }

}


#endif