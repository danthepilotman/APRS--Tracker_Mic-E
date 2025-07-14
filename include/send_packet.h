#ifndef SEND_PACKET_H
#define SEND_PACKET_H


#include"aprs.h"
#include"crc_calc.h"


/******** Function prototypes ***********/

void send_tone( bool afsk_tone );
void sendbyte ( uint8_t inbyte, bool flag_in, uint8_t &stuff_ctr );
void send_packet();
bool smart_beaconing ( uint16_t &beacon_period, uint16_t secs_since_beacon, uint8_t &mic_e_message );

#endif
