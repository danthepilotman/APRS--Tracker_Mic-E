/* Timer 4 used for 1200/2200 sine wave generation */

ISR ( WAVE_GEN_TMR_ISR_VECT )
{

  WAVE_PORT = SIN_ARRAY[smp_num];
  
  smp_num++;
  
  if ( smp_num == WAVE_ARRY_SIZE )
    smp_num = 0;

}


/* Timer 5 used for 1200 baud timing */

ISR ( BAUD_TMR_ISR_VECT )
{

  baud_tmr_isr_busy = false;

}