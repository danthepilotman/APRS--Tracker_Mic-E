#include"setup_functions.h"


void setup_Pins()
{

    pinMode( DISP_MODE_PIN, INPUT_PULLUP );  // Configure display mode pin
  
    pinMode( PTT_PIN, OUTPUT );  // Configure PTT pin
  
    pinMode( GPS_VALID_PIN, OUTPUT );  // Configure GPS_VALID_PIN pin
  
    digitalWrite( PTT_PIN, LOW );  // Set PTT to off be default 
  
    digitalWrite( GPS_VALID_PIN, LOW );  // Set GPS_VALID_PIN to be off by default

    attachInterrupt( digitalPinToInterrupt( DISP_MODE_PIN ) , disp_mode_btn, FALLING );

}


void setup_Timers()
{

    /*------------------ Setup Wave Generator ------------------------ */

    WAVE_PORT_DDR = B00001111; // Set PORT (digital 3~0) to outputs
    WAVE_PORT = 8;  // Set wave port outputs to mid range to start
    
    WAVE_GEN_TMR_TCCRA = 0;  /* Set entire TCCRxA register to 0 */
    WAVE_GEN_TMR_TCCRB = 0;  /* Same for TCCRxB */
    WAVE_GEN_TMR_TIMSK = 0;  /* Initialize TimerX interrupt register */   
 
    WAVE_GEN_TMR_TCCRA |= ( 1 << WAVE_GEN_TMR_WGM1 ); /* CTC mode     */ 
    WAVE_GEN_TMR_TCCRB |= ( 1 << WAVE_GEN_TMR_CS0 );  /*  No prescaler */  

    WAVE_GEN_TMR_OCRA = MRK_TMR_CMP;  /* Set TimerX A compare value */     
  
   /*------------------ Setup Baud Rate Timer ------------------------ */

    BAUD_TMR_TCCRA = 0;  /* Set entire TCCRyA register to 0 */
    BAUD_TMR_TCCRB = 0;  /* Same for TCCRyB */
    BAUD_TMR_TIMSK = 0;  /* Initialize TimerY interrupt register */
  
    BAUD_TMR_TCCRB |= ( 1 << BAUD_TMR_WGM2 ) | ( 1 << BAUD_TMR_CS0 );  /* CTC mode, no prescaler */

    BAUD_TMR_OCRA = BAUD_TIMER_CMP;  /* Set Timer 1 compare value */

}



void setup_OLED()
{

  oled.begin( width, height, sizeof( tiny4koled_init_128x64br), tiny4koled_init_128x64br );
  
  oled.setFont( FONT8X16 );
  
  oled.clear();

  oled.on();


#if DEBUG
  display_timers_setup();
#endif 

}


void disp_mode_btn()
{
  
  static uint32_t last_interrupt_time;
  
  uint32_t interrupt_time = millis();
  
  // If interrupts come faster than  BTN_DBOUCE_TIME, assume it's a bounce and ignore
  if ( interrupt_time - last_interrupt_time > BTN_DBOUCE_TIME ) 
  {

    disp_mode++;
  
    if( disp_mode == NUM_OF_DISP_SCREENS )
      disp_mode = 0;

  }

  last_interrupt_time = interrupt_time;

}