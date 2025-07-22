#ifndef PACKET_COMPRESSES_DATA_H
#define PACKET_COMPRESSES_DATA_H


#include "aprs.h"


/******** Function prototypes ***********/

void compute_Dest_Address( uint8_t mic_e_message );
void compute_Info_Longitude();
void compute_Info_Spd_Srs();
void compute_Info_Alt();
void compute_Mic_E_Data( uint8_t mic_e_message );

#endif