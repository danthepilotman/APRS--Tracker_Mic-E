#include"gps_string_cap_case.h"


gps_NMEA my_gps( Serial, 9600 );  // Instantiate gps object


void gps_NMEA::get_GPS_Data()
{

  char NMEA_data[NMEA_DATA_MAX_SIZE];  // Character array to store NMEA sentences

  bool GGA_good = false;  // Boolean flags to determine if sentence has been successfully received and parsed
  bool GSA_good =  false;
  bool RMC_good = false;


#ifndef DEBUG

  memset( &gps_data, 0, sizeof( gps_data ) );  // Clear stale data from gps structure

#endif

  while ( GGA_good == false || GSA_good == false || RMC_good == false )  // Keep looping until all desired sentences have been successfully processed
  {
    get_NMEA_Sentence( NMEA_data );  // Capture NMEA sentence from serial port

    if ( strncmp( NMEA_data, "GPGGA", 5 ) == 0 && GGA_good == false)  // Check for desired NMEA sentence
    {
      GGA_good = parse_GGA( NMEA_data );  // Parse sentence and store parsing result
      continue;  // Move on to next NMEA sentence capture
    }

    if ( strncmp( NMEA_data, "GPGSA", 5 ) == 0 && GSA_good == false )  // Check for desired NMEA sentence
    {
      GSA_good = parse_GSA( NMEA_data );  // Parse sentence and store parsing result
      continue;  // Move on to next NMEA sentence capture
    }

    if ( strncmp( NMEA_data, "GPRMC", 5 ) == 0 && RMC_good == false )  // Check for desired NMEA sentence
    {
      RMC_good = parse_RMC( NMEA_data );  // Parse sentence and store parsing result

#ifdef USE_OLED

      if ( gps_data.fix == false )  // Check valid GPS fix flag 
      {
        
        oled.setCursor( 0, 0 );  // Set cursor to home position
        
        for ( uint8_t i = 0; i < NMEA_DATA_MAX_SIZE && NMEA_data[i]; i++ )  // Loop through all characters of the capture NMEA sentence
        {
          oled.print ( NMEA_data[i] );  // Print out each character of the NMEA sentence
        }
        
          oled.clearToEOP();  // Clear to the End of Page
      }

#endif

    }

#ifdef USE_WDT

    wdt_reset();  // Reset Watch Dog Timer once all desired NMEA sentences have been captured

#endif

  }
}


void gps_NMEA::get_NMEA_Sentence( char* NMEA_data )
{
  
  digitalWrite( GPS_VALID_PIN, HIGH );
  
  while ( true )
  {
    
    if ( my_gps.gpsSerial.read() != '$' )
      continue;

    NMEA_data[my_gps.gpsSerial.readBytesUntil( '\r', NMEA_data, NMEA_DATA_MAX_SIZE - 1 )] = '\0';

    digitalWrite( GPS_VALID_PIN, LOW );

#ifdef DEBUG

    my_gps.gpsSerial.print(F("NMEA_data: "));
    my_gps.gpsSerial.println(NMEA_data);

#endif

    if ( gps_NMEA::xsum_Check( NMEA_data ) )
    {

#ifdef DEBUG

      my_gps.gpsSerial.println(F("Good checksum"));

#endif

      break;
    }
  }
}


bool gps_NMEA::parse_GGA( const char* NMEA_data )
{
  
  const char* ptr = next_field( NMEA_data );  // Point to NMEA_data array 
  
  ptr = next_field( ptr );  // Skip the 1st comma
  ptr = next_field( ptr );  // Skip the 2nd comma
  ptr = next_field( ptr );  // Skip the 3rd comma
  ptr = next_field( ptr );  // Skip the 4th comma
  ptr = next_field( ptr );  // Skip the 5th comma

  if ( is_Empty(ptr) == false )  // Check for errors
  {
    gps_data.fixquality = atoi( ptr );  // Capture the fix quality parameter
    gps_data.fix = gps_data.fixquality > 0;
  }

  if ( gps_data.fix == false )  // Check if we have a valid fix
   return false;  // Return failure if fix is not valid

  ptr = next_field( ptr );  // Skip the 6th comma

  if ( is_Empty(ptr) == false )  // Check for errors
    gps_data.satellites = atoi( ptr );  // Capture the number of satellites parameter

  ptr = next_field( ptr );  // Skip the 7th comma
  ptr = next_field( ptr );  // Skip the 8th comma

  if ( is_Empty(ptr) == false )  // Check for errors
  gps_data.altitude = int16_t( round( atof( ptr ) ) );  // Capture the altitude parameter

  return true;  // If you made it this far parsing was successful
}


