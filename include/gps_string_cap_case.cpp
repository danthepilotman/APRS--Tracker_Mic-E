#include <stdlib.h>
#include <string.h>
#include <Tiny4kOLED.h>

/******** Function prototypes ***********/

uint8_t hex_str_to_int( const char* chk_sum );  // Converts string of characters representing hex values to an integer
bool xsum_check( char* NMEA_data );  // Computes and cross checks NMEA sentence checksum
bool isEmpty( const char* pStart );  // Check for empty data fields in the NMEA sentence
bool parseTime( const char* ptr );
bool parseFix( const char* ptr );
bool parse_GGA( char* NMEA_data );
bool parse_GSA( char* NMEA_data );
bool parse_RMC( char* NMEA_data );
bool parseCoord( char* coord );
void get_NMEA_sentence( char* NMEA_data );


void get_GPS_data() 
{

  char NMEA_data[NMEA_DATA_MAX_SIZE];  // Char array to store captured NMEA sentences

  bool GGA_not_cap = true;

  bool GSA_not_cap = true;

  bool RMC_not_cap = true;


  while ( GGA_not_cap || GSA_not_cap || RMC_not_cap )
  {

    get_NMEA_sentence ( NMEA_data );

    // Check that proper NMEA sentences have been captured

    if ( strncmp( NMEA_data, "$GPGGA", 6 ) == 0  &&  GGA_not_cap )
      GGA_not_cap = parse_GGA( NMEA_data );
      
    else if ( strncmp( NMEA_data, "$GPGSA", 6 ) == 0  &&  GSA_not_cap )
      GSA_not_cap = parse_GSA( NMEA_data );

    else if ( strncmp( NMEA_data, "$GPRMC", 6 ) == 0  &&  RMC_not_cap )
    {
      RMC_not_cap = parse_RMC( NMEA_data );

      if( gps_data.fix == false )  // Check if valid GPS data has been received
      {
        oled.setCursor( 0, 0 );  // Move to home position

        for( uint8_t i = 0; i < NMEA_DATA_MAX_SIZE; i++ )  // Loop through the NMEA sentence string
        {
          if ( NMEA_data[i] == '\0')  // Terminate string with null character
            break;
          else  
            oled.print( NMEA_data[i] );  // Print each NMEA sentence character
        }

        oled.clearToEOP();  // Erase old data
      }
    }
  }
} 



void get_NMEA_sentence( char* NMEA_data )
{

  get_NMEA_data:

  digitalWrite( GPS_VALID_PIN, LOW );  // Blink LED to indicate activity
  
  while ( gpsSerial.available() == 0 );  // Wait for serial data to be available
    
  NMEA_data[0] = gpsSerial.read();  // Read one character from Serial RX buffer

  if ( NMEA_data[0] != '$' )  // Loop back if not at the begining of a NMEA sentence  
    goto get_NMEA_data;  // Go back to the begining of the sentence capture sequence
    
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

  digitalWrite( GPS_VALID_PIN, HIGH );  // Blink LED to indicate activity

#if DEBUG
    Serial.print( "NMEA_data: " );
    Serial.println( NMEA_data );
#endif   

    
  // Get another NMEA sentence if Check Sums don't match

  if ( xsum_check( NMEA_data) )   // Negated logic on checksum_check 
  {

#if DEBUG
      Serial.println("Failed Xsum");
#endif 
      // continue;  // Returns to the begining of the while loop 
    goto get_NMEA_data;  // Go back to the begining of the sentence capture sequence

  }   

} 



//------------------Start of process for getting GPGGA sentence-----------------------
//strcpy(NMEA_data,"$GPGGA,161229.487,3723.2475,N,12158.3416,W,1,07,1.0,9.0,M,,,,0000*18");

