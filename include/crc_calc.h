#ifndef CALC_CRC_H
#define CALC_CRC_H


#include"aprs.h"


#ifndef BIT_BY_BIT_CRC_CALC

#include <CRC16.h>

extern CRC16 crc;  // Note: CRC16 can do CRC16_X_25

#else

/******** Function prototypes ***********/

void crcbit ( uint8_t tbyte, uint16_t &crc_value );  // Shift register bit based CRC calculation


#endif 


uint16_t calc_CRC() ;  // Top level CRC calculation function


#endif