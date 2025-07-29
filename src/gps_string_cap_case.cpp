#include"gps_string_cap_case.h"


void get_GPS_Data() 
{

  char NMEA_data[NMEA_DATA_MAX_SIZE];  // Char array to store captured NMEA sentences

  bool GGA_not_cap = true;

  bool GSA_not_cap = true;

  bool RMC_not_cap = true;


#ifndef DEBUG

  memset( &gps_data, 0, sizeof( gps_data ) );  // Clear stale data

#endif


  while ( GGA_not_cap || GSA_not_cap || RMC_not_cap )
  {

    get_NMEA_Sentence ( NMEA_data );

    // Check that proper NMEA sentences have been captured

    if ( strncmp( NMEA_data, "$GPGGA", 6 ) == 0  &&  GGA_not_cap )
    {

      GGA_not_cap = parse_GGA( NMEA_data );
      
      continue;
    
    }

      
    if ( strncmp( NMEA_data, "$GPGSA", 6 ) == 0  &&  GSA_not_cap )
    {

      GSA_not_cap = parse_GSA( NMEA_data );

      continue;

    }

    if ( strncmp( NMEA_data, "$GPRMC", 6 ) == 0  &&  RMC_not_cap )
    {
      RMC_not_cap = parse_RMC( NMEA_data );

      if( gps_data.fix == false )  // Check if valid GPS data has been received
      {
        oled.setCursor( 0, 0 );  // Move to home position

        for( uint8_t i = 0; i < NMEA_DATA_MAX_SIZE && NMEA_data[i]; i++ )  // Loop through the NMEA sentence string
          oled.print( NMEA_data[i] );  // Print each NMEA sentence character

        oled.clearToEOP();  // Erase old data
      }
    }

    wdt_reset();  // Reset WDT

  }

} 



void get_NMEA_Sentence( char* NMEA_data )
{

  while (true) 
  {

    digitalWrite( GPS_VALID_PIN, HIGH );  // Blink LED to indicate activity
    
    while ( gpsSerial.available() == 0 );  // Wait for serial data to be available
      
    NMEA_data[0] = gpsSerial.read();  // Read one character from Serial RX buffer

    if ( NMEA_data[0] != '$' )  // Loop back if not at the begining of a NMEA sentence  
      continue;  // Go back to the begining of the sentence capture sequence
      
    for ( uint8_t i = 1; i < NMEA_DATA_MAX_SIZE; i++ )  
    {
        
      while ( gpsSerial.available() == 0 );  // Wait for serial data to be available
        
      NMEA_data[i] = gpsSerial.read();  // Read one character from Serial RX buffer

      if ( NMEA_data[i]  == '\n' )  // If at the end of a NMEA sentence
      {    
        
        NMEA_data[i] = '\0';  // Terminate character array will null character
        
        break;  // Break out of the for-loop once we get to the end of the NMEA sentence                 
          
      }
    }

    digitalWrite( GPS_VALID_PIN, LOW );  // Blink LED to indicate activity

#ifdef DEBUG
      Serial.print( "NMEA_data: " );
      Serial.println( NMEA_data );
#endif   

      
    // Get another NMEA sentence if Check Sums don't match

    if ( xsum_Check( NMEA_data) )   // Negated logic on checksum_check 
    {

#ifdef DEBUG
        Serial.println("Failed Xsum");
#endif 
        // continue;  // Returns to the begining of the while loop 
      break;  // Go back to the begining of the sentence capture sequence

    }   

  }

}



//------------------Start of process for getting GPGGA sentence-----------------------
// strcpy(NMEA_data,"$GPGGA,161229.487,3723.2475,N,12158.3416,W,1,07,1.0,9.0,M,,,,0000*18");

bool parse_GGA( const char* NMEA_data )
{

  const char* ptr;
  
  ptr = NMEA_data;
  
  ptr = next_field(ptr); // Skip over first ','
  
  ptr = next_field(ptr); // Skip over time
      
  ptr = next_field(ptr);  // Skip lat
  ptr = next_field(ptr);  // Skip N/S
        
  ptr = next_field(ptr);  // Skip lon
  ptr = next_field(ptr);  // Skip E/W
    
  if ( is_Empty( ptr ) == false ) 
  { 
               
    gps_data.fixquality = atoi( ptr );  // Needs additional processing
      
    if ( gps_data.fixquality > 0 ) 
      gps_data.fix = true;       
    
    else
      gps_data.fix = false;
                     
  }

  if( gps_data.fix == false )
    return true;  // Go back and capture another NMEA sentence
                       
  ptr = next_field(ptr); // Then move on to the next data field
  
  // Most can just be parsed with atoi() or atof(), then move on to the next.
  
  if ( is_Empty( ptr ) == false )
    gps_data.satellites = atoi( ptr );
    
  ptr = next_field(ptr);  // Move to the next field
   
  ptr = next_field(ptr);  // Move to the next field

  if ( is_Empty( ptr ) == false )
    gps_data.altitude = int16_t( round( atof( ptr ) ) );
    
  // Skip the rest

  return false;

}


