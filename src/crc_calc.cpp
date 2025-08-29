#include"crc_calc.h"


#if !defined( BIT_BY_BIT_CRC_CALC )

CRC16 crc( CRC16_X_25_POLYNOME, CRC16_X_25_INITIAL , CRC16_X_25_XOR_OUT , CRC16_X_25_REV_IN , CRC16_X_25_REV_OUT );


uint16_t calc_CRC()
{

  crc.restart();  // Reset internal state

  crc.add( dest_address, DEST_ADDR_SIZE );  // Feed dest

  crc.add( src_digi_addrs_ctrl_pid_flds, SRC_DIGI_ADDRS_CTRL_PID_FLDS_LEN );  // Feed src/digi

  crc.add( info, INFO_LEN );  // Feed info

  return crc.calc();  // Calculate CRC and return value

}


#else

void crc_Bit ( uint8_t tbyte, uint16_t &crc_value )
{

  bool carry = 0;

  carry = crc_value & 0x0001;

  crc_value = crc_value >> 1;

  if ( ( carry ^ tbyte ) == 0x01 )
    crc_value = crc_value ^ 0x8408; // 0x8408 = 0x1021 with bits reversed (CRC-CCITT reversed)

}


uint16_t calc_CRC()
{

  uint16_t crc_value = 0xFFFF;  // Initialize CRC value

  for ( uint8_t i = 0; i < sizeof( dest_address ); ++i )
    for ( uint8_t j = 0; j < 8; j++ )
      crc_Bit( bitRead( dest_address[i], j), crc_value  );

  for ( uint8_t i = 0; i < SRC_DIGI_ADDRS_CTL_PID_FLDS_LEN ; ++i )
    for ( uint8_t j = 0; j < 8; j++ )
      crc_Bit( bitRead( src_digi_addrs_ctl_pid_flds[i], j), crc_value  );

  for ( uint8_t i = 0; i < INFO_LEN; ++i )
    for ( uint8_t j = 0; j < 8; j++ )
      crc_Bit( bitRead( info[i], j ), crc_value );

  crc_value = ~crc_value;

  return crc_value;

}

#endif