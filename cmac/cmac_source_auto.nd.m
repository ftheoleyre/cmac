MIL_3_Tfile_Hdr_ 81A 73B modeler 6 440DDD3F 44158CFB E ares-theo-1 ftheoley 0 0 none none 0 0 none 7335C70C 2C63 0 0 0 0                                                                                                                                                                                                                                                                                                                                                                                                        Ф═gЅ      8   L   Ђ  $  (  «  	У  ]  Г  х  )б  )д      node   WLAN       wireless_PDA           Wireless LAN station    6   General Node Functions:    -----------------------        The wireless station node model    /represents an IEEE802.11 wireless LAN station.    %The node model consists of following    
processes:       *1. The MAC layer which has a wireless lan    !mac process model with following    attributes:       MAC address -- station address       *Fragmentation Threshold --- based on this    (threshold station decides wether or not    "to send data packets in fragments.       *Rts threshold --- based on this threshold    (station decides wether Rts/Cts exchange    *is needed for every data transmission.           "The wireless LAN MAC layer has an    "interface with higher layer which    &receives packet from higher layer and    "generates random address for them.           2. Wireless LAN interface       #This process model is an interface    )between MAC layer and higher layer.  The    &function of this process is to accept    'packets from higher layer and generate    &random destination address for them.     )This information is then sent to the MAC    layer.            3. Wireless LAN receiver       *This is a wireless receiver which accepts    (any incomming packets from the physical    &layer and pass it to the wireless MAC    process.           4. Wireless LAN transmitter       %This is a wireless transmitter which    #receives packet from MAC layer and    $transmits it to the physical medium.                        Wireless LAN MAC Address      wireless_lan_mac.Address                                                                               altitude      altitude                                                                                          phase      phase                                                                                          
TIM source            none      Wireless LAN MAC Address                 Auto Assigned      altitude         
@Y             
   altitude modeling            relative to subnet-platform      	condition         
          
   financial cost            0.00      financial cost            0.00      financial cost            0.00      financial cost            0.00      financial cost            0.00      phase                           priority                        user id                                   ╚   ╚          
   mac   
       
   cmac_process   
          	processor                   Wireless LAN MAC Address          	   ■   From MAC Intf   	      Wireless LAN Parameters      ђ  
            count          
          
      list   	      
            	Data Rate         
Adч       11 Mbps   
      Channel Settings      ђ  
            count          
          
      list   	      
          
ђ  
   
ђ  
       ╬   ╚   l          
   wlan_mac_intf   
       
   wlan_mac_interface_auto   
          	processor                   Type of Service          
       Best Effort (0)   
   	   █   l  $          
   wlan_port_rx0   
    ђ  
            count          
          
      list   	      .            	data rate         
Adч           
      packet formats         .   all formatted   .      	bandwidth         
@Ј@            
      min frequency         
@б─            
      spreading code         
н▓IГ%ћ├}       
      processing gain         
н▓IГ%ћ├}       
   .ђ  
          bpsk          ?­                    
               
          
dra_ragain          
   wlan_alpha35_power   
          dra_bkgnoise             
dra_inoise             dra_snr             dra_ber          
   	dra_error   
       
   wlan_ecc   
          ra_rx                       nd_radio_receiver         reception end time         
           0.0   
          sec                                                               0.0                        !THIS ATTRIBUTE SHOULD NOT BE SET    TO ANY VALUE EXCEPT 0.0. This    "attribute is used by the pipeline     stages to determine whether the    receiver is busy or not. The    value of the attribute will be    updated by the pipeline stages    dynamically during the    simulation. The value will    "indicate when the receiver became    idle or when it is expected to    become idle.      С  $  $          
   wlan_port_tx0   
    ђ  
            count          
          
      list   	      .            	data rate         
Adч           
      packet formats         .   all formatted   .      	bandwidth         
@Ј@            
      min frequency         
@б─            
      spreading code         
н▓IГ%ћ├}       
      power         
?PbMмыЕЧ       
   .ђ  
          bpsk          
   wlan_rxgroup   
       
   	dra_txdel   
       
   dra_closure_all   
       
   wlan_chanmatch   
          
