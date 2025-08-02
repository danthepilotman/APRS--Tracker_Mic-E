#include"aprs.h"

volatile uint32_t phase_accumulator = 0;  // Stores current sine array sample to put onto output port pins

volatile uint32_t current_phase_step = MRK_PHASE_STEP;  // Stores current sine array sample to put onto output port pins

volatile bool baud_tmr_isr;  // Baud timer interrupt active flag

volatile uint8_t disp_mode = 0;  // Used to store display mode



uint8_t dest_address[DEST_ADDR_SIZE];  // APRS Destination address


const uint8_t src_digi_addrs_ctl_pid_flds[]  = { 

    (uint8_t) ( CALL_SIGN[0] << 1 )   //        A  #0 Begin of Source Address Field
  , (uint8_t) ( CALL_SIGN[1] << 1 )   //        I
  , (uint8_t) ( CALL_SIGN[2] << 1 )   //        4
  , (uint8_t) ( CALL_SIGN[3] << 1 )   //        Q
  , (uint8_t) ( CALL_SIGN[4] << 1 )   //        X
  , (uint8_t) ( CALL_SIGN[5] << 1 )   //       ' '
  , (uint8_t) ( ( CALL_SIGN[6] << 1 ) + 1 ) //  -1  #6 End Source Address and Digipeater Address Fields
  , 0x03  // #7 Control Field (UI-Frame)
  , 0xF0  // #8 Protocol ID Field (no layer 3 protocol)

};

constexpr uint8_t SRC_DIGI_ADDRS_CTL_PID_FLDS_LEN = sizeof( src_digi_addrs_ctl_pid_flds );


uint8_t info[] = {
  
   '`'   // #0  Current GPS Data
  ,'l'   // #1  d+28
  ,'K'   // #2  m+28
  ,'o'   // #3  h+28
  ,'o'   // #4  SP+28
  ,'{'   // #5  DC+28
  ,'I'   // #6  SE+28
  ,'>'   // #7  Symbol Code, O = Ballon, ' = Small Aircraft, ^ = Large Aircraft, > = Car
  ,'/'   // #8  Symbol Table ID,  Primary Symbol Table = '/', Alternate Symbol Table = '\'
  ,'"'   // #9  Begin Altitude Field
  ,'3'
  ,'r'
  ,'}'  // #12 End Altitude Field 
  ,'1'  // #13 Begin Radio Frequency Field
  ,'4'
  ,'6'
  ,'.'
  ,'6'
  ,'1'
  ,'0'
  ,'M'
  ,'H'
  ,'z' // #22 End Radio Frequency Field
  
};  

constexpr uint8_t INFO_LEN = sizeof( info );


// http://www.aprs.org/aprs12/mic-e-examples.txt
// http://www.aprs.org/aprs12/mic-e-types.txt



/*

  ,'\''  // #9 The ' character.  Display's manufacturer "McE-trk" or "McTrackr". One-way Tracker.
  ,'"'   // #10  Begin Altitude Field
  ,'3'
  ,'r'
  ,'}'  // #13 End Altitude Field 
  ,'1'  // #14 Begin Radio Frequency Field
  ,'4'
  ,'6'
  ,'.'
  ,'6'
  ,'1'
  ,'0'
  ,'M'
  ,'H'
  ,'z' // #23 End Radio Frequency Field
 
*/