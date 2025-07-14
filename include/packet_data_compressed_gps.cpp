void compute_dest_address( uint8_t mic_e_message )
{

  uint8_t char_offset;
  
  // Determine 1st Destination Address byte ( Lat Digit 1 + Message Bit A )

  if( bitRead ( mic_e_message, 2 ) )
    char_offset = 'P';
  
  else
    char_offset = '0';

  dest_address[0] = ( gps_data.lat_DD_10 + char_offset ) << 1;

  // Determine 2nd Destination Address byte ( Lat Digit 2 + Message Bit B )

  if( bitRead ( mic_e_message, 1 ) )
    char_offset = 'P';
  
  else
    char_offset = '0';

  dest_address[1] = ( gps_data.lat_DD_01 + char_offset ) << 1;

  // Determine 3rd Destination Address byte ( Lat Digit 3 + Message Bit C )

  if( bitRead ( mic_e_message, 0 ) )
    char_offset = 'P';
  
  else
    char_offset = '0';

  dest_address[2] = ( gps_data.lat_MM_10 + char_offset ) << 1;

  // Determine 4th Destination Address byte ( Lat Digit 4 + N/S Lat Indicator )

  if( gps_data.NorS == 'N' )
    char_offset = 'P';
  
  else
    char_offset = '0';

  dest_address[3] = ( gps_data.lat_MM_01 + char_offset ) << 1;

  // Determine 5th Destination Address byte ( Lat Digit 5 + Longitude Offset )

 
  if( gps_data.lon_DD > 99 || gps_data.lon_DD < 10 )
    char_offset = 'P';
  
  else
    char_offset = '0';

  dest_address[4] = ( gps_data.lat_hh_10 + char_offset ) << 1;

  // Determine 6th Destination Address byte ( Lat Digit 6 + W/E Lon Indicator )

  if( gps_data.EorW == 'W' )
    char_offset = 'P';
  
  else
    char_offset = '0';

  dest_address[5] = ( gps_data.lat_hh_01 + char_offset ) << 1;

  // Determine APRS Digi Path Code

  dest_address[6] = WIDE2_2 << 1; 

}



void compute_info_longitude()
{

  int char_offset = 0;
  
  // Determine 2nd Information Field byte ( d+28 )

  if ( gps_data.lon_DD < 10 )
    char_offset = 'v';
  
  else if ( gps_data.lon_DD < 100 ) 
    char_offset = '&' - 10;

  else if ( gps_data.lon_DD < 110 )
    char_offset = 'l';

  else
    char_offset = '&' - 110;
  
  info[1] = uint8_t( gps_data.lon_DD + char_offset );

  // Determine 3rd Information Field byte ( m+28 )

  if ( gps_data.lon_MM < 10 )
    char_offset = 'X';
  
  else
    char_offset = '&' - 10;

  info[2] = uint8_t( gps_data.lon_MM + char_offset );

  // Determine 4th Information Field byte ( h+28 )

  

  info[3] = uint8_t( gps_data.lon_hh + 28 );

}  



void compute_info_spd_crs()
{

  uint8_t speed_10 = uint8_t( gps_data.speed / 10 );
  
  if( gps_data.course == 0 )
    gps_data.course = 360;  // Only 360 allowed for Mic-E encoding
  
  // Determine 5th Information Field byte ( SP+28 )

  info[4] = speed_10 + 28;

  // Determine 6th Information Field byte ( DC+28 )

  info[5] = 10 * uint8_t( gps_data.speed % 10 ) +  uint8_t( gps_data.course / 100 ) + 32;  // +32 allows for more ASCII readble characters

  // Determine 7h Information Field byte ( SE+28 )

  info[6] = uint8_t( gps_data.course % 100 ) + 28;
  
} 



void compute_info_alt()
{
  uint16_t alt_abv_datum = 10000 + gps_data.altitude;

  info[ALT_INDX] = uint8_t( alt_abv_datum / 8281 ) + 33;
  
  uint16_t remainder =  uint16_t( alt_abv_datum % 8281 );

  info[ALT_INDX + 1] = uint8_t( remainder / 91 ) + 33;

  info[ALT_INDX + 2] = uint8_t( remainder % 91) + 33;

}



void compute_Mic_E_data( uint8_t mic_e_message )

{

  compute_dest_address( mic_e_message );

  compute_info_longitude();

  compute_info_spd_crs();

  compute_info_alt();

}