bool gps_NMEA::parse_RMC( const char* NMEA_data )
{

  const char* ptr = next_field( NMEA_data );  // Point to NMEA_data array and skip the 1st comma

  parse_Time( ptr );  // Get the time info

  ptr = next_field( ptr );   // Skip the 2nd comma

  parse_Fix( ptr );  // Capture fix paramater

  if ( gps_data.fix == false )
   return false;  // Go back and capture another NMEA sentence 

  ptr = next_field( ptr );  // Skip the 3rd comma 
  parse_Coord( ptr );   // Latitude

  ptr = next_field( ptr );   // Skip the 4th comma 
  gps_data.NorS = *ptr;  // Capture hemisphere paramater

  ptr = next_field( ptr) ;   // Skip the 5th comma 
  parse_Coord( ptr );   // Longitude

  ptr = next_field( ptr );   // Skip the 6th comma 
  gps_data.EorW = *ptr;  // Capture hemisphere paramater

  ptr = next_field( ptr );  // Skip the 7th comma 

  if ( is_Empty( ptr ) == false )   // Check for errors
    my_gps.gps_data.speed = (unsigned int)( round( atof( ptr ) ) );  // Capture speed paramater

  ptr = next_field( ptr );  // Skip the 8th comma 
  
  if ( is_Empty( ptr ) == false )   // Check for errors
    gps_data.course = (unsigned int)( round( atof( ptr ) ) );

  ptr = next_field( ptr );   // Skip the 9th comma 
  
  if ( is_Empty( ptr ) == false )   // Check for errors
  {
    uint32_t fulldate = atol( ptr );  // Date format is  ddmmyy
    my_gps.gps_data.day = fulldate / 10000;  // Capture day paramater
    my_gps.gps_data.month = ( fulldate % 10000 ) / 100;  // Capture month paramater
    my_gps.gps_data.year = fulldate % 100;   // Capture year paramater
  }

  return true;   // If you made it this far, all is good

}


bool gps_NMEA::parse_GSA( const char* NMEA_data )
{

  const char* ptr = next_field( NMEA_data );  // Point it to the 2nd field

  ptr = next_field( ptr );  // Move to the 3rd field

  if (is_Empty (ptr ) == false )  // Check for errors
  {
    gps_data.fixquality_3d = atoi( ptr );
    return true;
  }

  return false;  // If empty return false
}


char* gps_NMEA::next_field( const char* ptr )
{

  char* next = strchr( ptr, ',' );   // Pointer to the location of the next comma

  return ( next ) ? ( next + 1 ) : NULL;  // If the pointer is not NULL, return the location right after the comma, otherwise return NULL

}


bool gps_NMEA::is_Empty( const char* pStart )
{

  return ( pStart == nullptr || *pStart == ',' || *pStart == '*' );  // Check that it is not empty, nor another comma, nor the '*' for the checksum field

}


bool gps_NMEA::parse_Time( const char* ptr )
{

  if ( is_Empty( ptr ) == false )  // Check for empty field 
  {
    unsigned long time = atol( ptr );   // Convert string to 32-bit integer

    gps_data.hour = time / 10000;  // Hour is located 4 digits from the right (10^4 = 10000)

    gps_data.minute = ( time % 10000 ) / 100;  // Seconds is the first two digits (10^2 = 100) on the last 4 digits on the right (10^4=10000)

    gps_data.seconds = ( time % 100 );  // Seconds is the last two digits on the right (10^2)

    return true;   // Return true after computing time
  }
  
  return false;  // If field is empty report failure

}


bool gps_NMEA::parse_Fix( const char* ptr )
{

  if ( is_Empty( ptr ) == false )  // Check for empty field
  {

    if ( ptr[0] == 'A' )
      gps_data.fix = true;   // 'A' represents a good fix

    else if ( ptr[0] == 'V' )
      gps_data.fix = false;  // 'V' represents lack of a valid fix

    else return false;  // Return false if character is not an 'A' or a 'V'

    return true;  // If you made it this far down everything checked out okay

  }
  
  return false;  // Return false if field is empty

}


