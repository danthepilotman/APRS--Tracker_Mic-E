#ifndef CALC_CRC_H
#define CALC_CRC_H


#include"aprs.h"


#if DEBUG == false

#include <CRC16.h>

extern CRC16 crc;  // Note: CRC16 can do CRC16_X_25

#else

/******** Function prototypes ***********/

void crcbit ( uint8_t tbyte, uint16_t &crc_value );


#endif 


uint16_t calc_CRC() ;


#endif