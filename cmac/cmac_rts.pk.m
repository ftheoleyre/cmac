MIL_3_Tfile_Hdr_ 81A 40A modeler 40 4410B767 4416D233 2 ares-theo-1 ftheoley 0 0 none none 0 0 none A4DAA5C0 10F3 0 0 0 0                                                                                                                                                                                                                                                                                                                                                                                                                    ����             0x0             NONE                                 (   (@��          
   PLCP_Preamble_and_Header   
       
      >This field represents the physical layer overhead added to the   8WLAN MAC data packets, namely the PLCP Preamble and PLCP   Header fields.       7The size of the PLCP overhead depends on physical layer   ?technology that is used for transmissions. Infra-red technology   <has the shortest overhead, which is 57 microseconds (this is   >equivalent of 57 bits since PLCP fields are transmitted at the   Alowest data rate, which is 1 Mbps). If another technology is used   :then the additional size is added to the total size of the   packet as bulk size.       =As mentioned above, the PLCP fields are transmitted at 1 Mbps   =regardless of the data rate that will be used for the rest of   >the packet (one exception: when infra-red is used, also a part   ?of the PLCP header is transmitted at the data rate that will be   9used for the rest of the packet). To model this behaviour   >accurately, more bits are added to the bulk size of the packet   :if the data rate that will be used for the transmission is   =different than 1 Mbps. For instance, a MAC that operates over   =FHSS technology, whose PLCP overhead is 128 microseconds, and   ?chooses its transmission rate as 11 Mbps will set the bulk size   of each data frame to       11 * 128 - 57 = 1351 bits       ?before the transmissions, so that the transmission delay of the   @bulk together with this field will take exactly 128 microseconds   as expexted.   
       
   information   
          signed, big endian          
   57   
       ����             set             NONE          
@������   
          1                       pk_field      (   Z@���          
   SOURCE   
                     
   integer   
          signed, big endian          
   48   
       ����             unset             NONE          
@�������   
          2                       pk_field      (   x@���          
   DESTINATION   
                     
   integer   
          signed, big endian          
   48   
       ����             unset             NONE          
@�������   
          3                       pk_field      (   �@���          
   TYPE   
                     
   integer   
          signed, big endian          
   4   
       ����             unset             NONE          
@�������   
          4                       pk_field      d   �@���          
   DURATION   
                     
   integer   
          signed, big endian          
   16   
       ����             unset             NONE          
@�������   
          5                       pk_field      (  6@���          
   FCS   
       
      'Frame check sum (currently not modeled)   
       
   information   
          signed, big endian          
   32   
       ����             set             NONE          
@�������   
          6                       pk_field      (  ^@���          
   Accept   
       
      <This packet field is enabled by the pipeline stage when the	   +received frame is an erroneous frame.						   
          integer             signed, big endian          
   0   
       ����          
   set   
          NONE          
@�������   
          7                       pk_field      d  ^@��          
   Data Packet ID   
       
      4Stores the id of the data packet (MSDU) in service.	   
          integer             signed, big endian          
   0   
       ����             set             NONE          
@������   
          8                       pk_field   