bool gps_NMEA::parse_Coord( const char* coord )
{

  if ( coord == nullptr || *coord == '\0' )  // Check for errors
    return false;

  const char* dp = strchr( coord, '.' );  // Locate decimal point

  if ( dp == nullptr  )  // Check for errors
    return false;

  ptrdiff_t int_len = dp - coord;  // Compute distance between first digit character and the decimal point


  if ( int_len != 4 && int_len != 5 )  // Check for errors. Should only be 4 or 5 digits. eg. 12345.xxxx or 8912.xxxx
    return false;

  char digits[9] = {0};  // Create array for storing digits and initialize to zero

  if ( int_len == 5 ) // If longitude
    memcpy( digits, dp - 5, 5 );  // Copy whole degrees and minutes digits from NMEA_data to digits array

  else  // If latitude
    memcpy( digits + 1, dp - 4, 4 );  // Copy whole degrees and minutes digits from NMEA_data to digits array
  
  memcpy( digits + 5, dp + 1, 4 );  // Copy fractional minutes to digits array

  for ( int i = ( int_len == 5 ? 0 : 1 ); i < 9; ++i )  // Loop through the digits array
  {
    if ( isdigit( digits[i] ) == false )  // Check for errors
      return false;

    digits[i] -= '0';  // Convert from char to integer
  }

  if ( int_len == 5 )  // Longitude
  {

    my_gps.gps_data.lon_DD_100 = digits[0];  // Capture lat/lon 100's digit


    my_gps.gps_data.lon_DD_10  = digits[1];  // Capture lat/lon 10's digit


    my_gps.gps_data.lon_DD_01  = digits[2];  // Capture lat/lon 1's digit


    my_gps.gps_data.lon_MM_10  = digits[3];  // Capture lat/lon 10's digit


    my_gps.gps_data.lon_MM_01  = digits[4];  // Capture lat/lon 1's digit


    my_gps.gps_data.lon_hh_10  = digits[5];  // Capture lat/lon 10's digit


    my_gps.gps_data.lon_hh_01  = digits[6];  // Capture lat/lon 1's digit


    my_gps.gps_data.lon_mm_10  = digits[7];  // Capture lat/lon 10's digit


    my_gps.gps_data.lon_mm_01  = digits[8];  // Capture lat/lon 1's digit



    my_gps.gps_data.lon_DD = 100 * my_gps.gps_data.lon_DD_100 + 10 * my_gps.gps_data.lon_DD_10 + my_gps.gps_data.lon_DD_01;  // Compute integer value from 100's 10's and 1's components
    my_gps.gps_data.lon_MM = 10 * my_gps.gps_data.lon_MM_10 + my_gps.gps_data.lon_MM_01;  // Compute integer value from tens and ones components
    my_gps.gps_data.lon_hh =  10 * my_gps.gps_data.lon_hh_10 + my_gps.gps_data.lon_hh_01;  // Compute integer value from tens and ones components

  }

  else  // Latitude
  {
    my_gps.gps_data.lat_DD_10 = digits[1];  // Capture lat/lon 10's digit


    my_gps.gps_data.lat_DD_01 = digits[2];  // Capture lat/lon 1's digit


    my_gps.gps_data.lat_MM_10 = digits[3];  // Capture lat/lon 10's digit


    my_gps.gps_data.lat_MM_01 = digits[4];  // Capture lat/lon 1's digit


    my_gps.gps_data.lat_hh_10 = digits[5];  // Capture lat/lon 10's digit


    my_gps.gps_data.lat_hh_01 = digits[6];  // Capture lat/lon 1's digit


    my_gps.gps_data.lat_mm_10 = digits[7];  // Capture lat/lon 10's digit


    my_gps.gps_data.lat_mm_01 = digits[8];  // Capture lat/lon 1's digit


  }

  return true;  // Retrun success if you made it this far

}


uint8_t gps_NMEA::hex_Str_To_Int( const char* chk_sum_in )
{

  uint8_t delta, integer = 0;  // Initialize variables to 0


  if (  chk_sum_in[1] >= 'A' && chk_sum_in[1] <= 'F' )  // Check for alpha hexadecimal characters
   delta = 'A' - 0xA;  // Compute offset

  else
    delta = '0';  // Offset for numeric characters


  integer = ( chk_sum_in[1] - delta ) << 4;  // Convert first digit to numerical value


  if ( chk_sum_in[2] >= 'A' && chk_sum_in[2] <= 'F' ) // Check for alpha hexadecimal characters
   delta = 'A' - 0xA;  // Compute offset

  else
    delta = '0';   // Offset for numeric characters


  integer += ( chk_sum_in[2] - delta );   // Convert seconds digit to numerical value and add to first digit's value


  return integer;  // Return integer value

}



bool gps_NMEA::xsum_Check( const char* NMEA_data )
{

  uint8_t computed_check_sum = 0;  // Reset to zero

  char* asterisk_ptr = strchr( NMEA_data, '*' );  // Locate the asterisk

  if ( asterisk_ptr == nullptr || !*( asterisk_ptr + 1 ) || !*( asterisk_ptr + 2 ) )  // Check for errors
    return false;


  uint8_t received_check_sum = hex_Str_To_Int( asterisk_ptr );  // Convert the ASCII string to an integer


  for ( const char* p = NMEA_data; p < asterisk_ptr; p++ )  // Compute the checksum between the start of the NMEA sentence and the '*'
    computed_check_sum ^= *p;

  return computed_check_sum == received_check_sum; // Return the result of the comparison between the received and computed checksum

}