uint8_t dest_address[7];


const uint8_t src_digi_addrs_ctl_pid_flds[]  = { 

    'A'<<1   //        A  #0 Begin of Source Address Field
  , 'I'<<1   //        I
  , '4'<<1   //        4
  , 'Q'<<1   //        Q
  , 'X'<<1   //        X
  , ' '<<1   //       ' '
  , ('3'<<1) + 1  //  -3  #6 End Source Address and Digipeater Address Fields
  , 0x03               // #7 Control Field (UI-Frame)
  , 0xF0               // #8 Protocol ID Field (no layer 3 protocol)

};


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

  ,'"'   // #10  Begin Altitude Field
  ,'3'
  ,'r'
  ,'}'  // #13 End Altitude Field 
 

};  


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
  ,'z' // #24 End Radio Frequency Field

*/