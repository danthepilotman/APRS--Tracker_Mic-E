#include"send_packet.h"


void send_Tone( bool afsk_tone )
{

  baud_tmr_isr_running = true;  /* Reset baud timer interrupt busy flag */ 

  afsk_tone ? current_phase_step = MRK_PHASE_STEP: current_phase_step = SPC_PHASE_STEP;  // Set phase step value based on SPACE or MARK

  while( baud_tmr_isr_running )  // Wait until the BAUD timer ISR is triggered to move on to the next tone
  {
    // Wave generator timer ISR free runs during this while loop, generating sampled sine DAC values
  }

}


void send_Byte ( uint8_t inbyte )
{

  static bool afsk_tone;  // Remember previous tone used

  static uint8_t stuff_ctr = 0;  // Reset stuff counter
  
  for ( uint8_t i = 0; i < 8; i++ )  // Loop through all 8 bits of each byte
  {
    
    if ( bitRead( inbyte, i ) == SPACE )  // If this bit is a zero (SPACE)
    {

      afsk_tone = !afsk_tone;  //  Flip the tone for a SPACE

      send_Tone( afsk_tone ); // Then send the new tone

      stuff_ctr = 0;  // Reset stuff counter

    }

    else  // If the bit is a 1 (MARK)
    {

      stuff_ctr++;    // Increment sequential 1's count

      if ( ( inbyte != FLAG ) && ( stuff_ctr == 5 ) )  // Stuff an extra 0, if five 1's in a row
      {

        send_Tone( afsk_tone );  // Send the previous tone for a MARK

        afsk_tone = ! afsk_tone;  // Flip tone for a SPACE

        send_Tone( afsk_tone );  // Send the tone for a SPACE

        stuff_ctr = 0;  // Reset stuff counter

      }

      else
        send_Tone( afsk_tone );  // Send previous tone for a MARK

    }

  }

}


void send_Packet()
{

  uint16_t crc_value = calc_CRC();   // Calculate CRC

  uint8_t crc_lo_byte = crc_value & 0x00FF;  // Strip off low byte portion
  
  uint8_t crc_hi_byte = crc_value >> 8;  // Shift down high byte portion

  digitalWrite( PTT_PIN, HIGH );  // Key the Transmitter
  
  delay( TX_POWERUP_DLY );  // Wait for Transmitter to power up

  // Enable Timer interrupts
  WAVE_GEN_TMR_TIMSK |= _BV( WAVE_GEN_TMR_OCIEA );

  BAUD_TMR_TIMSK |= _BV( BAUD_TMR_OCIEA );

  // Send Start FLAGS
  for ( uint8_t i = 0; i < NUM_START_FLAGS; i++ )
    send_Byte( FLAG );

  // send Destination Address
  for( uint8_t i = 0; i < DEST_ADDR_SIZE; i++ )
    send_Byte( dest_address[i] );

  // send Source, Digipeater Addresses / Control, PID Fields 
  for( uint8_t i = 0; i < SRC_DIGI_ADDRS_CTRL_PID_FLDS_LEN; i++ )
    send_Byte( src_digi_addrs_ctrl_pid_flds[i] );

  // send Information Field
  for( uint8_t i = 0; i < INFO_LEN; i++ )
    send_Byte( info[i] );

  // Send FCS
  send_Byte( crc_lo_byte );  // Send the low byte of crc

  send_Byte( crc_hi_byte );  // Send the high byte of crc

  //Send End FLAGS
  for ( uint8_t i = 0; i < NUM_END_FLAGS; i++ )
    send_Byte( FLAG );

  digitalWrite( PTT_PIN, LOW );  //unkey PTT
  
    // Disable Timer interrupts
  WAVE_GEN_TMR_TIMSK &= ~( 1 << WAVE_GEN_TMR_OCIEA );

  BAUD_TMR_TIMSK &= ~( 1 << BAUD_TMR_OCIEA );

  WAVE_PORT = 8;  // Reset output port to 0s (low)

}


bool smart_Beaconing ( uint16_t &beacon_period, uint16_t secs_since_beacon, uint8_t &mic_e_message )
{

  static uint16_t prev_course = my_gps.gps_data.course;  // Retain the previous course for corner pegging comparison

  uint16_t delta_course = abs( my_gps.gps_data.course - prev_course );    // Compute course angle change since last packet transmission


  if( delta_course > 180 )  // Handle course changes greater than 180°
    delta_course = 360 - delta_course;  // Compute "short way round" angular change

  if ( my_gps.gps_data.speed < SLOW_SPEED )  // Determine if our speed is below the "slow" threshold
  {

    beacon_period = SLOW_RATE;  // Set beacon period to slow rate if we're moving slow or stopped

    mic_e_message = in_service;  // In service if we are NOT moving

  }

  else  // We're moving; adjust beacon period based on speed, and peg corners 
  {

    mic_e_message = en_route;  // En route if we are moving

    uint16_t turn_threshold = MIN_TURN_ANGLE + uint16_t( float( TURN_SLOPE ) / float( my_gps.gps_data.speed ) );  // Adjust turn threshold according to speed

    /************************************ Adjust beacon rate according to speed ************************************/

    if ( my_gps.gps_data.speed > FAST_SPEED )   // Determine if our speed is below the "fast" threshold
      beacon_period = FAST_RATE;  // Limit beacon period to avoid overly frequent transmissions

    else
      beacon_period = uint16_t( round( float( FAST_RATE ) * float( FAST_SPEED ) / float( my_gps.gps_data.speed ) ) );  // Compute beacon period based on speed

    /******************************** Corner pegging ********************************/

    if ( ( delta_course > turn_threshold ) && ( secs_since_beacon > MIN_TURN_TIME ) )
      return true;

  }

  /******* Beacon period exceeded *******/

  if ( secs_since_beacon > beacon_period )
    return true;

  return false;  // If you made it this far, you haven't turned a corner and the beacon period has not expired

}


void mic_E_Beacon()
{

  static uint16_t beacon_period;  // Time in seconds until next packet transmission

  static uint16_t secs_since_beacon;  // Time in seconds since last packet transmission

  static uint32_t last_TX_time;  // Timestamp in seconds of last packet transmission

  uint8_t mic_e_message;  // Contents of Mic-e message (En-route, Off Duty, Emergency, etc)


#ifdef USE_GPS

  if ( my_gps.gps_data.fix == false )
    oled.print( F( "Waiting for GPS signal" ) );

  my_gps.get_GPS_Data();  // Get data from GPS unit

#endif


#ifdef DEBUG

  print_GPS_Data();

#endif

  secs_since_beacon = uint16_t( ( millis() - last_TX_time ) / 1000 );  // Compute seconds since last packet transmission

#ifdef USE_OLED

  display_Data( beacon_period, secs_since_beacon );  // Displays captured GPS data to LCD

#else

  display_Beacon_Timing( beacon_period, secs_since_beacon );  // Print beaconing timing data

#endif

/* ---------------------- Compress data for transmission and send packet  ----------------------- */


  if ( smart_Beaconing( beacon_period, secs_since_beacon, mic_e_message ) )
  {

    compute_Mic_E_Data( mic_e_message );  // Compress data using Mic-E encoding

    send_Packet();  // Send APRS data packet

    last_TX_time = millis();  // Update last_TX_time with current time

  }

}