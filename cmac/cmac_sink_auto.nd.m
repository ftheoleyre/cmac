MIL_3_Tfile_Hdr_ 81A 73B modeler 6 440DDD47 4483EB3D 3A ares-theo-1 ftheoley 0 0 none none 0 0 none C687DAC4 4056 0 0 0 0                                                                                                                                                                                                                                                                                                                                                                                                       Ф═gЅ      8   L   }     $  >  
║   б  1а  1е  =Ћ  =Ў      node   WLAN       terminal           Wireless LAN station    6   General Node Functions:    -----------------------        The wireless station node model    /represents an IEEE802.11 wireless LAN station.    %The node model consists of following    
processes:       *1. The MAC layer which has a wireless lan    !mac process model with following    attributes:       MAC address -- station address       *Fragmentation Threshold --- based on this    (threshold station decides wether or not    "to send data packets in fragments.       *Rts threshold --- based on this threshold    (station decides wether Rts/Cts exchange    *is needed for every data transmission.           "The wireless LAN MAC layer has an    "interface with higher layer which    &receives packet from higher layer and    "generates random address for them.           2. Wireless LAN interface       #This process model is an interface    )between MAC layer and higher layer.  The    &function of this process is to accept    'packets from higher layer and generate    &random destination address for them.     )This information is then sent to the MAC    layer.            3. Wireless LAN receiver       *This is a wireless receiver which accepts    (any incomming packets from the physical    &layer and pass it to the wireless MAC    process.           4. Wireless LAN transmitter       %This is a wireless transmitter which    #receives packet from MAC layer and    $transmits it to the physical medium.                        Traffic Type of Service      wlan_mac_intf.Type of Service                                                                                Wireless LAN MAC Address      wireless_lan_mac.Address                                                                               altitude      altitude                                                                                          phase      phase                                                                                          
TIM source            none      Traffic Type of Service                 Best Effort (0)      Wireless LAN MAC Address                 Auto Assigned      altitude         
@Y             
   altitude modeling            relative to subnet-platform      	condition         
          
   financial cost            0.00      financial cost            0.00      financial cost            0.00      financial cost            0.00      financial cost            0.00      phase                           priority                        user id                        	           Ш   ╚          
   mac   
       
   cmac_process   
          	processor                   Tx Power         
?PbMмыЕЧ       
      Wireless LAN Parameters      ђ  
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
       ╠  $   >          
   sink   
       
   
sink_perso   
          	processor                    ╬   Ш   l          
   wlan_mac_intf   
       
   wifi_interface_auto   
          	processor                	   █   џ  $          
   wlan_port_rx0   
    ђ  
            count          
          
      list   	      
            	data rate         
Adч           
      packet formats         
   all formatted   
      	bandwidth         
@Ј@            
      min frequency         
@б─            
      spreading code         
н▓IГ%ћ├}       
   
ђ  
          bpsk          ?­                    
               
          
dra_ragain          
   wlan_alpha_power   
          dra_bkgnoise             
dra_inoise             dra_snr             dra_ber          
   	dra_error   
       
   wlan_ecc   
          ra_rx                       nd_radio_receiver         reception end time         
           0.0   
          sec                                                               0.0                        !THIS ATTRIBUTE SHOULD NOT BE SET    TO ANY VALUE EXCEPT 0.0. This    "attribute is used by the pipeline     stages to determine whether the    receiver is busy or not. The    value of the attribute will be    updated by the pipeline stages    dynamically during the    simulation. The value will    "indicate when the receiver became    idle or when it is expected to    become idle.      С  R  $          
   wlan_port_tx0   
    ђ  
            count          
          
      list   	      
            	data rate         
Adч           
      packet formats         
   all formatted   
      	bandwidth         
@Ј@            
      min frequency         
@б─            
      spreading code         
н▓IГ%ћ├}       
      power         
?PbMмыЕЧ       
   
ђ  
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
          ra_tx                       nd_radio_transmitter      
   Ы  
   џ   
          direc_narrow          
   tx_direc   
       
               
       
               
       
               
       
?­             
                                antenna                       
nd_antenna         з  «  $          
   tx_busy_tone   
    ђ  
            count          
          
      list   	      .            	data rate         .@Уj            .      packet formats         
   unformatted   
      	bandwidth         
?ёzрG«{       
      min frequency         
@              
      spreading code         
н▓IГ%ћ├}       
      power         
?PbMмыЕЧ       
   .ђ  
          bpsk          
   dra_rxgroup   
       
   	dra_txdel   
       
   dra_closure_all   
       
   dra_chanmatch   
          
dra_tagain          
   dra_propdel   
          ra_tx                       nd_radio_transmitter      	   ш   >  $          
   rx_busy_tone   
    ђ  
            count          
          
      list   	      
            	data rate         
@I             
      packet formats         
   unformatted   
      	bandwidth         
?tzрG«{       
      min frequency         
@              
      spreading code         
н▓IГ%ћ├}       
      processing gain         
н▓IГ%ћ├}       
   
ђ  
       
   bpsk   
       ?­                    
               
          
dra_ragain          
   dra_power_alpha   
          dra_bkgnoise             
