#ifndef APRS_H
#define APRS_H

#include <Arduino.h>
#include <math.h>


/******************************************* Option Flags *******************************************/

//#define DEBUG  // Set to 'true' to enable debugging serial prints

#define SEND_ALTITUDE  // Telemetry and Altitude are mutually exclusive
//#define SEND_TELEMETRY  // Telemetry and Altitude are mutually exclusive
#define SEND_COMMENT

#define USE_OLED
#define USE_WDT
#define USE_GPS
// #define BIT_BY_BIT_CRC_CALC  // Comment in to use original, bit-by-bit version of CRC calculation

/**************** Smart Beaconing Parameters ****************/

 constexpr uint16_t FAST_SPEED = 60;  // mph
 constexpr uint16_t FAST_RATE  = 60;  // seconds  
 
 constexpr uint16_t SLOW_SPEED  = 5;  // mph

#ifdef DEBUG

 constexpr uint16_t SLOW_RATE = 30;  // seconds 

#else

 constexpr uint16_t SLOW_RATE = 600;  // seconds 
 
#endif

 constexpr uint16_t MIN_TURN_TIME = 7;    // seconds
 constexpr uint16_t MIN_TURN_ANGLE = 30;  // degrees
 constexpr uint16_t TURN_SLOPE = 255;     // unitless

/******************************* OLED Parameters *******************************/

 constexpr uint8_t width = 128;
 constexpr uint8_t height = 64;

 constexpr uint8_t OLED_COLS = 16;  // 16 columns for 8x16 font size
 constexpr uint8_t FIRST_ROW = 0;
 constexpr uint8_t SECOND_ROW = 2;  // 2 pages per font height
 constexpr uint8_t THIRD_ROW = 4;
 constexpr uint8_t FOURTH_ROW = 6;
 
 /****************************** Splash Screen Parameters ******************************/

 constexpr uint32_t SPLASH_SCRN_DLY = 2000;  // How long to display splash screen [ms]

/******************************************** Digital Pin Parameters********************************************/

 constexpr uint8_t SCROLL_PIN = 2;  // IMPORTANT: Pin state interrupts only available on pins 2,3 on Pro Mini
 constexpr uint8_t SELECT_PIN = 3;  // IMPORTANT: Pin state interrupts only available on pins 2,3 on Pro Mini
 constexpr unsigned long  BTN_DBOUCE_TIME = 200;  // ms

 constexpr uint8_t PTT_PIN = A0;  // Transmitter Push To Talk pin
 constexpr uint8_t GPS_VALID_PIN = LED_BUILTIN;  // LED for indicating valid GPS position

/************************************************** APRS Transmission Parameters **************************************************/

 constexpr uint8_t MARK = 1;              // Mark symbol defined as binary 1
 constexpr uint8_t SPACE = 0;             // Space symbol defined as binary 0
 constexpr uint8_t FLAG = 0x7E;           // Start/end FLAG(s) sent before and after data portion of packet
 constexpr uint8_t NUM_START_FLAGS = 30;  // Number of FLAGS to  send before data portion of packet
 constexpr uint8_t NUM_END_FLAGS = 10;    // Number of FLAGS to send at end of packet

 constexpr uint32_t TX_POWERUP_DLY = 30;  // Wait time between TX keying and begining of transmission. [ms]

 const PROGMEM uint8_t SIN_ARRAY[] = {
  8,  9,  9, 10, 11, 11, 12, 13,
  13, 14, 14, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 14, 14, 13, 13,
  12, 11, 11, 10,  9,  9,  8,  7,
   7,  6,  5,  5,  4,  3,  3,  2,
   2,  1,  1,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  1,  1,  2,  2,
   3,  3,  4,  5,  5,  6,  7,  7
};
  
#if 0
 const PROGMEM uint8_t SIN_ARRAY[] = {  // Sampled sine integer array
  32, 36, 36, 42, 46, 46, 50, 54,
  54, 58, 58, 63, 63, 63, 63, 63,
  63, 63, 63, 63, 58, 58, 54, 54,
  50, 46, 46, 42, 36, 36, 32, 28,
  28, 25, 21, 21, 17, 13, 13, 8,
  8, 4, 4, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 4, 4, 8, 8,
  13, 13, 17, 21, 21, 25, 28, 28
  };