dra_tagain          
   wlan_propdel   
          ra_tx                       nd_radio_transmitter          т   l   >          
   source   
       
   bursty_source_perso   
          	processor                   Packet Interarrival Arg1         	?­             	      Start Time Packet Generation         	@N             	                  С      ╦   н    #   
          strm_3          
   src stream [0]   
       
   dest stream [0]   
       
          
                                                                               nd_packet_stream             █          x  ,   ╩   н   
          strm_4          
   src stream [0]   
       
   dest stream [0]   
       
          
                             
   0       
                                        nd_packet_stream             ╬          ╠   x   ╠   ╗   
          strm_12          
   src stream [1]   
       
   dest stream [5]   
       
          
                                                                               nd_packet_stream                 ╬      ┴   ╗   ┴   x   
          strm_13          
   src stream [5]   
       
   dest stream [1]   
       
          
                             
   0       
                                        nd_packet_stream            С         "    "   ╠   н   ╩          
   txstat_11Mbps   
       
   channel [0]   
       
   radio transmitter.busy   
       
   
instat [1]   
       
          
                             
           
       
          
       
           
       
           
       
н▓IГ%ћ├}       
       
н▓IГ%ћ├}       
       
          
                                        nd_statistic_wire            █          l     l   ╠   ╗   ╠          
   rxstat_11Mbps   
          channel [0]          
   radio receiver.received power   
          
instat [0]          
          
                             
           
       
           
       
           
       
           
       
               
       
=4АмW1└ў       
       
          
                                        nd_statistic_wire             т   ╬      x   =   ╝   =   к   b   
          strm_10          
   0   
       
   0   
       
          
                             
          
                                        nd_packet_stream      Т         &wireless_lan_mac.Backoff Slots (slots)   Backoff Slots (slots)           Wireless Lan   bucket/default total/sum   linear   Wireless Lan   0wireless_lan_mac.Control Traffic Rcvd (bits/sec)   Control Traffic Rcvd (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   0wireless_lan_mac.Control Traffic Sent (bits/sec)   Control Traffic Sent (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   -wireless_lan_mac.Data Traffic Rcvd (bits/sec)   Data Traffic Rcvd (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   -wireless_lan_mac.Data Traffic Sent (bits/sec)   Data Traffic Sent (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   0wireless_lan_mac.Data Traffic Sent (packets/sec)   Data Traffic Sent (packets/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   *wireless_lan_mac.Channel Reservation (sec)   Channel Reservation (sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   3wireless_lan_mac.Control Traffic Rcvd (packets/sec)   "Control Traffic Rcvd (packets/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   3wireless_lan_mac.Control Traffic Sent (packets/sec)   "Control Traffic Sent (packets/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   0wireless_lan_mac.Data Traffic Rcvd (packets/sec)   Data Traffic Rcvd (packets/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan    wireless_lan_mac.Load (bits/sec)   Load (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   wireless_lan_mac.Load (packets)   Load (packets)           Wireless Lan   bucket/default total/sum   linear   Wireless Lan   &wireless_lan_mac.Throughput (bits/sec)   Throughput (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   0wireless_lan_mac.Dropped Data Packets (bits/sec)   Dropped Data Packets (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   2wireless_lan_mac.Retransmission Attempts (packets)   !Retransmission Attempts (packets)           Wireless Lan   bucket/default total/sum   linear   Wireless Lan   3wireless_lan_mac.Dropped Data Packets (packets/sec)   "Dropped Data Packets (packets/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   )wireless_lan_mac.Media Access Delay (sec)   Media Access Delay (sec)           Wireless Lan    bucket/default total/sample mean   linear   Wireless Lan   )wireless_lan_mac.Hld Queue Size (packets)   Hld Queue Size (packets)           Wireless Lan   !bucket/default total/time average   linear   Wireless Lan   wireless_lan_mac.Delay (sec)   Delay (sec)           Wireless Lan    bucket/default total/sample mean   linear   Wireless Lan          machine type       station   Model Attributes      8.1.A-Feb18-2002                interface type      
IEEE802.11           wlan_port_tx<n>   wlan_port_rx<n>           