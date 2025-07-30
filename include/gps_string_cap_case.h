#ifndef GPS_STRING_CAP_CASE_H
#define GPS_STRING_CAP_CASE_H

#include <stdlib.h>
#include <string.h>
#include "aprs.h"
#include<Tiny4kOLED.h>
#include <avr/wdt.h>

/******** Function prototypes ***********/

void get_GPS_Data();  // Capture of valid, checksum verified NMEA sentences
void get_NMEA_Sentence( char* NMEA_data );  // Serial capture of GPS NMEA sentence
bool parse_GGA( const char* NMEA_data );   // Parse NMEA sentence
bool parse_RMC( const char* NMEA_data );   // Parse NMEA sentence
bool parse_GSA( const char* NMEA_data );  // Parse NMEA sentence
char* next_field( const char* ptr );  // Go to next data field
bool is_Empty( const char* pStart );  // Check for empty data fields in the NMEA sentence
bool parse_Time( const char* ptr );  // Parse time and date
bool parse_Fix( const char* ptr );  // Parse valid fix state
bool parse_Coord( const char* coord );  // Parse Lat/Lon coordinates
uint8_t hex_Str_To_Int( const char* chk_sum );  // Converts string of characters representing hex values to an integer
bool xsum_Check( const char* NMEA_data );  // Computes and cross checks NMEA sentence checksum

#endif