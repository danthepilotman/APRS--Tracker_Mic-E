#include"display_OLED.h"


#ifdef DEBUG

void display_Beacon_Timing( uint16_t beacon_period,  uint16_t secs_since_beacon ) 
{
  int16_t next_tx = beacon_period - secs_since_beacon;  

  static int16_t prev_sec = 0;

  if ( next_tx < 0 )
    next_tx = 0;
      
  uint8_t nxt_sec = next_tx % 60;

  if ( nxt_sec != prev_sec )
  {
    Serial.print( F("Next Tx:" ) );
    Serial.println( nxt_sec );
    prev_sec = nxt_sec;
  }

}

#else

void display_Data( uint16_t beacon_period,  uint16_t secs_since_beacon ) 
{  
 
  enum{ POSITION, SATS_INFO, DATE_TIME };



  static uint8_t current_disp_mode;

  

  
  const char *pos_fix[] = { "Not available", "GPS SPS Mode",
                    "Differential GPS" , "GPS PPS Mode" };
  
  char oled_row[ OLED_COLS + 1 ];  // Used to create character string for display on OLED


/* --- Clear OLED display if OLED display mode is changed --- */

  if ( current_disp_mode != disp_mode )
  {

    oled.clear();

    current_disp_mode = disp_mode; 
    
  }




 disp_mode =  DATE_TIME;
 




/*----- Display relevant data to OLED screen based on page selection ------- */

  switch ( disp_mode )
  {



    case POSITION:

      oled.setCursor( 0, FIRST_ROW ); // Latitude
      sprintf( oled_row, "%02u %02u.%02u%02u %c", gps_data.lat_DD, gps_data.lat_MM, gps_data.lat_hh, gps_data.lat_mm, gps_data.NorS );
      oled.print ( oled_row );
      //oled.bitmap( 16, FIRST_ROW, 8 + 16, FIRST_ROW + 2, degree_symbol_bitmap );
      oled.clearToEOL();


      oled.setCursor( 0, SECOND_ROW ); // Longitude
      sprintf( oled_row, "%03u %02u.%02u%02u %c", gps_data.lon_DD, gps_data.lon_MM, gps_data.lon_hh, gps_data.lon_mm, gps_data.EorW );
      oled.print( oled_row );
      //oled.bitmap( 24, SECOND_ROW, 8 + 24, SECOND_ROW + 2, degree_symbol_bitmap );
      oled.clearToEOL();


      oled.setCursor( 0, THIRD_ROW );  // Course
      sprintf( oled_row, "%03u  ", gps_data.course );
      oled.print( oled_row );
      oled.print( KTS_to_MPH * float( gps_data.speed ), 0 );
      oled.print( " mph");
      oled.clearToEOL();

      
      oled.setCursor( 0, FOURTH_ROW );  // Altitude
      oled.print( M_to_F * float( gps_data.altitude ), 0 );
      oled.print( " ft" );
      oled.clearToEOL();

    break;  
      

    case SATS_INFO:

      
      oled.setCursor( 0, FIRST_ROW );  // Fix quality
      oled.print( pos_fix[gps_data.fixquality] );
      oled.clearToEOL();


      oled.setCursor( 0, SECOND_ROW ); // Fix type
      sprintf( oled_row, "Fix type: %uD", gps_data.fixquality_3d );
      oled.print( oled_row );
      oled.clearToEOL();
      
      oled.setCursor( 0, THIRD_ROW ); // Numer of satellites being tracked
      sprintf( oled_row, "Sats: %u", gps_data.satellites );
      oled.print( oled_row );
      oled.clearToEOL();
        
    break;    



    case DATE_TIME: 
    {
      
      short next_tx = beacon_period - secs_since_beacon;  

      if ( next_tx < 0 )
        next_tx = 0;
      

      uint8_t bkn_min = beacon_period / 60;
      uint8_t bkn_sec = beacon_period % 60;


      uint8_t nxt_sec = next_tx % 60;


    
      Serial.print( F(" Next Tx:" ) );
      Serial.println( nxt_sec );
      delay(500);




      oled.setCursor( 0, FIRST_ROW );  // Beacon period
      sprintf( oled_row, "Bkn prd: %02u:%02u", bkn_min, bkn_sec );
      oled.print ( oled_row );
      oled.clearToEOL();

    
      oled.setCursor( 0, SECOND_ROW ); // Time until next transmission
      sprintf( oled_row, "Next TX: %02u:%02u", nxt_min, nxt_sec  );
      oled.print ( oled_row );
      oled.clearToEOL();


      oled.setCursor( 0, THIRD_ROW );  // Time
      sprintf( oled_row, "Time: %0u:%02u:%02u", gps_data.hour, gps_data.minute, gps_data.seconds );
      oled.print ( oled_row );
      oled.clearToEOL();


      oled.setCursor( 0, FOURTH_ROW );  // Date
      sprintf( oled_row, "Date: %02u-%02u-%02u", gps_data.month, gps_data.day, gps_data.year );
      oled.print ( oled_row );
      oled.clearToEOL();


    }

    break;

  } // end switch

/*----------------- Display GPS data on the serial terminal for debugging -----------------  */
#ifdef DEBUG

  //print_oled_debug();
  
#endif  

}


