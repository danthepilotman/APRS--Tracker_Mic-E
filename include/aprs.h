/*  Moved PTT and DISP MODE pins to A0,3
 *  IMPORTANT: Pin state interrupts only available on pins 2,3 on Pro Mini
*/

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

#define DEBUG false  // Set to 'true' to enable debugging serial prints

#define BTN_DBOUCE_TIME 200            // ms

/********** Smart Beaconing Parameters **********/

#define FAST_SPEED (float) 60.0        // mph
#define FAST_RATE (float) 60.0         // seconds  
#define SLOW_SPEED (float) 5.0         // mph
#define SLOW_RATE (uint16_t) 600       // seconds  
#define MIN_TURN_TIME (uint16_t) 7    // seconds
#define MIN_TURN_ANGLE (float ) 30.0   // degrees
#define TURN_SLOPE (float) 255.0       // unitless

/********** LCD Parameters **********/

#define LCD_I2C_ADDR 0x27
#define LCD_COL 16
#define LCD_ROW 2
#define TOP_ROW 0
#define BOT_ROW 1
#define SPD_FLD_OFFSET 0  // LCD display x-axis offset for speed
#define ALT_FLD_OFFSET 4  // LCD display x-axis offset for altitude
#define TRK_FLD_OFFSET 8  // LCD display x-axis offset for course
#define SAT_FLD_OFFSET 14  // LCD display x-axis offset for # of satellites
#define NUM_OF_DISP_SCREENS 6  // Number of unique display screen options

/********** Serial Port Parameters **********/

#define gpsSerial Serial
#define GPS_BAUD_RATE 9600

/********** Digital Pin Parameters **********/

#define DISP_MODE_PIN 3
#define PTT_PIN A0  // Transmitter Push To Talk pin
#define GPS_VALID_PIN LED_BUILTIN  // LED for indicating valid GPS position

/********** Splash Screen Parameters **********/

#define SPLASH_SCRN_DLY 2000

/********** APRS Transmission Parameters **********/

#define MARK 1
#define SPACE 0
#define FLAG 0x7E           // Start/end FLAG(s) sent before and after data portion of packet
#define NUM_START_FLAGS 30  // Number of FLAGS to  send before data portion of packet
#define NUM_END_FLAGS 10    // Number of FLAGS to send at end of packet

#define TX_POWERUP_DLY 30    // Wait time between TX keying and begining of transmission. [ms]

#define CPU_FREQ 16E6  // MCU clock frequency
#define PRE_SCLR 1     // Timer Pre-scaler value
#define WAVE_ARRY_SIZE sizeof( SIN_ARRAY ) 

#define BAUD_FREQ 1200  // Transmit baud rate [Hz]
#define MRK_FREQ 1200   // Mark (1) waveform frequency [Hz]
#define SPC_FREQ 2200   // Space (0) waveform frequency [Hz]

#define MRK_NUM_SAMP   round (  ( (float) MRK_FREQ / (float) BAUD_FREQ ) * (float) WAVE_ARRY_SIZE  )
#define SPC_NUM_SAMP   round (  ( (float) SPC_FREQ / (float) BAUD_FREQ ) * (float) WAVE_ARRY_SIZE  )

#define MRK_TMR_CMP    round(  ( (float)CPU_FREQ  / (float) ( PRE_SCLR * BAUD_FREQ * MRK_NUM_SAMP ) ) - 1 )
#define SPC_TMR_CMP    round(  ( (float)CPU_FREQ  / (float) ( PRE_SCLR * BAUD_FREQ * SPC_NUM_SAMP ) ) - 1  )
#define BAUD_TIMER_CMP round(  ( (float)CPU_FREQ  / (float) ( PRE_SCLR * BAUD_FREQ ) ) - 1  )

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

#define NMEA_DATA_MAX_SIZE 80

/********** APRS AX.25 Parameters **********/

#define ALT_INDX 10 // starting index of the altitude data field in the info array

#define WIDE2_2 2  // APRS Digi Path Code

/********** Unit Conversion Factors **********/

#define M_to_F ( float ) 3.28084  // Conversion factor between meters to feet
#define KTS_to_MPH (  float ) 1.15078  // Conversion factor between kts to mph

/********** Global Variables **********/

volatile byte smp_num = 0;  // stores current sine array sample to put onto output port pins

volatile bool baud_tmr_isr_busy = true;  // Timer 1 used for 1200 baud timing

volatile byte lcd_disp_mode;

const byte SIN_ARRAY[] = {
8,8,9,10,10,11,12,12,
13,13,14,14,14,15,15,15,
15,15,15,15,14,14,14,13,
13,12,12,11,10,10,9,8,
8,7,6,5,5,4,3,3,
2,2,1,1,1,0,0,0,
0,0,0,0,1,1,1,2,
2,3,3,4,5,5,6,7
};


struct GPS_data
{
   
  byte hour;          // GMT hours
  byte minute;        // GMT minutes
  byte seconds;       // GMT seconds
  byte year;          // GMT year
  byte month;         // GMT month
  byte day;           // GMT day


  unsigned char lat_DD_10;
  unsigned char lat_DD_01;
  unsigned char lat_MM_10;
  unsigned char lat_MM_01;
  unsigned char lat_hh_10;
  unsigned char lat_hh_01;
  unsigned char lat_mm_10;
  unsigned char lat_mm_01;


  byte lat_DD;
  byte lat_MM;
  byte lat_hh;
  byte lat_mm;
  
  char NorS;

  unsigned char lon_DD_100;
  unsigned char lon_DD_10;
  unsigned char lon_DD_01;
  unsigned char lon_MM_10;
  unsigned char lon_MM_01;
  unsigned char lon_hh_10;
  unsigned char lon_hh_01;
  unsigned char lon_mm_10;
  unsigned char lon_mm_01;

  int lon_DD;
  byte lon_MM;
  byte lon_hh;
  byte lon_mm;

  char EorW;
    
  unsigned long altitude;  // Altitude in meters above MSL (32 bits)
  unsigned int speed;      // Current speed over ground in knots (16 bits)
  unsigned int course;     // Course in degrees from true north (16 bits )
  
  bool fix;            //  Have a fix?
  byte fixquality;     //  Fix quality (0, 1, 2 = Invalid, GPS, DGPS)
  byte fixquality_3d;  //  3D fix quality (1, 2, 3 = Nofix, 2D fix, 3D fix)
  byte satellites;     //  Number of satellites in use

} gps_data;

/********** Object Instance Declarations **********/

LiquidCrystal_I2C lcd( LCD_I2C_ADDR, LCD_COL, LCD_ROW );

/********** Additional Included Files **********/

#include "packet_data.h"
#include "gps_string_cap_case.cpp"
#include "display_LCD.cpp"
#include "mark_space_gen.cpp"
#include "setup_functions.cpp"
#include "packet_data_compressed_gps.cpp"
#include "crc_calc.cpp"
#include "send_packet.cpp"