dra_inoise             dra_snr             dra_ber          
   	dra_error   
       
   dpt_ecc   
          ra_rx                       nd_radio_receiver         reception end time         
           0.0   
          sec                                                               0.0                        !THIS ATTRIBUTE SHOULD NOT BE SET    TO ANY VALUE EXCEPT 0.0. This    "attribute is used by the pipeline     stages to determine whether the    receiver is busy or not. The    value of the attribute will be    updated by the pipeline stages    dynamically during the    simulation. The value will    "indicate when the receiver became    idle or when it is expected to    become idle.      Ш  «   џ          
   tx_sync_direc   
    ђ  
            count          
          
      list   	      
            	data rate         
Adч           
      packet formats         
   all formatted   
      	bandwidth         
@Ј@            
      min frequency         
@б─            
      spreading code         
н▓IГ%ћ├}       
      power         
?PbMмыЕЧ       
   
ђ  
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
          ra_tx                       nd_radio_transmitter                     С      щ   н  E  #   
          strm_3          
   src stream [0]   
       
   dest stream [0]   
       
          
                                                                               nd_packet_stream             █          д  ,   Э   н   
          strm_4          
   src stream [0]   
       
   dest stream [0]   
       
          
                             
   0       
                                        nd_packet_stream             ╬   ╠      Щ   _   Щ   >     >   
          strm_11          
   src stream [1]   
       
   dest stream [0]   
       
          
                             
   0       
                                        nd_packet_stream             ╬          Щ   x   Щ   ╗   
          strm_12          
   src stream [0]   
       
   dest stream [5]   
       
          
                                                                               nd_packet_stream                 ╬      №   ╗   №   x   
          strm_13          
   src stream [5]   
       
   dest stream [0]   
       
          
                             
   0       
                                        nd_packet_stream         ,   С         R    ,   Ш   ш   ┬          
   txstat_11Mbps   
          channel [0]          
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
                                        nd_statistic_wire          6   ш          J  ,   №   к   
          strm_6          
   0   
       
   1   
       
          
                             
   0       
                                        nd_packet_stream         7   █          б     ь   ═          
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
                                        nd_statistic_wire         8   ш          E  !   Ж   ┴          
   rxstat_11Mbps_0   
          channel [0]          
   radio receiver.received power   
       
   
instat [2]   
       
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
                                        nd_statistic_wire         9   з         Ц  &   Э   ─          
   txstat_11Mbps_1   
          channel [0]          
   radio transmitter.busy   
       
   
instat [3]   
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
                                        nd_statistic_wire          :       з      Щ   Й  г  !   
          strm_15          
   src stream [1]   
          0                                                                                                   nd_packet_stream         ;   Ш         «   Њ   Ш   ┴          
   txstat_11Mbps_0   
          channel [0]          
   radio transmitter.busy   
       
   
instat [4]   
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
                                        nd_statistic_wire          <       Ш      Ш   ╚  А   Ў   
          strm_5          
   2   
       
   0   
       
          
                                                                               nd_packet_stream          =   Ш   Ы     ░   ћ     Ќ   
          strm_14             0             0                                                                                                   nd_packet_stream      э   >      &wireless_lan_mac.Backoff Slots (slots)   Backoff Slots (slots)           Wireless Lan   bucket/default total/sum   linear   Wireless Lan   0wireless_lan_mac.Control Traffic Rcvd (bits/sec)   Control Traffic Rcvd (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   0wireless_lan_mac.Control Traffic Sent (bits/sec)   Control Traffic Sent (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   -wireless_lan_mac.Data Traffic Rcvd (bits/sec)   Data Traffic Rcvd (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   -wireless_lan_mac.Data Traffic Sent (bits/sec)   Data Traffic Sent (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   0wireless_lan_mac.Data Traffic Sent (packets/sec)   Data Traffic Sent (packets/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   *wireless_lan_mac.Channel Reservation (sec)   Channel Reservation (sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   3wireless_lan_mac.Control Traffic Rcvd (packets/sec)   "Control Traffic Rcvd (packets/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   3wireless_lan_mac.Control Traffic Sent (packets/sec)   "Control Traffic Sent (packets/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   0wireless_lan_mac.Data Traffic Rcvd (packets/sec)   Data Traffic Rcvd (packets/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan    wireless_lan_mac.Load (bits/sec)   Load (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   wireless_lan_mac.Load (packets)   Load (packets)           Wireless Lan   bucket/default total/sum   linear   Wireless Lan   &wireless_lan_mac.Throughput (bits/sec)   Throughput (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   0wireless_lan_mac.Dropped Data Packets (bits/sec)   Dropped Data Packets (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   2wireless_lan_mac.Retransmission Attempts (packets)   !Retransmission Attempts (packets)           Wireless Lan   bucket/default total/sum   linear   Wireless Lan   3wireless_lan_mac.Dropped Data Packets (packets/sec)   "Dropped Data Packets (packets/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   )wireless_lan_mac.Media Access Delay (sec)   Media Access Delay (sec)           Wireless Lan    bucket/default total/sample mean   linear   Wireless Lan   )wireless_lan_mac.Hld Queue Size (packets)   Hld Queue Size (packets)           Wireless Lan   !bucket/default total/time average   linear   Wireless Lan   wireless_lan_mac.Delay (sec)   Delay (sec)           Wireless Lan    bucket/default total/sample mean   linear   Wireless Lan          machine type       station   Model Attributes      8.1.A-Feb18-2002                interface type      
IEEE802.11           wlan_port_tx<n>   wlan_port_rx<n>           