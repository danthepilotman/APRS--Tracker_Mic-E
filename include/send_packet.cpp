void send_tone( bool afsk_tone )
{         
  
  baud_tmr_isr_busy = true;  /* Reset timer 5 interrupt flag */ 
  
  WAVE_GEN_TMR_TCNT  = 0x0000;  /* Initialize Timer4 counter value to 0 */
  
  BAUD_TMR_TCNT  = 0x0000;  /* Initialize Timer5 counter value to 0 */
 
  afsk_tone ? WAVE_GEN_TMR_OCRA = MRK_TMR_CMP: WAVE_GEN_TMR_OCRA = SPC_TMR_CMP;

  while( baud_tmr_isr_busy ); 

}



void sendbyte ( uint8_t inbyte, bool flag_in, uint8_t &stuff_ctr )
{
  
  static bool afsk_tone;
  
  for ( uint8_t i = 0; i < 8; i++ )
  {
    
    uint8_t bt;

    bt = inbyte & 0x01;  // Strip off the rightmost bit
          
    if ( bt == 0 )  // If this bit is a zero,
    {  

      afsk_tone = ! afsk_tone;  //  Flip the output state

      send_tone( afsk_tone ); // Then send the new tone

      stuff_ctr = 0;
       
    }
     
    else
    {
      
      stuff_ctr++;    // Increment 1's count
      
      if ( ( ! flag_in ) && ( stuff_ctr == 5 ) )  // Stuff an extra 0, if five 1's in a row
      {   
        
        send_tone( afsk_tone );  // Send the 1
        
        afsk_tone = ! afsk_tone;  // Flip tone for sending 0
       
        send_tone( afsk_tone );  // Send 0
        
        stuff_ctr = 0;  // Reset stuff counter
                                    
      }   

      else
        send_tone( afsk_tone );

    }
    
    inbyte = inbyte >> 1;  // Shift one to the right to look at the next bit                    
    
  }

}



void send_packet()
{

  unsigned short crc_value;

  uint8_t crc_lo_byte, crc_hi_byte;

  static uint8_t stuff_ctr = 0;  // Reset stuff counter
     

 
  //crc_value = crc16(pkt_data, sizeof(pkt_data),0x1021,0xFFFF,0xFFFF,true,true); 
  crc_value = calc_crc();   // Calculate CRC
  
  crc_lo_byte = crc_value & 0xFF;
  
  crc_hi_byte = crc_value >> 8;
  
  digitalWrite( PTT_PIN, HIGH );  // Key the Transmitter
  
  delay( TX_POWERUP_DLY );  // Wait for Transmitter to power up 
   
  // Enable Timer interrupts
  WAVE_GEN_TMR_TIMSK |= ( 1 << WAVE_GEN_TMR_OCIEA );
  
  BAUD_TMR_TIMSK |= ( 1 << BAUD_TMR_OCIEA );
   
  // Send Start FLAGS
  for ( uint8_t i = 0; i < NUM_START_FLAGS; i++ ) 
    sendbyte( FLAG, true, stuff_ctr );                   
   
  // send Destination Address 
  for( uint8_t i = 0; i < sizeof( dest_address ); i++ )
    sendbyte( dest_address[i], false, stuff_ctr );

  // send Source, Digipeater Addresses / Control, PID Fields 
  for( uint8_t i = 0; i < sizeof( src_digi_addrs_ctl_pid_flds ); i++ )
    sendbyte( src_digi_addrs_ctl_pid_flds[i], false, stuff_ctr );

  // send Information Field
  for( uint8_t i = 0; i < sizeof( info ); i++ )
    sendbyte( info[i], false, stuff_ctr );

  // Send FCS
  sendbyte( crc_lo_byte, false, stuff_ctr );  // Send the low byte of crc
 
  sendbyte( crc_hi_byte, false, stuff_ctr );  // Send the high byte of crc
      
  //Send End FLAGS
  for ( uint8_t i = 0; i < NUM_END_FLAGS; i++ ) 
    sendbyte( FLAG, true, stuff_ctr );                 
  
  // Disable Timer interrupts
  WAVE_GEN_TMR_TIMSK &= ( 0 << WAVE_GEN_TMR_OCIEA );
  
  BAUD_TMR_TIMSK &= ( 0 << BAUD_TMR_OCIEA );

  WAVE_PORT = 0;  // Reset output port to 0s (low)
  //*reinterpret_cast<volatile unsigned char* > ( 0x05 + 0x20 ) = 0;

  digitalWrite( PTT_PIN, LOW );  //unkey PTT     

}



bool smart_beaconing ( uint16_t &beacon_period, uint16_t secs_since_beacon, uint8_t &mic_e_message )
{

  static uint16_t prev_course;  // Retain the previous course for corner pegging comparison
  
  uint16_t delta_course;

  enum mic_E_msg{ emergency, priority, special, commited, returning, in_service, en_route, off_duty };
     
  delta_course = abs( gps_data.course - prev_course );    // Compute course angle change since last packet transmission

  prev_course = gps_data.course;  // Capture course for future comparison


  if( delta_course > 180 )
    delta_course = 360 - delta_course;
 
   
  if ( gps_data.speed < SLOW_SPEED )  // "Stop" threshold
  {

    beacon_period = SLOW_RATE;

    mic_e_message = in_service;  // In service if we are NOT moving

  }

  else  // We're moving; adjust beacon period to speed, and peg corners 
  { 
    
    mic_e_message = en_route;  // En route if we are moving
    
    uint16_t turn_threshold = MIN_TURN_ANGLE + (uint16_t)( (float)TURN_SLOPE / (float)gps_data.speed );  // Adjust turn threshold according to speed

    // Adjust beacon rate according to speed

    if ( gps_data.speed > FAST_SPEED )  
      beacon_period = FAST_RATE;
    
    else
      beacon_period = (uint16_t) round( (float)FAST_RATE * (float)FAST_SPEED / (float)gps_data.speed );

    // Corner pegging

    if ( ( delta_course > turn_threshold ) && ( secs_since_beacon > MIN_TURN_TIME ) )
     return true;     
   
  }
  
  // Beacon period exceeded

  if ( secs_since_beacon > beacon_period )
    return true;
  


  return false;    // Otherwise return false

}