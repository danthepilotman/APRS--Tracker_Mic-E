#include <Arduino.h>
#include <Tiny4kOLED.h>
#include <math.h>


//#define MCP4725_DAC  // Comment out to use 4 resistor ladder DAC vs MCP4725 DAC chip


#ifdef MCP4725_DAC

#include <Adafruit_MCP4725.h>

#endif


#define DEBUG false  // Set to 'true' to enable debugging serial prints


/********** Smart Beaconing Parameters **********/

#define FAST_SPEED (float) 60.0        // mph
#define FAST_RATE (float) 60.0         // seconds  
#define SLOW_SPEED (float) 5.0         // mph
#define SLOW_RATE (uint16_t) 600       // seconds  
#define MIN_TURN_TIME (uint16_t) 7    // seconds
#define MIN_TURN_ANGLE (float ) 30.0   // degrees
#define TURN_SLOPE (float) 255.0       // unitless

/********** OLED Parameters **********/

static constexpr uint8_t width = 128;
static constexpr uint8_t height = 64;

static constexpr uint8_t OLED_COLS = 21;
static constexpr uint8_t FIRST_ROW = 0;
static constexpr uint8_t SECOND_ROW = 10;
static constexpr uint8_t THIRD_ROW = 20;
static constexpr uint8_t FOURTH_ROW = 30;
static constexpr uint8_t SPD_FLD_OFFSET = 10;  // OLED display x-axis offset for speed
static constexpr uint8_t NUM_OF_DISP_SCREENS = 3;  // Number of unique display screen options

/********** Serial Port Parameters **********/

#define gpsSerial Serial
static constexpr unsigned long GPS_BAUD_RATE = 9600;

/********** Digital Pin Parameters **********/

static constexpr uint8_t DISP_MODE_PIN = 3;  // IMPORTANT: Pin state interrupts only available on pins 2,3 on Pro Mini
static constexpr unsigned long  BTN_DBOUCE_TIME = 200;  // ms

static constexpr uint8_t PTT_PIN = A0;  // Transmitter Push To Talk pin
static constexpr uint8_t GPS_VALID_PIN = LED_BUILTIN;  // LED for indicating valid GPS position

/********** Splash Screen Parameters **********/

static constexpr uint32_t SPLASH_SCRN_DLY = 2000;

/********** APRS Transmission Parameters **********/

static constexpr uint8_t MARK = 1;
static constexpr uint8_t SPACE = 0;
static constexpr uint8_t FLAG = 0x7E;           // Start/end FLAG(s) sent before and after data portion of packet
static constexpr uint8_t NUM_START_FLAGS = 30;  // Number of FLAGS to  send before data portion of packet
static constexpr uint8_t NUM_END_FLAGS = 10;    // Number of FLAGS to send at end of packet

static constexpr uint32_t TX_POWERUP_DLY = 30;    // Wait time between TX keying and begining of transmission. [ms]

static constexpr uint32_t CPU_FREQ = 16E6;  // MCU clock frequency
static constexpr uint8_t PRE_SCLR = 1;     // Timer Pre-scaler value


#ifdef MCP4725_DAC

const PROGMEM uint16_t DACLookup_FullSine_6Bit[64] =
{
  2048, 2248, 2447, 2642, 2831, 3013, 3185, 3346,
  3495, 3630, 3750, 3853, 3939, 4007, 4056, 4085,
  4095, 4085, 4056, 4007, 3939, 3853, 3750, 3630,
  3495, 3346, 3185, 3013, 2831, 2642, 2447, 2248,
  2048, 1847, 1648, 1453, 1264, 1082,  910,  749,
   600,  465,  345,  242,  156,   88,   39,   10,
     0,   10,   39,   88,  156,  242,  345,  465,
   600,  749,  910, 1082, 1264, 1453, 1648, 1847
};

static constexpr uint8_t WAVE_ARRY_SIZE = sizeof( DACLookup_FullSine_6Bit ); 

#else

const PROGMEM uint8_t SIN_ARRAY[64] = {
8,8,9,10,10,11,12,12,
13,13,14,14,14,15,15,15,
15,15,15,15,14,14,14,13,
13,12,12,11,10,10,9,8,
8,7,6,5,5,4,3,3,
2,2,1,1,1,0,0,0,
0,0,0,0,1,1,1,2,
2,3,3,4,5,5,6,7
};

#define WAVE_ARRY_SIZE sizeof( SIN_ARRAY ) 

#endif


static constexpr uint16_t BAUD_FREQ = 1200;  // Transmit baud rate [Hz]
static constexpr uint16_t  MRK_FREQ = 1200;   // Mark (1) waveform frequency [Hz]
static constexpr uint16_t  SPC_FREQ = 2200;   // Space (0) waveform frequency [Hz]

static constexpr uint8_t MRK_NUM_SAMP  = round (  ( (float) MRK_FREQ / (float) BAUD_FREQ ) * (float) WAVE_ARRY_SIZE  );
static constexpr uint8_t SPC_NUM_SAMP = round (  ( (float) SPC_FREQ / (float) BAUD_FREQ ) * (float) WAVE_ARRY_SIZE  );

