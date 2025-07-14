#ifndef PACKET_COMPRESSES_DATA_H
#define PACKET_COMPRESSES_DATA_H


#include "aprs.h"


/******** Function prototypes ***********/

void compute_dest_address( uint8_t mic_e_message );
void compute_info_longitude();
void compute_info_spd_crs();
void compute_info_alt();
void compute_Mic_E_data( uint8_t mic_e_message );

#endif