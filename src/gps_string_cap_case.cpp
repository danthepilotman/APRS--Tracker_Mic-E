#include"gps_string_cap_case.h"


gps_NMEA my_gps( Serial, 9600 );


void gps_NMEA::get_GPS_Data()
{

  char NMEA_data[NMEA_DATA_MAX_SIZE];

  bool GGA_good = false;
  bool GSA_good =  false;
  bool RMC_good = false;


#ifndef DEBUG

  memset( &gps_data, 0, sizeof( gps_data ) );

#endif

  while ( GGA_good == false || GSA_good == false || RMC_good == false )
  {
    get_NMEA_Sentence( NMEA_data );

    if ( strncmp( NMEA_data, "$GPGGA", 6 ) == 0 && GGA_good == false)
    {
      GGA_good = parse_GGA( NMEA_data );
      continue;
    }

    if ( strncmp( NMEA_data, "$GPGSA", 6 ) == 0 && GSA_good == false )
    {
      GSA_good = parse_GSA( NMEA_data );
      continue;
    }

    if ( strncmp( NMEA_data, "$GPRMC", 6 ) == 0 && RMC_good == false )
    {
      RMC_good = parse_RMC( NMEA_data );

#ifdef USE_OLED

      if ( gps_data.fix == false )
      {
        
        oled.setCursor( 0, 0 );
        
        for ( uint8_t i = 0; i < NMEA_DATA_MAX_SIZE && NMEA_data[i]; i++ )
        {
          oled.print ( NMEA_data[i] );
        }
        
          oled.clearToEOP();
      }

#endif

    }

#ifdef USE_WDT

    wdt_reset();

#endif

  }
}


