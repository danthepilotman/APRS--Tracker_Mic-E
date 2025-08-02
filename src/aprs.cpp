#include"aprs.h"

volatile uint32_t phase_accumulator = 0;  // Stores current sine array sample to put onto output port pins

volatile uint32_t current_phase_step = MRK_PHASE_STEP;  // Stores current sine array sample to put onto output port pins

volatile bool baud_tmr_isr_running;  // Baud timer interrupt active flag

volatile uint8_t disp_mode = 0;  // Used to store display mode



uint8_t dest_address[DEST_ADDR_SIZE];  // APRS Destination address


const uint8_t src_digi_addrs_ctrl_pid_flds[]  = { 

    (uint8_t) ( Call_Sign[0] << 1 )   //       #0 Begin of Source Address Field
  , (uint8_t) ( Call_Sign[1] << 1 )   //        
  , (uint8_t) ( Call_Sign[2] << 1 )   //        
  , (uint8_t) ( Call_Sign[3] << 1 )   //        
  , (uint8_t) ( Call_Sign[4] << 1 )   //        
  , (uint8_t) ( Call_Sign[5] << 1 )   //       
  , (uint8_t) ( ( Call_Sign[6] << 1 ) + 1 ) // #6 End Source Address and Digipeater Address Fields
  , 0x03  // #7 Control Field (UI-Frame)
  , 0xF0  // #8 Protocol ID Field (no layer 3 protocol)

};

constexpr uint8_t SRC_DIGI_ADDRS_CTRL_PID_FLDS_LEN = sizeof( src_digi_addrs_ctrl_pid_flds );


uint8_t info[] = {
  
   '`'   // #0  Data Type Identifier (DTI). Current GPS Data
  ,'l'   // #1  d+28
  ,'A'   // #2  m+28
  ,'u'   // #3  h+28
  ,' '   // #4  SP+28
  ,'"'   // #5  DC+28
  ,'>'   // #6  SE+28
  ,'>'   // #7  Symbol Code, O = Ballon, ' = Small Aircraft, ^ = Large Aircraft, > = Car
  ,'/'   // #8  Symbol Table ID,  Primary Symbol Table = '/', Alternate Symbol Table = '\'

 #ifdef SEND_ALTITUDE 
  ,'"'   // #9  Begin Altitude Field
  ,'3'
  ,'r'
  ,'}'  // #12 End Altitude Field 

#endif

#ifdef SEND_TELEMETRY

,'\''  // #9  Begin Telemetry Field - 5 printable hex telemetry values follow.
  ,'A'   // #10 Begin Telemetry Channel 1
  ,'7'
  ,'C'   // #12 Begin Telemetry Channel 2
  ,'F'  
  ,'4'   // #14 Begin Telemetry Channel 3
  ,'6'
  ,'0'   // #16 Begin Telemetry Channel 4
  ,'A'   
  ,'8'   // #18 Begin Telemetry Channel 5
  ,'D'   // #19 End Telemetry Field

  #endif

#ifdef SEND_COMMENT

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

#endif
  
};  


constexpr uint8_t INFO_LEN = sizeof( info );


// http://www.aprs.org/aprs12/mic-e-examples.txt
// http://www.aprs.org/aprs12/mic-e-types.txt