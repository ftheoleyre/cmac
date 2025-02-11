MIL_3_Tfile_Hdr_ 120A 94B modeler 40 441197D6 4B5D977A 9 lefkada-laptop theoleyre 0 0 none none 0 0 none 84C7839A 15E9 0 0 0 0 0 0 11ab 5                                                                                                                                                                                                                                                                                                                                                                                                    ����             0x0             NONE                              
   (   (              
   PLCP_Preamble_and_Header   
       
      >This field represents the physical layer overhead added to the   8WLAN MAC data packets, namely the PLCP Preamble and PLCP   Header fields.       7The size of the PLCP overhead depends on physical layer   ?technology that is used for transmissions. Infra-red technology   <has the shortest overhead, which is 57 microseconds (this is   >equivalent of 57 bits since PLCP fields are transmitted at the   Alowest data rate, which is 1 Mbps). If another technology is used   :then the additional size is added to the total size of the   packet as bulk size.       =As mentioned above, the PLCP fields are transmitted at 1 Mbps   =regardless of the data rate that will be used for the rest of   >the packet (one exception: when infra-red is used, also a part   ?of the PLCP header is transmitted at the data rate that will be   9used for the rest of the packet). To model this behaviour   >accurately, more bits are added to the bulk size of the packet   :if the data rate that will be used for the transmission is   =different than 1 Mbps. For instance, a MAC that operates over   =FHSS technology, whose PLCP overhead is 128 microseconds, and   ?chooses its transmission rate as 11 Mbps will set the bulk size   of each data frame to       11 * 128 - 57 = 1351 bits       ?before the transmissions, so that the transmission delay of the   @bulk together with this field will take exactly 128 microseconds   as expexted.   
       
   information   
          signed, big endian          
   57   
       ����             set             NONE          
@������   
       J   1   J             (   Z              J   source   J                     
   integer   
          signed, big endian          J   32   J       ����          J   set   J          NONE          
@�������   
       J   2   J             (   x              J   destination   J                     
   integer   
          signed, big endian          J   32   J       ����          J   set   J          NONE          
@�������   
       J   3   J             (   �              J   type   J                     
   integer   
          signed, big endian          
   4   
       ����          J   set   J          NONE          
@�������   
       J   4   J             d   �              J   nav_duration   J                     
   integer   
          signed, big endian          
   16   
       ����          J   set   J          NONE          
@�������   
       J   5   J             (  6              
   FCS   
       
      'Frame check sum (currently not modeled)   
       
   information   
          signed, big endian          
   32   
       ����             set             NONE          
@�������   
       J   6   J             (  ^              
   Accept   
       
      <This packet field is enabled by the pipeline stage when the	   +received frame is an erroneous frame.						   
          integer             signed, big endian          
   0   
       ����          
   set   
          NONE          
@�������   
       J   7   J             d  ^              
   Data Packet ID   
       
      4Stores the id of the data packet (MSDU) in service.	   
          integer             signed, big endian          
   0   
       ����             set             NONE          
@������   
       J   8   J             �  ^              J   Tx Data Rate   J       
      DThis field conveys the actual transmission data rate applied to this   Epacket (in bits per second). Since it is added for modeling purposes,   Gits size is set to 0 bits. The value of this field is needed to compute   Gthe transmission delay accurately, because all the supported data rates   Juse the same channel of the radio tranceivers (hence the data rate setting   0of the channel cannot be used for that purpose).   
       
   floating point   
          
big endian          
   0   
       
   	1000000.0   
          set             NONE          
@�`����   
       J   9   J             �  ^              J   PHY Info   J       
      AThis field conveys the physical layer technology information that   Ais used for the transmission of the packet. The size of the field   Ais set to 0 bits, since this is a non-standard WLAN packet field,   =which is added for modeling purposes. The set of valid values   ?for this field are defined in the wlan_support.h header file as   also shown below:        typedef enum WlanT_Phy_Char_Code   	{   	WlanC_Frequency_Hopping,			   	WlanC_Direct_Sequence,				   	WlanC_Infra_Red,   	WlanC_OFDM_11a,   	WlanC_ERP_11g   	} WlanT_Phy_Char_Code;       
          integer             signed, big endian          
   0   
       
   1   
          set             NONE          
@�B����   
       J   10   J          