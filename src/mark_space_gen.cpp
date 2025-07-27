#include"mark_space_gen.h"


/* Timer used for 1200/2200 sine wave generation */

ISR ( WAVE_GEN_TMR_ISR_VECT )
{

  wave_gen_tmr_isr = true;  // Set flag to true to allow for processing of next DAC sample

}


/* Timer used for 1200 baud timing */

ISR ( BAUD_TMR_ISR_VECT )
{

  baud_tmr_isr = false;  // Sets flag to false to allow baud while loop to exit

}