bool parse_GGA( char* NMEA_data )
{

  char* ptr;
  
  ptr = NMEA_data;
  
  ptr = strchr( ptr, ',' ) + 1; // Skip over first ','
  
  ptr = strchr( ptr, ',' ) + 1; // Skip over time
      
  ptr = strchr( ptr, ',' ) + 1;  // Skip lat
  ptr = strchr( ptr, ',' ) + 1;  // Skip N/S
        
  ptr = strchr( ptr, ',' ) + 1;  // Skip lon
  ptr = strchr( ptr, ',' ) + 1;  // Skip E/W
    
  if ( isEmpty( ptr ) == false ) 
  { 
               
    gps_data.fixquality = atoi( ptr );  // Needs additional processing
      
    if ( gps_data.fixquality > 0 ) 
      gps_data.fix = true;       
    
    else
      gps_data.fix = false;
                     
  }

  if( gps_data.fix == false )
    return true;  // Go back and capture another NMEA sentence
                       
  ptr = strchr( ptr, ',' ) + 1; // Then move on to the next data field
  
  // Most can just be parsed with atoi() or atof(), then move on to the next.
  
  if ( isEmpty( ptr ) == false )
    gps_data.satellites = atoi( ptr );
    
  ptr = strchr( ptr, ',' ) + 1;  // Move to the next field
   
  ptr = strchr( ptr, ',' ) + 1;  // Move to the next field

  if ( isEmpty( ptr ) == false )
    gps_data.altitude = uint16_t( round( atof( ptr ) ) );
    
  // Skip the rest

  return false;

}


//------------------Start of process for getting GPRMC sentence-----------------------
// = {"$GPRMC,161229.487,A,3723.2475,N,12158.3416,W,0.13,309.62,120598,,*10"};

