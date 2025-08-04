#ifndef SEND_PACKET_H
#define SEND_PACKET_H


#include"aprs.h"
#include"crc_calc.h"
#include"gps_string_cap_case.h"
#include"display_OLED.h"
#include"packet_data_compressed_gps.h"


/******** Function prototypes ***********/

void send_Tone( bool afsk_tone );  // Sends tone via DAC by setting proper phase step
void send_Byte ( uint8_t inbyte );  // Send 8 bits worth data. Toggles tone frequency if bit symbol is a SPACE. Performs bit stuffing if needed.
void send_Packet();  // Sends full APRS packet, sends start flags, destination field, Source, Digipeater Addresses / Control, PID fields , info field, CRC and end flags
bool smart_Beaconing ( uint16_t &beacon_period, uint16_t secs_since_beacon,uint8_t &mic_e_message );  // Detects turns and sets beaconing rate based on speed
void mic_E_Beacon();

#endif
