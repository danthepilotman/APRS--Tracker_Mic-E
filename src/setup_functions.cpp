#include"setup_functions.h"
#include"display_OLED.h"



void setup_Peripherals()
{

  
#if defined( USE_WDT )

  wdt_reset();  // Reset Watch Dog Timer once all desired NMEA sentences have been captured
  
  wdt_enable( WDTO_4S );  // Set watchdog timer for 4 seconds

#endif
  

  ADCSRA &= ~(1 << ADEN); // Clear ADEN bit to 0. Disables ADC for power savings.

  
  ACSR |= (1 << ACD);  // Disable the Analog Comparator by setting the ACD bit in ACSR. Set ACD bit to 1. 
                       // ACSR &= ~(1 << ACD) would enable it if it's already disabled
  
  ACSR &= ~(1 << ACIE);  // Ensure the Analog Comparator Interrupt is also disabled (ACIE bit in ACSR). Clear ACIE bit to 0

}


void setup_Pins()
{

    pinMode( SCROLL_PIN, INPUT_PULLUP );  // Configure display mode pin

    attachInterrupt( digitalPinToInterrupt( SCROLL_PIN ) , scroll_Btn, FALLING );

    pinMode( PTT_PIN, OUTPUT );  // Configure PTT pin

    digitalWrite( PTT_PIN, LOW );  // Set PTT to off be default
  
    pinMode( GPS_VALID_PIN, OUTPUT );  // Configure GPS_VALID_PIN pin
  
    digitalWrite( GPS_VALID_PIN, LOW );  // Set GPS_VALID_PIN to be off by default

    
#if defined( DEBUG )

  // DDRD |= _BV(PD7);  // Set PD7 (digital pin 7) as output

#endif

}


void setup_Timers()
{

    /*------------------ Setup Wave Generator ------------------------ */

    WAVE_PORT_DDR = WAVE_PORT_DDR | R2R_MASK; // Set PORT (digital 0) to output

    WAVE_GEN_TMR_TCCRA = 0;  /* Set entire TCCRxA register to 0 */
    WAVE_GEN_TMR_TCCRB = 0;  /* Same for TCCRxB */
    WAVE_GEN_TMR_TIMSK = 0;  /* Initialize TimerX interrupt register */

    WAVE_GEN_TMR_TCCRA |= _BV(WGM21); /* CTC mode     */
    WAVE_GEN_TMR_TCCRB |= _BV( CS21 );  /*  No prescaler */

    WAVE_GEN_TMR_OCRA = TONE_TIMER_CMP;  /* Set TimerX A compare value */

   /*------------------ Setup Baud Rate Timer ------------------------ */

    BAUD_TMR_TCCRA = 0;  /* Set entire TCCRyA register to 0 */
    BAUD_TMR_TCCRB = 0;  /* Same for TCCRyB */
    BAUD_TMR_TIMSK = 0;  /* Initialize TimerY interrupt register */

    BAUD_TMR_TCCRB |= _BV( BAUD_TMR_WGM2 ) | _BV( BAUD_TMR_CS0 );  /* CTC mode, no prescaler */

    BAUD_TMR_OCRA = BAUD_TIMER_CMP;  /* Set Timer 1 compare value */

}


#if defined( USE_OLED )


void setup_OLED()
{

  oled.begin( width, height, sizeof( tiny4koled_init_128x64br), tiny4koled_init_128x64br );  // Set OLED parameters at start up

  oled.setFont( FONT8X16 );  // Set OLED font

  oled.clear();  // Clear the display

  oled.on();  // Activate the display

}


#endif