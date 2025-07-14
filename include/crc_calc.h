#ifndef CALC_CRC_H
#define CALC_CRC_H


#include"aprs.h"

/******** Function prototypes ***********/

void crcbit ( uint8_t tbyte, uint16_t &crc_value );
uint16_t calc_crc() ;


#endif