#include"aprs.h"

volatile uint8_t smp_num = 0;  // stores current sine array sample to put onto output port pins

volatile bool baud_tmr_isr_busy = true;  // Timer 1 used for 1200 baud timing
 
volatile uint8_t disp_mode;  // Used to store display page

GPS_data gps_data; // GPS data structure


uint8_t dest_address[7];  // APRS Destination address


const uint8_t src_digi_addrs_ctl_pid_flds[]  = { 

    'A' << 1   //        A  #0 Begin of Source Address Field
  , 'I' << 1   //        I
  , '4' << 1   //        4
  , 'Q' << 1   //        Q
  , 'X' << 1   //        X
  , ' ' << 1   //       ' '
  , ( '1' << 1 ) + 1  //  -1  #6 End Source Address and Digipeater Address Fields
  , 0x03               // #7 Control Field (UI-Frame)
  , 0xF0               // #8 Protocol ID Field (no layer 3 protocol)

};

const uint8_t SRC_DIGI_ADDRS_CTL_PID_FLDS_LEN = sizeof(src_digi_addrs_ctl_pid_flds);

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
  ,'\''  // #9 The ' character.  Display's manufacturer "McE-trk" or "McTrackr". One-way Tracker.
  ,'"'   // #10  Begin Altitude Field
  ,'3'
  ,'r'
  ,'}'  // #13 End Altitude Field 
  ,'1'  // #14 Begin Radio Frequency Field
  ,'4'
  ,'6'
  ,'.'
  ,'8'
  ,'5'
  ,'0'
  ,'M'
  ,'H'
  ,'z' // #23 End Radio Frequency Field
 
};  


const uint8_t INFO_LEN = sizeof(info);


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
  ,'8'
  ,'5'
  ,'0'
  ,'M'
  ,'H'
  ,'z' // #23 End Radio Frequency Field

*/