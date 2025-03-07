void setup_Pins()
{

    pinMode( DISP_MODE_PIN, INPUT_PULLUP );  // configure display mode pin
  
    pinMode( PTT_PIN, OUTPUT );  // configure PTT pin
  
    pinMode( GPS_VALID_PIN, OUTPUT );  // configure GPS_VALID_PIN pin
  
    digitalWrite( PTT_PIN, LOW );  // Set PTT to off be default 
  
    digitalWrite( GPS_VALID_PIN, LOW );  // Set GPS_VALID_PIN to be off by default

    attachInterrupt( digitalPinToInterrupt( DISP_MODE_PIN ) , disp_mode_btn, FALLING );

}


void setup_Timers()
{

    /*------------------ Setup Wave Generator ------------------------ */

    WAVE_PORT_DDR = B00001111; // set PORT (digital 3~0) to outputs
    WAVE_PORT = 8;  // Set wave port outputs to mid range to start
    
    WAVE_GEN_TMR_TCCRA = 0;  /* set entire TCCRxA register to 0 */
    WAVE_GEN_TMR_TCCRB = 0;  /* same for TCCRxB */
    WAVE_GEN_TMR_TIMSK = 0;  /* initialize TimerX interupt register */   
 
    WAVE_GEN_TMR_TCCRA |= ( 1<<WAVE_GEN_TMR_WGM1 ); /* CTC mode     */ 
    WAVE_GEN_TMR_TCCRB |= ( 1<<WAVE_GEN_TMR_CS0 ); /*  no prescaler */  

    WAVE_GEN_TMR_OCRA = MRK_TMR_CMP;  /* set TimerX A compare value */     
  
   /*------------------ Setup Baud Rate Timer ------------------------ */

    BAUD_TMR_TCCRA = 0;  /* set entire TCCRyA register to 0 */
    BAUD_TMR_TCCRB = 0;  /* same for TCCRyB */
    BAUD_TMR_TIMSK = 0;  /* initialize TimerY interupt register */
  
    BAUD_TMR_TCCRB |= ( 1<<BAUD_TMR_WGM2 ) | ( 1<<BAUD_TMR_CS0 ); /* CTC mode, no prescaler */

    BAUD_TMR_OCRA = BAUD_TIMER_CMP;  /* Set Timer 1 compare value */

}



void setup_LCD()
{

    lcd.init();

    lcd.backlight();

    /* ----- Print Splash Screen to LCD ------ */

    lcd.print( F( "APRS courseer" ) );
  
    lcd.setCursor( 0, BOT_ROW );
  
    lcd.print( F( "v1.1 DLDesigns" ) );
  
    delay( SPLASH_SCRN_DLY );
  
    lcd.clear();

#if DEBUG
  display_timers_setup();
#endif 

}