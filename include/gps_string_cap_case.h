#ifndef GPS_STRING_CAP_CASE_H
#define GPS_STRING_CAP_CASE_H

#include <stdlib.h>
#include <string.h>
#include "aprs.h"
#include<Tiny4kOLED.h>
#include <avr/wdt.h>

/******** Function prototypes ***********/

void get_GPS_data(); 
void get_NMEA_sentence( char* NMEA_data );
bool parse_GGA( char* NMEA_data );
bool parse_RMC( char* NMEA_data );
bool parse_GSA( char* NMEA_data );
bool isEmpty( const char* pStart );  // Check for empty data fields in the NMEA sentence
bool parseTime( const char* ptr );
bool parseFix( const char* ptr );
bool parseCoord( char* coord );
uint8_t hex_str_to_int( const char* chk_sum );  // Converts string of characters representing hex values to an integer
bool xsum_check( char* NMEA_data );  // Computes and cross checks NMEA sentence checksum

#endif