#endif


 constexpr uint8_t WAVE_ARRY_SIZE = sizeof( SIN_ARRAY );  // Store size of sampled sine array


 constexpr uint32_t CPU_FREQ = 16E6;   // MCU clock frequency [Hz]
 constexpr uint8_t BAUD_PRE_SCLR = 1;  // Baud timer Pre-scaler value
 constexpr uint8_t TONE_PRE_SCLR = 8;  // Wave generator DAC timer Pre-scaler value


 constexpr uint16_t  BAUD_FREQ = 1200;  // Transmit baud rate [symbols/sec]
 constexpr uint16_t  MRK_FREQ = 1200;   // Mark (1) waveform frequency [Hz]
 constexpr uint16_t  SPC_FREQ = 2200;   // Space (0) waveform frequency [Hz]
 constexpr uint32_t  SAMPLE_RATE = 32000;  // 32 kHz fixed sample rate

constexpr uint32_t MRK_PHASE_STEP = uint32_t( ( double( MRK_FREQ ) / SAMPLE_RATE ) * double( 1ULL << 32 ) );  // 1ULL << 32 = 2^32
constexpr uint32_t SPC_PHASE_STEP = uint32_t( ( double( SPC_FREQ ) / SAMPLE_RATE ) * double( 1ULL << 32 ) );  // Phase step for Mark and Space tones

 constexpr uint8_t  TONE_TIMER_CMP = uint8_t ( double( CPU_FREQ ) / ( TONE_PRE_SCLR  * SAMPLE_RATE ) - 1 );  // Timer compare value for tone generator ISR

 constexpr uint16_t BAUD_TIMER_CMP = uint16_t( double( CPU_FREQ ) / ( BAUD_PRE_SCLR * BAUD_FREQ )  - 1 );  // Timer compare value for baud rate ISR

#define WAVE_PORT PORTB      // MCU port used to output waveform
#define WAVE_PORT_DDR DDRB   // MCU port used to output waveform data direction register
#define R2R_MASK 0b00001111  // bits PB0–PB3, PB7 & PB8 are for the quartz crystal

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

/*********************************************************************** APRS AX.25 Parameters ***********************************************************************/

enum DESTINATION_INDEXES : uint8_t { LAT_DIG_1, LAT_DIG_2, LAT_DIG_3, LAT_DIG_4, LAT_DIG_5, LAT_DIG_6, DIGI_PATH, DEST_ADDR_SIZE };  
// DES_ADDR_SIZE is not a valid destination field.
// It's used to determine the size of the destination field by adding one more elemnet to the enum. This captures the size of the enum since it is zero indexed by default.

enum MIC_E_MSG{ emergency, priority, special, commited, returning, in_service, en_route, off_duty };  // Enumerated Mic-E status messages

enum DIGI_PATH_CODE : uint8_t { VIA, WIDE_1_1, WIDE2_2, WIDE3_3, WIDE4_4, WIDE5_5, WIDE6_6, WIDE7_7, NORTH, SOUTH, EAST, WEST, NORTH_WIDE, SOUTH_WIDE, EAST_WIDE, WEST_WIDE };

enum INFORMATION_INDEXES : uint8_t { DATA_TYPE, d_28, m_28, h_28, SP_28, DC_28, SE_28, SYMBOL_CODE, SYMBOL_TABLE, ALT_INDX, MSG_INDX = ALT_INDX + 4 };
// The message field begins comes right after the altitude field which is 4 bytes long.

/*************************** Unit Conversion Factors ***************************/

 constexpr float M_to_F = 3.28084;  // Conversion factor between meters to feet
 constexpr float KTS_to_MPH = 1.15078;  // Conversion factor between kts to mph

/************************************************ Global Variables ************************************************/

// ARPS parameters

const char Call_Sign[] = "AI4QX 1";  // Set call sign here

constexpr uint8_t Digi_Path = WIDE2_2; 

// Timers

extern volatile uint32_t phase_accumulator;  // Stores current sine array sample to put onto output port pins

extern volatile uint32_t current_phase_step;  // Stores current phase steep need for either Space or Mark tone sampling

extern volatile bool baud_tmr_isr_running;  // Timer used for 1200 baud timing


// APRS packet fields

extern uint8_t dest_address[DEST_ADDR_SIZE];  // Destination address portion of APRS packet

extern const uint8_t src_digi_addrs_ctrl_pid_flds[];    // Source address, Control and PID portion of APRS packet
extern const uint8_t SRC_DIGI_ADDRS_CTRL_PID_FLDS_LEN;  // Used to store length of src_digi_addrs_ctl_pid_flds[] array

extern uint8_t info[];  // Infomation portion of APRS packet
extern const uint8_t INFO_LEN;  // Used to store length of info[] array

#endif  // APRS_H