void gps_NMEA::get_NMEA_Sentence( char* NMEA_data )
{
  
  while ( true )
  {
    digitalWrite( GPS_VALID_PIN, HIGH );

    while ( my_gps.gpsSerial.available() == false );
    
    NMEA_data[0] = my_gps.gpsSerial.read();

    if ( NMEA_data[0] != '$' )
      continue;

    for ( uint8_t i = 1; i < NMEA_DATA_MAX_SIZE; i++ )
    {
      while ( my_gps.gpsSerial.available() == false );
      
      NMEA_data[i] = my_gps.gpsSerial.read();
      
      if ( NMEA_data[i] == '\n' )
      {
        NMEA_data[i] = '\0';
        break;
      }
    }

    digitalWrite( GPS_VALID_PIN, LOW );

#ifdef DEBUG

    my_gps.gpsSerial.print(F("NMEA_data: "));
    my_gps.gpsSerial.println(NMEA_data);

#endif

    if  (gps_NMEA::xsum_Check( NMEA_data ) )
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
  
  const char* ptr = next_field( NMEA_data );
  
  ptr = next_field( ptr );
  ptr = next_field( ptr );
  ptr = next_field( ptr );
  ptr = next_field( ptr );
  ptr = next_field( ptr );

  if (is_Empty(ptr) == false)
  {
   gps_data.fixquality = atoi( ptr );
   gps_data.fix =gps_data.fixquality > 0;
  }

  if ( gps_data.fix == false )
   return false;

  ptr = next_field( ptr );

  if ( is_Empty(ptr) == false )
    gps_data.satellites = atoi( ptr );

  ptr = next_field( ptr );
  ptr = next_field( ptr );

  if ( is_Empty(ptr) == false )
  gps_data.altitude = int16_t( round( atof( ptr ) ) );

  return true;
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

  if (is_Empty(ptr) == false )  // Check for errors
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
  
  return ( pStart == NULL || *pStart == ',' || *pStart == '*' );  // Check that it is not empty, nor another comma, nor the '*' for the checksum field

}


bool gps_NMEA::parse_Time( const char* ptr )
{
  
  if (is_Empty(ptr) == false )  // Check for empty field 
  {
    unsigned long time = atol( ptr );   // Convert string to 32-bit integer
    gps_data.hour = time / 10000;  // Hour is located 4 digits from the right (10^4 = 10000)
    gps_data.minute = (time % 10000) / 100;  // Seconds is the first two digits (10^2 = 100) on the last 4 digits on the right (10^4=10000)
    gps_data.seconds = (time % 100);  // Seconds is the last two digits on the right (10^2)
    return true;   // Return true after computing time
  }
  
  return false;  // If field is empty report failure
}


bool gps_NMEA::parse_Fix( const char* ptr )
{
  
  if (is_Empty(ptr) == false )  // Check for empty field
  {
    if (ptr[0] == 'A') 
      gps_data.fix = true;   // 'A' represents a good fix    
    else if (ptr[0] == 'V')
      gps_data.fix = false;  // 'V' represents lack of a valid fix 
    
    else return false;  // Return false if character is not an 'A' or a 'V'
    
    return true;  // If you made it this far down everything checked out okay
  }
  
  return false;  // Return false if field is empty
}


bool gps_NMEA::parse_Coord( const char* coord )
{
  
  if ( coord == NULL || *coord == '\0' )
    return false;

  const char* dp = strchr( coord, '.' );

  if ( dp == NULL  )
   return false;

  ptrdiff_t int_len = dp - coord;


  if ( int_len != 4 && int_len != 5 )
    return false;

  char digits[9] = {0};

  if ( int_len == 5 )
    memcpy( digits, dp - 5, 5 );
  
  else
    memcpy( digits + 1, dp - 4, 4 );
  
  memcpy( digits + 5, dp + 1, 4 );

  for ( int i = ( int_len == 5 ? 0 : 1 ); i < 9; ++i )
  {
    if ( isdigit( digits[i] ) == false )
      return false;

    digits[i] -= '0';
  }

  if ( int_len == 5 )
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
    //my_gps.gps_data.lon_mm = 10 * my_gps.gps_data.lon_mm_10 + my_gps.gps_data.lon_mm_01;  // Compute integer value from tens and ones components
  } 

  else 
  {
    my_gps.gps_data.lat_DD_10 = digits[1];  // Capture lat/lon 10's digit


    my_gps.gps_data.lat_DD_01 = digits[2];  // Capture lat/lon 1's digit


    my_gps.gps_data.lat_MM_10 = digits[3];  // Capture lat/lon 10's digit


    my_gps.gps_data.lat_MM_01 = digits[4];  // Capture lat/lon 1's digit


    my_gps.gps_data.lat_hh_10 = digits[5];  // Capture lat/lon 10's digit


    my_gps.gps_data.lat_hh_01 = digits[6];  // Capture lat/lon 1's digit


    my_gps.gps_data.lat_mm_10 = digits[7];  // Capture lat/lon 10's digit


    my_gps.gps_data.lat_mm_01 = digits[8];  // Capture lat/lon 1's digit



    // my_gps.gps_data.lat_DD = 10 *  my_gps.gps_data.lat_DD_10 +  my_gps.gps_data.lat_DD_01;  // Compute integer value from tens and ones components
    // my_gps.gps_data.lat_MM = 10 *  my_gps.gps_data.lat_MM_10 +  my_gps.gps_data.lat_MM_01;  // Compute integer value from tens and ones components
    // my_gps.gps_data.lat_hh = 10 *  my_gps.gps_data.lat_hh_10 +  my_gps.gps_data.lat_hh_01;  // Compute integer value from tens and ones components
    // my_gps.gps_data.lat_mm = 10 *  my_gps.gps_data.lat_mm_10 +  my_gps.gps_data.lat_mm_01;  // Compute integer value from tens and ones components
  }

  return true;
}


uint8_t gps_NMEA::hex_Str_To_Int(const char* chk_sum_in)
{
  
  uint8_t integer;
  
  uint8_t delta;

  if (isalpha(chk_sum_in[0]))
   delta = 'A' - 0xA;
  else
   delta = '0';

  integer = (chk_sum_in[0] - delta) << 4;

  if (isalpha(chk_sum_in[1]))
   delta = 'A' - 0xA;
  
   else 
    delta = '0';

  integer |= (chk_sum_in[1] - delta);

  return integer;
}


bool gps_NMEA::xsum_Check(const char* NMEA_data)
{

  uint8_t computed_check_sum = 0;

  char* ptr = strchr(NMEA_data, '*');

  if ( !ptr || !*(ptr + 1) || !*(ptr + 2) )
   return false;

  ptr++;

  uint8_t received_check_sum = hex_Str_To_Int(ptr);

  for (uint8_t i = 1; i < NMEA_DATA_MAX_SIZE && NMEA_data[i] != '*'; i++)
    computed_check_sum ^= NMEA_data[i];

  return computed_check_sum == received_check_sum;

}