//------------------Start of process for getting GPRMC sentence-----------------------
//  strcpy( NMEA_data, "$GPRMC,161229.487,A,3723.2475,N,12158.3416,W,0.13,309.62,120598,,*10" );

bool parse_RMC( const char* NMEA_data )
{

  const char* ptr;

  ptr = NMEA_data;

  ptr = next_field(ptr);  // Skip the 1st comma

  parse_Time( ptr );              // Get the time info
    
  ptr = next_field(ptr);  // Skip the 2nd comma
    
  parse_Fix( ptr ); // Status

  if( gps_data.fix == false )
    return true;  // Go back and capture another NMEA sentence 
    
  ptr = next_field(ptr);  // Skip the 3rd comma 

  parse_Coord( ptr ); // Latitude
 
  ptr = next_field(ptr);  // Skip the 4th comma 

  gps_data.NorS = *ptr; // Field is not empty

  ptr = next_field(ptr);  // Skip the 5th comma 

  parse_Coord( ptr );

  ptr = next_field(ptr);  // Skip the 6th comma 

  gps_data.EorW = *ptr; // Field is not empty

  ptr = next_field(ptr);  // Skip the 7th comma 

  if ( is_Empty ( ptr ) == false )
    gps_data.speed = ( unsigned int ) ( round( atof ( ptr ) ) );
    
  ptr = next_field(ptr);  // Skip the 8th comma 

  if ( is_Empty ( ptr ) == false )
    gps_data.course = ( unsigned int ) ( round( atof ( ptr ) ) );
    
  ptr = next_field(ptr);  // Skip the 9th comma 

  if ( is_Empty( ptr ) == false )
  {

    unsigned long fulldate = atol( ptr );  // ddmmyy
    
    gps_data.day = fulldate / 10000;
    
    gps_data.month = ( fulldate % 10000 ) / 100;
    
    gps_data.year = ( fulldate % 100 );
    
  }

  return false;

}


//--------------Start of process for getting GPGSA sentence--------------------
// strcpy( NMEA_data,"$GPGSA,A,3,01,20,19,13,,,,,,,,,40.4,24.4,32.2*0A" );

bool parse_GSA( const char* NMEA_data )
{

  const char* ptr;

  ptr = NMEA_data;

  ptr = next_field(ptr);  // Move to the next field

  ptr = next_field(ptr);  // Move to the next field

  if ( is_Empty( ptr ) == false )
    gps_data.fixquality_3d = atoi( ptr );

  return false;

}



/**************************************************************************/
/*!
    @brief Finds the starting location of the next data field in the NMEA string 
    in the NMEA string
    @param ptr Pointer to the location of the token in the NMEA string
    @return Pointer to the location of the start of the next data field 
 */
/**************************************************************************/
char* next_field( const char* ptr )
{
  
  char* next = strchr( ptr, ',' );
  
  return ( next != NULL ) ? ( next + 1 ) : NULL;

}



/**************************************************************************/
/*!
    @brief Parse a part of an NMEA string for time. Independent of number
    of decimal places after the '.'
    @param ptr Pointer to the location of the token in the NMEA string
    @return true if successful, false otherwise
*/
/**************************************************************************/
bool is_Empty( const char* pStart ) 
{
  
  return ( pStart == NULL || *pStart == ',' || *pStart == '*' );

}


bool parse_Time( const char* ptr ) 
{
  
  if ( is_Empty( ptr ) == false) 
  { 
    
    unsigned long time = atol( ptr );
    
    gps_data.hour = time / 10000;
    
    gps_data.minute = ( time % 10000 ) / 100;
    
    gps_data.seconds = ( time % 100 );
    
    return true; 

  }
  
  return false;

}


/**************************************************************************/
/*!
    @brief Parse a part of an NMEA string for whether there is a fix
    @param ptr Pointer to the location of the token in the NMEA string
    @return True if we parsed it, false if it has invalid data
*/
/**************************************************************************/
bool parse_Fix( const char* ptr ) 
{
  
  if ( is_Empty( ptr ) == false )
  {
    
    if ( ptr[0] == 'A' ) 
      gps_data.fix = true;    
    
    else if ( ptr[0] == 'V' )
      gps_data.fix = false;
    
    else
      return false;
     
    return true;

  }

  return false;

}