bool parse_RMC( char* NMEA_data )
{

  char* ptr;

  ptr = NMEA_data;

  ptr = strchr( ptr, ',' ) + 1;  // Skip the 1st comma

  parseTime( ptr );              // Get the time info
    
  ptr = strchr( ptr, ',' ) + 1;  // Skip the 2nd comma
    
  parseFix( ptr ); // Status

  if( gps_data.fix == false )
    return true;  // Go back and capture another NMEA sentence 
    
  ptr = strchr( ptr, ',' ) + 1;  // Skip the 3rd comma 

  parseCoord( ptr ); // Latitude
 
  ptr = strchr( ptr, ',' ) + 1;  // Skip the 4th comma 

  gps_data.NorS = *ptr; // Field is not empty

  ptr = strchr( ptr, ',' ) + 1;  // Skip the 5th comma 

  parseCoord( ptr );

  ptr = strchr( ptr, ',' ) + 1;  // Skip the 6th comma 

  gps_data.EorW = *ptr; // Field is not empty

  ptr = strchr( ptr, ',' ) + 1;  // Skip the 7th comma 

  if ( isEmpty ( ptr ) == false )
    gps_data.speed = ( unsigned int ) ( round( atof ( ptr ) ) );
    
  ptr = strchr( ptr, ',' ) + 1;  // Skip the 8th comma 

  if ( isEmpty ( ptr ) == false )
    gps_data.course = ( unsigned int ) ( round( atof ( ptr ) ) );
    
  ptr = strchr( ptr, ',' ) + 1;  // Skip the 9th comma 

  if ( isEmpty( ptr ) == false )
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

bool parse_GSA( char* NMEA_data )
{

  char* ptr;

  ptr = NMEA_data;

  ptr = strchr( ptr, ',' ) + 1;  // Move to the next field

  ptr = strchr( ptr, ',' ) + 1;  // Move to the next field

  if ( isEmpty( ptr ) == false )
    gps_data.fixquality_3d = atoi( ptr );

  return false;

}



/**************************************************************************/
/*!
    @brief Parse a part of an NMEA string for time. Independent of number
    of decimal places after the '.'
    @param ptr Pointer to the location of the token in the NMEA string
    @return true if successful, false otherwise
*/
/**************************************************************************/
bool isEmpty( const char* pStart ) 
{
  
  if ( ',' != *pStart && '*' != *pStart && pStart != NULL )
    return false;
  
  else
    return true;

}


bool parseTime( const char* ptr ) 
{
  
  if ( isEmpty( ptr ) == false) 
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
bool parseFix( const char* ptr ) 
{
  
  if ( isEmpty( ptr ) == false )
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
bool parseCoord( char* coord )
{
  
  const char* ptr = coord;
  
  if ( isEmpty( ptr ) == false )
  {
    
    char* e = strchr( ptr, '.' );
    
    if ( e == NULL || e - ptr > 5 )
      return false;                // No decimal point in range
    
    if ( e - ptr ==  5 )
    {

      gps_data.lon_DD_100 = *( e - 5 ) - '0';  // Get character at lon 100's location and convert to decimal digit
      gps_data.lon_DD_10  = *( e - 4 ) - '0';
      gps_data.lon_DD_01  = *( e - 3 ) - '0';

      gps_data.lon_MM_10  = *( e - 2 ) - '0';
      gps_data.lon_MM_01  = *( e - 1 ) - '0';

      gps_data.lon_hh_10  = *( e + 1 ) - '0';
      gps_data.lon_hh_01  = *( e + 2 ) - '0';

      gps_data.lon_mm_10  = *( e + 3 ) - '0';
      gps_data.lon_mm_01  = *( e + 4 ) - '0';

      gps_data.lon_DD = 100 * gps_data.lon_DD_100 + 10 * gps_data.lon_DD_10 + gps_data.lon_DD_01;

      gps_data.lon_MM = 10 * gps_data.lon_MM_10 + gps_data.lon_MM_01;

      gps_data.lon_hh = 10 * gps_data.lon_hh_10 + gps_data.lon_hh_01;

      gps_data.lon_mm = 10 * gps_data.lon_mm_10 + gps_data.lon_mm_01;

    }

    else if ( e - ptr == 4 )
    {

      gps_data.lat_DD_10 = *( e - 4 ) - '0';  // Get character at lat 10's location and convert to decimal digit
      gps_data.lat_DD_01 = *( e - 3 ) - '0';

      gps_data.lat_MM_10 = *( e - 2 ) - '0';
      gps_data.lat_MM_01 = *( e - 1 ) - '0';

      gps_data.lat_hh_10 = *( e + 1 ) - '0';
      gps_data.lat_hh_01 = *( e + 2 ) - '0';

      gps_data.lat_mm_10 = *( e + 3 ) - '0';
      gps_data.lat_mm_01 = *( e + 4 ) - '0';

      gps_data.lat_DD = 10 * gps_data.lat_DD_10 + gps_data.lat_DD_01;

      gps_data.lat_MM = 10 * gps_data.lat_MM_10 + gps_data.lat_MM_01;

      gps_data.lat_hh = 10 * gps_data.lat_hh_10 + gps_data.lat_hh_01;

      gps_data.lat_mm = 10 * gps_data.lat_mm_10 + gps_data.lat_mm_01;

    }
    
  }

  return true;

}



uint8_t hex_str_to_int( const char* chk_sum_in )
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



bool xsum_check( char* NMEA_data )
{

  uint8_t xsum = 0x00;

  uint8_t chk_sum = 0x00;

  char* ptr = NMEA_data;


  ptr = strchr( ptr, '*' ) + 1; // Find the location of the *, then go one more index over

  // Get NMEA sentence Check Sum value
  chk_sum = hex_str_to_int( ptr );

  /* Calculate NMEA sentence Check Sum value
     Note that the checksum calculation does NOT include the $ character at the start of the NMEA sentence.
     Therefore we start the index at 1 instead of 0.
  */

  for( uint8_t i = 1; i < NMEA_DATA_MAX_SIZE; i++ )
  { 
    if( NMEA_data[i] == '*' )
      break;

    xsum ^= NMEA_data[i];
  }
 
  
  // Determine if checksums match and return result
  if( xsum == chk_sum ) 
    return( false );

  else
    return( true );     

}