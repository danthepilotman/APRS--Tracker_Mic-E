#include"send_packet.h"


void send_Tone( bool afsk_tone )
{         
  
  volatile uint8_t smp_num = 0;  // Store and remember SINE array sample number
  
  baud_tmr_isr = true;  /* Reset baud timer interrupt busy flag */ 

  wave_gen_tmr_isr = false;  /* Reset wave generator timer interrupt flag */ 
  
  WAVE_GEN_TMR_TCNT = 0x0000;  // Initialize wave generator counter value to 0
  
  BAUD_TMR_TCNT = 0x0000;  // Initialize baud timer counter value to 0
 
  afsk_tone ? WAVE_GEN_TMR_OCRA = MRK_TMR_CMP: WAVE_GEN_TMR_OCRA = SPC_TMR_CMP;  // Set timer compare value based on SPACE or MARK

  while( baud_tmr_isr )  // Wait until the BAUD timer ISR is triggered to move on to the next tone
  {

    if( wave_gen_tmr_isr )  // If DAC timer ISR is triggered
    {
      
      wave_gen_tmr_isr = false;  // Clear DAC timer ISR flag
      
      WAVE_PORT =  pgm_read_byte( &SIN_ARRAY[smp_num] );  // Update DAC value

      smp_num++;  // Increment sample number by one
    
      if ( smp_num == WAVE_ARRY_SIZE )  // Handle wrap around
        smp_num = 0;  // Reset sample to zero after one complete SINE array period

    }

  } 

}



void send_Byte ( uint8_t inbyte )
{
  
  static bool afsk_tone;

  static uint8_t stuff_ctr = 0;  // Reset stuff counter
  
  for ( uint8_t i = 0; i < 8; i++ )
  {
    
    if ( bitRead( inbyte, i ) == SPACE )  // If this bit is a zero,
    {  

      afsk_tone = ! afsk_tone;  //  Flip the output state

      send_Tone( afsk_tone ); // Then send the new tone

      stuff_ctr = 0;
       
    }
     
    else
    {
      
      stuff_ctr++;    // Increment sequential 1's count
      
      if ( ( inbyte != FLAG ) && ( stuff_ctr == 5 ) )  // Stuff an extra 0, if five 1's in a row
      {   
        
        send_Tone( afsk_tone );  // Send the 1
        
        afsk_tone = ! afsk_tone;  // Flip tone for sending 0
       
        send_Tone( afsk_tone );  // Send 0
        
        stuff_ctr = 0;  // Reset stuff counter
                                    
      }   

      else
        send_Tone( afsk_tone );

    }
    
  }

}



void send_Packet()
{

  uint16_t crc_value = calc_CRC();   // Calculate CRC

  uint8_t crc_lo_byte = crc_value & 0xFF;  // Strip off low byte portion
  
  uint8_t crc_hi_byte = crc_value >> 8;  // Shift down high byte portion

 
  digitalWrite( PTT_PIN, HIGH );  // Key the Transmitter
  
  delay( TX_POWERUP_DLY );  // Wait for Transmitter to power up
   
  // Enable Timer interrupts
  WAVE_GEN_TMR_TIMSK |= ( 1 << WAVE_GEN_TMR_OCIEA );
  
  BAUD_TMR_TIMSK |= ( 1 << BAUD_TMR_OCIEA );
   
  // Send Start FLAGS
  for ( uint8_t i = 0; i < NUM_START_FLAGS; i++ ) 
    send_Byte( FLAG );                   
   
  // send Destination Address 
  for( uint8_t i = 0; i < sizeof( dest_address ); i++ )
    send_Byte( dest_address[i] );

  // send Source, Digipeater Addresses / Control, PID Fields 
  for( uint8_t i = 0; i < SRC_DIGI_ADDRS_CTL_PID_FLDS_LEN; i++ )
    send_Byte( src_digi_addrs_ctl_pid_flds[i] );

  // send Information Field
  for( uint8_t i = 0; i < INFO_LEN; i++ )
    send_Byte( info[i] );

  // Send FCS
  send_Byte( crc_lo_byte );  // Send the low byte of crc
 
  send_Byte( crc_hi_byte );  // Send the high byte of crc
      
  //Send End FLAGS
  for ( uint8_t i = 0; i < NUM_END_FLAGS; i++ ) 
    send_Byte( FLAG );                 
  
  // Disable Timer interrupts
  WAVE_GEN_TMR_TIMSK &= ~( 1 << WAVE_GEN_TMR_OCIEA );
  
  BAUD_TMR_TIMSK &= ~( 1 << BAUD_TMR_OCIEA );

  WAVE_PORT = 0;  // Reset output port to 0s (low)
  //*reinterpret_cast<volatile unsigned char* > ( 0x05 + 0x20 ) = 0;

  digitalWrite( PTT_PIN, LOW );  //unkey PTT     

}



bool smart_Beaconing ( uint16_t &beacon_period, uint16_t secs_since_beacon, uint8_t &mic_e_message )
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
    
    uint16_t turn_threshold = MIN_TURN_ANGLE + uint16_t( float( TURN_SLOPE ) / float( gps_data.speed ) );  // Adjust turn threshold according to speed

    // Adjust beacon rate according to speed

    if ( gps_data.speed > FAST_SPEED )  
      beacon_period = FAST_RATE;
    
    else
      beacon_period = uint16_t( round( float( FAST_RATE ) * float( FAST_SPEED ) / float( gps_data.speed ) ) );

    // Corner pegging

    if ( ( delta_course > turn_threshold ) && ( secs_since_beacon > MIN_TURN_TIME ) )
     return true;     
   
  }
  
  // Beacon period exceeded

  if ( secs_since_beacon > beacon_period )
    return true;
  


  return false;    // Otherwise return false

}