/**************************************************************************/
/*!

    @return true if successful, false if failed or no value
*/
/**************************************************************************/
bool parse_Coord( const char* coord )
{
  
  if ( coord == NULL || *coord == '\0')  // Check for errors
    return false;

  const char* e = strchr(coord, '.');  // Locate the decimal point  

  if ( e == NULL )  // Check for errors
    return false;

  ptrdiff_t int_len = e - coord;  // Determine if it's Lat with 4 digits or Lon with 5 digits

  if ( int_len != 4 && int_len != 5 )  // Check for errors
    return false;

  
  char digits[9] = { 0 };  // Buffer for the 9 digit chars: 5 left of '.' + 4 right of '.'


  if ( int_len == 5 )
    memcpy( digits, e - 5, 5 );    // Copy digits left of '.' lon: DDDMM

  else
    memcpy( digits + 1, e - 4, 4 );  // lat: DDMM, offset by 1 to align

  

  memcpy( digits + 5, e + 1, 4 );  // Copy digits right of '.'

  // Validate all chars are digits
  for (int i = ( int_len == 5 ? 0 : 1 ); i < 8; ++i )
  {

    if ( isdigit( digits[i] ) == false )
      return false;

    digits[i] -= '0'; // Convert in place

  }

  if ( int_len == 5 )
  {
    // Longitude
    gps_data.lon_DD_100 = digits[0];
    gps_data.lon_DD_10  = digits[1];
    gps_data.lon_DD_01  = digits[2];
    gps_data.lon_MM_10  = digits[3];
    gps_data.lon_MM_01  = digits[4];
    gps_data.lon_hh_10  = digits[5];
    gps_data.lon_hh_01  = digits[6];
    gps_data.lon_mm_10  = digits[7];
    gps_data.lon_mm_01  = digits[8];

    gps_data.lon_DD = 100 * gps_data.lon_DD_100 + 10 * gps_data.lon_DD_10 + gps_data.lon_DD_01;
    gps_data.lon_MM = 10 * gps_data.lon_MM_10 + gps_data.lon_MM_01;
    gps_data.lon_hh = 10 * gps_data.lon_hh_10 + gps_data.lon_hh_01;
    gps_data.lon_mm = 10 * gps_data.lon_mm_10 + gps_data.lon_mm_01;
  } 
  
  else
  {
    // Latitude
    gps_data.lat_DD_10 = digits[1];
    gps_data.lat_DD_01 = digits[2];
    gps_data.lat_MM_10 = digits[3];
    gps_data.lat_MM_01 = digits[4];
    gps_data.lat_hh_10 = digits[5];
    gps_data.lat_hh_01 = digits[6];
    gps_data.lat_mm_10 = digits[7];
    gps_data.lat_mm_01 = digits[8];

    gps_data.lat_DD = 10 * gps_data.lat_DD_10 + gps_data.lat_DD_01;
    gps_data.lat_MM = 10 * gps_data.lat_MM_10 + gps_data.lat_MM_01;
    gps_data.lat_hh = 10 * gps_data.lat_hh_10 + gps_data.lat_hh_01;
    gps_data.lat_mm = 10 * gps_data.lat_mm_10 + gps_data.lat_mm_01;
  }

  return true;
  
}


uint8_t hex_Str_To_Int( const char* chk_sum_in )
{

  uint8_t integer = 0;
  
  uint8_t delta;


  if( isalpha( chk_sum_in[0] ) )  // If first character is a letter
      delta = 'A' - 0xA;  // ( ASCII letter A = 0x41 ) - 0xA = 0x37

  else
    delta = '0';
  
  integer = ( chk_sum_in[0] - delta ) << 4;  // Shift first character to MSB position


  if( isalpha( chk_sum_in[1] ) )  // If second character is a letter
    delta = 'A' - 0xA;  // ( ASCII letter A = 0x41 ) - 0xA = 0x37

  else
    delta = '0';

  integer = integer | ( chk_sum_in[1] - delta );  // Combine converted first & second characters 
                           

  return( integer );

}



bool xsum_Check( const char* NMEA_data )
{

  uint8_t xsum = 0x00;

  uint8_t chk_sum = 0x00;

  char* ptr = strchr( NMEA_data, '*' ); // Find the location of the *, then go one more index over

  if ( !ptr || !*( ptr + 1 ) || !*( ptr + 2 ) )  // Erro check for no checksum characters found
    return false; 

  
  ptr++;  // Increment pointer to first digit in checksum field
  
  chk_sum = hex_Str_To_Int( ptr );  // Get NMEA sentence Check Sum value

  /* Calculate NMEA sentence Check Sum value
     Note that the checksum calculation does NOT include the $ character at the start of the NMEA sentence.
     Therefore we start the index at 1 instead of 0.
  */

  for( uint8_t i = 1; i < NMEA_DATA_MAX_SIZE && NMEA_data[i] != '*'; i++ )
    xsum ^= NMEA_data[i];
  
 
  // Determine if checksums match and return result
  return xsum == chk_sum; 

}