#endif



#ifndef DEBUG

void show_SPLASH_SCRN( uint32_t splash_screen_delay )
{
  oled.print( F( "APRS Tracker\r\nV1.0" ) );

  delay(  splash_screen_delay );

  oled.clear();
}

#endif



#ifdef DEBUG


void display_Timers_Setup() 
{

  Serial.println( F("\r\n-----WAVE_GEN_TMR Settings-----") );
  Serial.print( F("WAVE_ARRY_SIZE: ") );
  Serial.println( WAVE_ARRY_SIZE );
  Serial.print( F("WAVE_GEN_TIMER_CMP: ") );
  Serial.println(TONE_TIMER_CMP );
  Serial.print( F("WAVE_GEN_TMR_TCCRA = ") );
  Serial.println( WAVE_GEN_TMR_TCCRA, BIN );
  Serial.print( F("WAVE_GEN_TMR_TCCRB = ") );
  Serial.println( WAVE_GEN_TMR_TCCRB, BIN );
  Serial.print( F("WAVE_GEN_TMR_OCRA = ") );
  Serial.println( WAVE_GEN_TMR_OCRA, BIN );
  Serial.print( F("WAVE_GEN_TMR_TIMSK = ") );
  Serial.println( WAVE_GEN_TMR_TIMSK, BIN );
  

  Serial.println( F("\r\n-----BAUD_TMR Settings-----") );
  Serial.print( F("BAUD_TIMER_CMP: ") );
  Serial.println( BAUD_TIMER_CMP );
  Serial.print( F("BAUD_TMR_TCCRA = ") );
  Serial.println( BAUD_TMR_TCCRA, BIN );
  Serial.print( F("BAUD_TMR_TCCRB = ") );
  Serial.println( BAUD_TMR_TCCRB, BIN );
  Serial.print( F("BAUD_TMR_OCRA = ") );
  Serial.println( BAUD_TMR_OCRA, BIN );
  Serial.print( F("BAUD_TMR_TIMSK = ") );
  Serial.println( BAUD_TMR_TIMSK, BIN );
  Serial.println( "" );
   
}  // End display_timers_setup()



/* void print_oled_debug()
{

  char gps_time[50];

  sprintf( gps_time,"%02d:%02d:%02d UTC %d/%02d/%02d", hour, minute, seconds, month, day, year ) ;          

  Serial.println( F("\r\n---------GPS Data--------") );
  Serial.print( F("Time: ") );
  Serial.println( gps_time );
 
  Serial.print( F("Pos: ") );
  Serial.print( lat_degreebuff );
  Serial.print( F(" ") );
  Serial.print ( NorS );
  Serial.print( F(", ") );
  Serial.print( lon_degreebuff );
  Serial.print( F(" ") );
  Serial.println ( EorW );

  Serial.print(F ("Speed: ") );
  Serial.print( speed, 0 );
  Serial.println( F(" mph" ) );

  Serial.print( F("course: ") );
  Serial.print(course, 0);
  Serial.println( F("°" ) );

  Serial.print( F("Altitude: ") );
  Serial.print( altitude, 0 );
  Serial.println( F(" ft" ) );

  Serial.print( F("Sats in use: ") );
  Serial.println( satellites );

  Serial.print( F("Pos Fix Indicator: ") );
  Serial.println( pos_fix[fixquality] );

  Serial.print( F("Fix type [none/2D/3D]: ") );
  Serial.println( fixquality_3d );
  
  Serial.println( F("\r\n") );

} */

#endif 