#include"packet_data_compressed_gps.h"


void compute_Dest_Address( uint8_t mic_e_message )
{

  uint8_t char_offset;  // Store offset value used for MIC-E encoding
  
  // Determine 1st Destination Address byte ( Lat Digit 1 + Message Bit A )

  if( bitRead ( mic_e_message, 2 ) )
    char_offset = 'P';
  
  else
    char_offset = '0';

  dest_address[LAT_DIG_1] = ( my_gps.gps_data.lat_DD_10 + char_offset ) << 1;

  // Determine 2nd Destination Address byte ( Lat Digit 2 + Message Bit B )

  if( bitRead ( mic_e_message, 1 ) )
    char_offset = 'P';
  
  else
    char_offset = '0';

  dest_address[LAT_DIG_2] = ( my_gps.gps_data.lat_DD_01 + char_offset ) << 1;

  // Determine 3rd Destination Address byte ( Lat Digit 3 + Message Bit C )

  if( bitRead ( mic_e_message, 0 ) )
    char_offset = 'P';

  else
    char_offset = '0';

  dest_address[LAT_DIG_3] = ( my_gps.gps_data.lat_MM_10 + char_offset ) << 1;

  // Determine 4th Destination Address byte ( Lat Digit 4 + N/S Lat Indicator )

  if( my_gps.gps_data.NorS == 'N' )
    char_offset = 'P';

  else
    char_offset = '0';

  dest_address[LAT_DIG_4] = ( my_gps.gps_data.lat_MM_01 + char_offset ) << 1;

  // Determine 5th Destination Address byte ( Lat Digit 5 + Longitude Offset )


  if( my_gps.gps_data.lon_DD > 99 || my_gps.gps_data.lon_DD < 10 )
    char_offset = 'P';

  else
    char_offset = '0';

  dest_address[LAT_DIG_5] = ( my_gps.gps_data.lat_mm_1 + char_offset ) << 1;

  // Determine 6th Destination Address byte ( Lat Digit 6 + W/E Lon Indicator )

  if( my_gps.gps_data.EorW == 'W' )
    char_offset = 'P';

  else
    char_offset = '0';

  dest_address[LAT_DIG_6] = ( my_gps.gps_data.lat_mm_01 + char_offset ) << 1;

  // Determine APRS Digi Path Code

  dest_address[DIGI_PATH] =  Digi_Path << 1;

}


void compute_Info_Longitude()
{

  int char_offset = 0;  // Store offset value used for MIC-E encoding. Note that offset can be a negative value.

  // Determine 2nd Information Field byte ( d+28 )

  if ( my_gps.gps_data.lon_DD < 10 )
    char_offset = 'v';

  else if ( my_gps.gps_data.lon_DD < 100 )
    char_offset = '&' - 10;

  else if ( my_gps.gps_data.lon_DD < 110 )
    char_offset = 'l';

  else
    char_offset = '&' - 110;

  info[d_28] = uint8_t( my_gps.gps_data.lon_DD + char_offset );

  // Determine 3rd Information Field byte ( m+28 )

  if (my_gps.gps_data.lon_MM < 10 )
    char_offset = 'X';

  else
    char_offset = '&' - 10;

  info[m_28] = uint8_t( my_gps.gps_data.lon_MM + char_offset );

  // Determine 4th Information Field byte ( h+28 )

  info[h_28] = uint8_t( my_gps.gps_data.lon_mm + 28 );

}


void compute_Info_Spd_Crs()
{

  if( my_gps.gps_data.course == 0 )
    my_gps.gps_data.course = 360;  // Only 360 allowed for Mic-E encoding

  // Determine 5th Information Field byte ( SP+28 )

  info[SP_28] = uint8_t( my_gps.gps_data.speed / 10 ) + 28;  // Speed's 10's digit

  // Determine 6th Information Field byte ( DC+28 )

  info[DC_28] = 10 * uint8_t( my_gps.gps_data.speed % 10 ) +  uint8_t( my_gps.gps_data.course / 100 ) + 32;  // +32 allows for more ASCII readble characters

  // Determine 7h Information Field byte ( SE+28 )

  info[SE_28] = uint8_t( my_gps.gps_data.course % 100 ) + 28;

}


void compute_Info_Alt()
{
  uint16_t alt_abv_datum = 10000 + my_gps.gps_data.altitude;  // Adjust altitude for datum reference

  info[ALT_INDX] = uint8_t( alt_abv_datum / 8281 ) + 33;  // Encode first altitude value

  uint16_t remainder =  uint16_t( alt_abv_datum % 8281 );  // Altitude encoding computation for Mic-E

  info[ALT_INDX + 1] = uint8_t( remainder / 91 ) + 33;  // Encode second altitude value

  info[ALT_INDX + 2] = uint8_t( remainder % 91) + 33;  // Encode third altitude value

}


void compute_Mic_E_Data( uint8_t mic_e_message )
{

  compute_Dest_Address( mic_e_message );  // Encode destination address

  compute_Info_Longitude();  // Encode longitude

  compute_Info_Spd_Crs();  // Encode speed and course

#if defined( SEND_ALTITUDE )

  compute_Info_Alt();  // Encode altitude

#endif

}