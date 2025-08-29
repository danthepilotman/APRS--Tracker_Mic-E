#ifndef GPS_NMEA_H
#define GPS_NMEA_H

#include <Arduino.h>
#include"aprs.h"
#include <Tiny4kOLED.h>
#include <stdlib.h>
#include <string.h>

#if defined( USE_WDT )

#include <avr/wdt.h>

#endif



class gps_NMEA {
  
    public:
    
    const uint8_t NMEA_DATA_MAX_SIZE = 83;  // Define maximum size of NMEA character array for storing NMEA sentences
  
#if defined( DEBUG )

    struct GPS_data  // Structure for storing GPS data
    {
    
    uint8_t hour = 20;          // GMT hours
    uint8_t minute = 00;        // GMT minutes
    uint8_t seconds = 15;       // GMT seconds
    uint8_t year = 25;          // GMT year
    uint8_t month = 7;          // GMT month
    uint8_t day = 29;           // GMT day


    uint8_t lat_DD_10 = 2;
    uint8_t lat_DD_01 = 8;
    uint8_t lat_MM_10 = 0;
    uint8_t lat_MM_01 = 1;
    uint8_t lat_mm_1 = 6;
    uint8_t lat_mm_01 = 2;
    uint8_t lat_mm_001 = 0;
    uint8_t lat_mm_0001 = 0;


    char NorS = 'N';

    uint8_t lon_DD_100 = 0;
    uint8_t lon_DD_10 = 8;
    uint8_t lon_DD_01 = 0;
    uint8_t lon_MM_10 = 3;
    uint8_t lon_MM_01 = 7;
    uint8_t lon_mm_1 = 8;
    uint8_t lon_mm_01 = 9;
    uint8_t lon_mm_001 = 0;
    uint8_t lon_mm_0001 = 0;

    uint8_t lon_DD = 80;
    uint8_t lon_MM = 37;
    uint8_t lon_mm = 89;

    char EorW = 'W';
        
    int16_t altitude = 5;   // Altitude in meters above MSL (32 bits)
    uint16_t speed = 0;     // Current speed over ground in knots (16 bits)
    uint16_t course = 234;  // Course in degrees from true north (16 bits )
    
    bool fix = true;            //  Have a fix?
    uint8_t fixquality = 1;     //  Fix quality (0, 1, 2 = Invalid, GPS, DGPS)
    uint8_t fixquality_3d = 3;  //  3D fix quality (1, 2, 3 = Nofix, 2D fix, 3D fix)
    uint8_t satellites = 12;    //  Number of satellites in use

    };

    
    #else

    struct GPS_data  // Structure for storing GPS data
    {
    
    uint8_t hour;     // GMT hours
    uint8_t minute;   // GMT minutes
    uint8_t seconds;  // GMT seconds
    uint8_t year;     // GMT year
    uint8_t month;    // GMT month
    uint8_t day;      // GMT day


    uint8_t lat_DD_10;    // Latitude degrees 10's digit
    uint8_t lat_DD_01;    // Latitude degrees 1's digit
    uint8_t lat_MM_10;    // Latitude minutes 10's digit
    uint8_t lat_MM_01;    // Latitude minutes 1's digit
    uint8_t lat_mm_1;     // Latitude minutes .1's digit
    uint8_t lat_mm_01;    // Latitude minutes .01's digit
    uint8_t lat_mm_001;   // Latitude minutes .001's digit
    uint8_t lat_mm_0001;  // Latitude minutes .0001's digit


    char NorS;  // Latitude Hemisphere

    uint8_t lon_DD_100;   // Longitude degrees 100's digit
    uint8_t lon_DD_10;    // Longitude degrees 10's digit
    uint8_t lon_DD_01;    // Longitude degrees 1's digit
    uint8_t lon_MM_10;    // Longitude minutes 10's digit
    uint8_t lon_MM_01;    // Longitude minutes 1's digit
    uint8_t lon_mm_1;     // Longitude minutes .1's digit
    uint8_t lon_mm_01;    // Longitude minutes .01's digit
    uint8_t lon_mm_001;   // Longitude minutes .001's digit
    uint8_t lon_mm_0001;  // Longitude minutes .0001's digit

    uint8_t lon_DD;  // Longitude degrees. Needed for MIC-E encoding
    uint8_t lon_MM;  // Longitude whole minutes. Needed for MIC-E encoding
    uint8_t lon_mm;  // Longitude decimal minutes (.mm). Needed for MIC-E encoding

    char EorW;  // Longitude Hemisphere
        
    int16_t altitude;  // Altitude in meters above MSL (32 bits)
    uint16_t speed;    // Current speed over ground in knots (16 bits)
    uint16_t course;   // Course in degrees from true north (16 bits )
    
    bool fix;               //  Have a fix?
    uint8_t fixquality;     //  Fix quality (0, 1, 2 = Invalid, GPS, DGPS)
    uint8_t fixquality_3d;  //  3D fix quality (1, 2, 3 = Nofix, 2D fix, 3D fix)
    uint8_t satellites;     //  Number of satellites in use

    } __attribute__((packed));  // 'packed' ensures that structure data members are stored sequentially in memory permitting pointer operations


#endif

    GPS_data gps_data;  // Create gps data structure

    const char *pos_fix[4] = { "Not available", "GPS SPS Mode",
                               "Differential GPS" , "GPS PPS Mode" };

    HardwareSerial& gpsSerial;     // Reference to the passed-in serial port
    uint32_t baudRate;


  gps_NMEA( HardwareSerial& serialPort, uint32_t baudRate ):
  gpsSerial( serialPort ), baudRate( baudRate ) {}

void get_GPS_Data();


  private:

    void get_NMEA_Sentence(char* NMEA_data);
    bool parse_GGA(const char* NMEA_data);
    bool parse_RMC(const char* NMEA_data);
    bool parse_GSA(const char* NMEA_data);
    char* next_field(const char* ptr);
    bool is_Empty(const char* pStart);
    bool parse_Time(const char* ptr);
    bool parse_Fix(const char* ptr);
    bool parse_Coord(const char* coord);
    uint8_t hex_Str_To_Int( const char* chk_sum);
    bool xsum_Check(const char* NMEA_data);

};


extern gps_NMEA my_gps;  // CI_V object instantiation for radio station # 1

#endif
