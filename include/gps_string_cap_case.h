#ifndef GPS_STRING_CAP_CASE_H
#define GPS_STRING_CAP_CASE_H

#include <stdlib.h>
#include <string.h>
#include "aprs.h"
#include<Tiny4kOLED.h>
#include <avr/wdt.h>

/******** Function prototypes ***********/

void get_GPS_Data(); 
void get_NMEA_Sentence( char* NMEA_data );
bool parse_GGA( char* NMEA_data );
bool parse_RMC( char* NMEA_data );
bool parse_GSA( char* NMEA_data );
bool is_Empty( const char* pStart );  // Check for empty data fields in the NMEA sentence
bool parse_Time( const char* ptr );
bool parse_Fix( const char* ptr );
bool parse_Coord( char* coord );
uint8_t hex_Str_To_Int( const char* chk_sum );  // Converts string of characters representing hex values to an integer
bool xsum_Check( char* NMEA_data );  // Computes and cross checks NMEA sentence checksum

#endif