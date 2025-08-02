#include"mark_space_gen.h"


/* Timer used for 1200/2200 sine wave generation */

ISR ( WAVE_GEN_TMR_ISR_VECT )
{
 
#ifdef DEBUG

  //PORTD |= _BV(PD7);  // Set PD7 HIGH to mark ISR entry

  //PIND |= _BV(PD7);
  
#endif

  phase_accumulator += current_phase_step;  // Increment the phase by the proper amount each cycle

  uint8_t sample_index = uint8_t( phase_accumulator >> ( 32 - 6 ) );  // 32-bit integer shifted down to a 6-bit number for the 64 = log(2,6) element array index

  uint8_t sample = pgm_read_byte( &SIN_ARRAY[sample_index] );   // Get current sine sample:

  WAVE_PORT =  sample;  // Drive output port value

#ifdef DEBUG

  //PORTD &= ~_BV(PD7); // Set PD7 LOW to mark ISR exit

#endif

}


/* Timer used for 1200 baud timing */

ISR ( BAUD_TMR_ISR_VECT )
{

  baud_tmr_isr_running = false;  // Sets flag to false to allow baud while loop to exit

}