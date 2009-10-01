MIL_3_Tfile_Hdr_ 120A 94B modeler 40 44170AEF 4AC4A8A7 6 lefkada-laptop theoleyr 0 0 none none 0 0 none CA89525A 11F3 0 0 0 0 0 0 11ab 5                                                                                                                                                                                                                                                                                                                                                                                                     ����             0x0             NONE                              	   (   (              
   PLCP_Preamble_and_Header   
       
      >This field represents the physical layer overhead added to the   8WLAN MAC data packets, namely the PLCP Preamble and PLCP   Header fields.       7The size of the PLCP overhead depends on physical layer   ?technology that is used for transmissions. Infra-red technology   <has the shortest overhead, which is 57 microseconds (this is   >equivalent of 57 bits since PLCP fields are transmitted at the   Alowest data rate, which is 1 Mbps). If another technology is used   :then the additional size is added to the total size of the   packet as bulk size.       =As mentioned above, the PLCP fields are transmitted at 1 Mbps   =regardless of the data rate that will be used for the rest of   >the packet (one exception: when infra-red is used, also a part   ?of the PLCP header is transmitted at the data rate that will be   9used for the rest of the packet). To model this behaviour   >accurately, more bits are added to the bulk size of the packet   :if the data rate that will be used for the transmission is   =different than 1 Mbps. For instance, a MAC that operates over   =FHSS technology, whose PLCP overhead is 128 microseconds, and   ?chooses its transmission rate as 11 Mbps will set the bulk size   of each data frame to       11 * 128 - 57 = 1351 bits       ?before the transmissions, so that the transmission delay of the   @bulk together with this field will take exactly 128 microseconds   as expexted.   
       
   information   
          signed, big endian          
   57   
       ����             set             NONE          
@������   
       J   1   J                    pk_field      (   Z              J   source   J                     
   integer   
          signed, big endian          J   32   J       ����             unset             NONE          
@�������   
       J   2   J                    pk_field      (   x              J   destination   J                     
   integer   
          signed, big endian          J   32   J       ����             unset             NONE          
@�������   
       J   3   J                    pk_field      (   �              J   type   J                     
   integer   
          signed, big endian          
   4   
       ����             unset             NONE          
@�������   
       J   4   J                    pk_field      d   �              J   nb_tier   J                     
   integer   
          signed, big endian          
   4   
       ����             unset             NONE          
@�������   
       J   5   J                    pk_field      �   �              J   branch   J                     
   integer   
          signed, big endian          
   4   
       ����             unset             NONE          
@�������   
       J   6   J                    pk_field      (  6              
   FCS   
       
      'Frame check sum (currently not modeled)   
       
   information   
          signed, big endian          
   32   
       ����             set             NONE          
@�������   
       J   7   J                    pk_field      (  ^              
   Accept   
       
      <This packet field is enabled by the pipeline stage when the	   +received frame is an erroneous frame.						   
          integer             signed, big endian          
   0   
       ����          
   set   
          NONE          
@�������   
       J   8   J                    pk_field      d  ^              
   Data Packet ID   
       
      4Stores the id of the data packet (MSDU) in service.	   
          integer             signed, big endian          
   0   
       ����             set             NONE          
@������   
       J   9   J                    pk_field   