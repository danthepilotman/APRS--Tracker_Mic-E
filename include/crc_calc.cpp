void crcbit ( byte tbyte, unsigned short &crc_value ) 
{

  bool carry = 0;

  carry = crc_value & 0x0001;

  crc_value = crc_value >> 1;

  if ( ( carry ^ tbyte ) == 0x01 ) 
    crc_value = crc_value ^ 0x8408; // 0x8408 = 0x1021 with bits reversed (CRC-CCITT reversed)

}


unsigned short calc_crc()  
{
  unsigned short crc_value = 0xFFFF;  // Initialize CRC value

  for ( uint8_t i = 0; i < sizeof( dest_address ); i++ ) 
    for ( uint8_t j = 0; j < 8; j++ )  
      crcbit( bitRead( dest_address[i], j), crc_value  );

  for ( uint8_t i = 0; i < sizeof( src_digi_addrs_ctl_pid_flds ); i++ ) 
    for ( uint8_t j = 0; j < 8; j++ )  
      crcbit( bitRead( src_digi_addrs_ctl_pid_flds[i], j), crc_value  );                                       

  for ( uint8_t i = 0; i < sizeof( info ); i++ ) 
    for ( uint8_t j = 0; j < 8; j++ )
      crcbit( bitRead( info[i], j ), crc_value );     

  crc_value = ~crc_value; 

  return crc_value; 

}