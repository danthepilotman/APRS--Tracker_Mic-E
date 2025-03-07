void display_data( unsigned short beacon_period, unsigned short secs_since_beacon) 
{  
 
  static byte current_disp_mode;
  
  const char *pos_fix[] = {"Not available", "GPS SPS Mode",
                    "Differential GPS" , "GPS PPS Mode"};
  
  char lcd_upper_row[LCD_COL + 1];

  char lcd_lower_row[LCD_COL + 1];


/* --- Clear LCD display if lcd display mode is changed --- */

  if ( current_disp_mode != lcd_disp_mode )
  {

    lcd.clear();

    current_disp_mode = lcd_disp_mode; 
    
  }

  lcd.home();

/*----- Display relevant data to LCD screen based on page selection ------- */

  switch ( lcd_disp_mode )
  {

    case 0:

      sprintf( lcd_upper_row, " %i\xdf %02i.%02i%02i %c", gps_data.lat_DD, gps_data.lat_MM, gps_data.lat_hh, gps_data.lat_mm, gps_data.NorS );

      lcd.print ( lcd_upper_row );


      lcd.setCursor( 0, BOT_ROW );

      sprintf( lcd_lower_row, "%03d\xdf %02i.%02i%02i %c", gps_data.lon_DD, gps_data.lon_MM, gps_data.lon_hh, gps_data.lon_mm, gps_data.EorW );

      lcd.print ( lcd_lower_row );

    break;  
      

    case 1:

      lcd.setCursor( 0, TOP_ROW );
      lcd.print( F( "Spd Alt Trk Sats" ) );
      
      lcd.setCursor( SPD_FLD_OFFSET, BOT_ROW );
      lcd.print( KTS_to_MPH * gps_data.speed, 0 );
      
      lcd.setCursor( ALT_FLD_OFFSET, BOT_ROW );
      lcd.print( M_to_F * gps_data.altitude, 0 );
      
      lcd.setCursor( TRK_FLD_OFFSET, BOT_ROW );
      lcd.print( gps_data.course );
      lcd.print( F( "\xdf  ") );
      
      lcd.setCursor( SAT_FLD_OFFSET, BOT_ROW ); 
      lcd.print( gps_data.satellites );
        
    break;    


    case 2:

      lcd.print( pos_fix[gps_data.fixquality] );

      
      lcd.setCursor( 0, BOT_ROW );

      sprintf( lcd_lower_row, "Fix type: %dD", gps_data.fixquality_3d );

      lcd.print( lcd_lower_row );

    break;             


    case 3: 
    {
      
      short next_tx = beacon_period - secs_since_beacon;  

      if ( next_tx < 0 )
        next_tx = 0;
      
      byte bkn_min = beacon_period / 60;
      byte bkn_sec = beacon_period % 60;
      byte nxt_min = next_tx / 60;
      byte nxt_sec = next_tx % 60;


      sprintf( lcd_upper_row, "Bkn prd: %02d:%02d", bkn_min, bkn_sec );

      lcd.print ( lcd_upper_row );

    
      lcd.setCursor( 0, BOT_ROW );
   
      sprintf( lcd_lower_row, "Next TX: %02d:%02d", nxt_min, nxt_sec  );

      lcd.print ( lcd_lower_row );

    }

    break;


    case 4:

      lcd.print( F( "HH MM SS") );


      lcd.setCursor( 0, BOT_ROW );

      sprintf( lcd_lower_row, "%0d:%02d:%02d", gps_data.hour, gps_data.minute, gps_data.seconds );

      lcd.print ( lcd_lower_row );

    break;


    case 5:

      lcd.print( F( "MM DD YY") );
      

      lcd.setCursor( 0, BOT_ROW );

      sprintf( lcd_lower_row, "%02d-%02d-%02d", gps_data.month, gps_data.day, gps_data.year );

      lcd.print ( lcd_lower_row );

    break;
    
  } // end switch

/*----------------- Display GPS data on the serial terminal for debugging -----------------  */
#if DEBUG

  print_lcd_debug();
  
#endif  

}



void disp_mode_btn()
{
  
  static unsigned long last_interrupt_time;
  
  unsigned long interrupt_time = millis();
  
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if ( interrupt_time - last_interrupt_time > BTN_DBOUCE_TIME ) 
  {

    lcd_disp_mode++;
  
    if( lcd_disp_mode == NUM_OF_DISP_SCREENS )
      lcd_disp_mode = 0;

  }

  last_interrupt_time = interrupt_time;

}


#if DEBUG

void display_timers_setup() 
{

  Serial.println( F("\r\n-----WAVE_GEN_TMR Settings-----") );
  Serial.print( F("WAVE_ARRY_SIZE: ") );
  Serial.println( WAVE_ARRY_SIZE );
  Serial.print( F("MRK_NUM_SAMP: ") );
  Serial.println( MRK_NUM_SAMP );
  Serial.print( F("SPC_NUM_SAMP: ") );
  Serial.println( SPC_NUM_SAMP );
  Serial.print( F("MRK_TMR_CMP: ") );
  Serial.println( MRK_TMR_CMP );
  Serial.print( F("SPC_TMR_CMP: ") );
  Serial.println( SPC_TMR_CMP );
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



void print_lcd_debug()
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

}

#endif 