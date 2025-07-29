#include"mark_space_gen.h"


/* Timer used for 1200/2200 sine wave generation */

ISR ( WAVE_GEN_TMR_ISR_VECT )
{
  WAVE_PORT =  pgm_read_byte( &SIN_ARRAY[smp_num] );  // Update DAC value

  smp_num++;  // Increment sample number by one
    
  if ( smp_num == WAVE_ARRY_SIZE )  // Handle wrap around
    smp_num = 0;  // Reset sample to zero after one complete SINE array period


}


/* Timer used for 1200 baud timing */

ISR ( BAUD_TMR_ISR_VECT )
{

  baud_tmr_isr = false;  // Sets flag to false to allow baud while loop to exit

}