static constexpr uint8_t MRK_TMR_CMP = (uint8_t) round( ( (float)CPU_FREQ  / (float) ( PRE_SCLR * BAUD_FREQ * MRK_NUM_SAMP ) ) - 1 );
static constexpr uint8_t SPC_TMR_CMP = (uint8_t) round( ( (float)CPU_FREQ  / (float) ( PRE_SCLR * BAUD_FREQ * SPC_NUM_SAMP ) ) - 1  );
static constexpr uint16_t BAUD_TIMER_CMP = round( ( (float)CPU_FREQ  / (float) ( PRE_SCLR * BAUD_FREQ ) ) - 1  );

#define WAVE_PORT PORTB     // MCU port used to output waveform
#define WAVE_PORT_DDR DDRB  // MCU port used to output waveform data direction register

//* Define timer register & ISR values for the Baud Rate Timer */
#define BAUD_TMR_ISR_VECT TIMER1_COMPA_vect
#define BAUD_TMR_TCCRA TCCR1A
#define BAUD_TMR_TCCRB TCCR1B
#define BAUD_TMR_TIMSK TIMSK1
#define BAUD_TMR_OCIEA OCIE1A
#define BAUD_TMR_OCRA OCR1A
#define BAUD_TMR_TCNT TCNT1
#define BAUD_TMR_WGM2 WGM12
#define BAUD_TMR_CS0 CS10

/* Define timer register & ISR values for the Waveform Generator Timer */
#define WAVE_GEN_TMR_ISR_VECT TIMER2_COMPA_vect 
#define WAVE_GEN_TMR_TCCRA TCCR2A
#define WAVE_GEN_TMR_TCCRB TCCR2B
#define WAVE_GEN_TMR_TIMSK TIMSK2
#define WAVE_GEN_TMR_OCIEA OCIE2A
#define WAVE_GEN_TMR_OCRA OCR2A
#define WAVE_GEN_TMR_TCNT TCNT2
#define WAVE_GEN_TMR_WGM1 WGM21
#define WAVE_GEN_TMR_CS0 CS20

#define delay_cycles(n) __builtin_avr_delay_cycles(n)

/********** NMEA-0183 Sentence Fields Parameters **********/

static constexpr uint8_t NMEA_DATA_MAX_SIZE = 80;

/********** APRS AX.25 Parameters **********/

static constexpr uint8_t ALT_INDX = 10; // starting index of the altitude data field in the info array

static constexpr uint8_t  WIDE2_2 = 2;  // APRS Digi Path Code

/********** Unit Conversion Factors **********/

#define M_to_F (float) 3.28084  // Conversion factor between meters to feet
#define KTS_to_MPH (float) 1.15078  // Conversion factor between kts to mph

/********** Global Variables **********/

volatile uint8_t smp_num = 0;  // stores current sine array sample to put onto output port pins

volatile bool baud_tmr_isr_busy = true;  // Timer 1 used for 1200 baud timing

volatile uint8_t disp_mode;


struct GPS_data
{
   
  uint8_t hour;          // GMT hours
  uint8_t minute;        // GMT minutes
  uint8_t seconds;       // GMT seconds
  uint8_t year;          // GMT year
  uint8_t month;         // GMT month
  uint8_t day;           // GMT day


  uint8_t lat_DD_10;
  uint8_t lat_DD_01;
  uint8_t lat_MM_10;
  uint8_t lat_MM_01;
  uint8_t lat_hh_10;
  uint8_t lat_hh_01;
  uint8_t lat_mm_10;
  uint8_t lat_mm_01;


  uint8_t lat_DD;
  uint8_t lat_MM;
  uint8_t lat_hh;
  uint8_t lat_mm;
  
  char NorS;

  uint8_t lon_DD_100;
  uint8_t lon_DD_10;
  uint8_t lon_DD_01;
  uint8_t lon_MM_10;
  uint8_t lon_MM_01;
  uint8_t lon_hh_10;
  uint8_t lon_hh_01;
  uint8_t lon_mm_10;
  uint8_t lon_mm_01;

  uint8_t lon_DD;
  uint8_t lon_MM;
  uint8_t lon_hh;
  uint8_t lon_mm;

  char EorW;
    
  uint16_t altitude;  // Altitude in meters above MSL (32 bits)
  uint16_t speed;      // Current speed over ground in knots (16 bits)
  uint16_t course;     // Course in degrees from true north (16 bits )
  
  bool fix;            //  Have a fix?
  uint8_t fixquality;     //  Fix quality (0, 1, 2 = Invalid, GPS, DGPS)
  uint8_t fixquality_3d;  //  3D fix quality (1, 2, 3 = Nofix, 2D fix, 3D fix)
  uint8_t satellites;     //  Number of satellites in use

} gps_data;

/********** Additional Included Files **********/

#include "packet_data.h"
#include "gps_string_cap_case.cpp"
#include "display_OLED.cpp"
#include "setup_functions.cpp"
#include "mark_space_gen.cpp"
#include "packet_data_compressed_gps.cpp"
#include "crc_calc.cpp"
#include "send_packet.cpp"