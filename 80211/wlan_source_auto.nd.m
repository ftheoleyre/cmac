MIL_3_Tfile_Hdr_ 81A 73B modeler 6 43F3330E 443BFC9A 1D ares-theo-1 ftheoley 0 0 none none 0 0 none 1E795671 3974 0 0 0 0                                                                                                                                                                                                                                                                                                                                                                                                       ЋЭg      8   L     $  (  Ў  	ш  R  *О  *Ц  6Г  6З      node   WLANџџџџ   wireless_PDAџџџџџџџџ   Wireless LAN station    6   General Node Functions:    -----------------------        The wireless station node model    /represents an IEEE802.11 wireless LAN station.    %The node model consists of following    
processes:       *1. The MAC layer which has a wireless lan    !mac process model with following    attributes:       MAC address -- station address       *Fragmentation Threshold --- based on this    (threshold station decides wether or not    "to send data packets in fragments.       *Rts threshold --- based on this threshold    (station decides wether Rts/Cts exchange    *is needed for every data transmission.           "The wireless LAN MAC layer has an    "interface with higher layer which    &receives packet from higher layer and    "generates random address for them.           2. Wireless LAN interface       #This process model is an interface    )between MAC layer and higher layer.  The    &function of this process is to accept    'packets from higher layer and generate    &random destination address for them.     )This information is then sent to the MAC    layer.            3. Wireless LAN receiver       *This is a wireless receiver which accepts    (any incomming packets from the physical    &layer and pass it to the wireless MAC    process.           4. Wireless LAN transmitter       %This is a wireless transmitter which    #receives packet from MAC layer and    $transmits it to the physical medium.                        Wireless LAN MAC Address      wireless_lan_mac.Addressџџџџ    џџџџ           џџџџ          џџџџ          џџџџ                        altitude      altitudeџџџџ   џџџџ               џџџџ              џџџџ              џџџџ                        phase      phaseџџџџ   џџџџ               џџџџ              џџџџ              џџџџ                        
TIM source            none      Wireless LAN MAC Address                 Auto Assigned      altitude         
@Y      џџџџ   
   altitude modeling            relative to subnet-platform      	condition         
   џџџџ   
   financial cost            0.00      financial cost            0.00      financial cost            0.00      financial cost            0.00      financial cost            0.00      phase                 џџџџ      priority              џџџџ      user id              џџџџ                 Ш   Ш          
   mac   
       
   cmac_process   
          	processor                   Address    џџџ   
    џџџџ   
      Transmission Power   џџџ   
?6тыC-џџџџ   
      Wireless LAN Parameters   џџџ  
            count    џџџ   
   џџџџ   
      list   	џџџ   
            	Data Rate   џџџ   
Adћ       11 Mbps   
      Channel Settings   џџџ  
            count    џџџ   
   џџџџ   
      list   	џџџ   
          
  
   
  
       Ю   Ш   l          
   wlan_mac_intf   
       
   wifi_interface_auto   
          	processor                   Type of Service    џџџ   
       Best Effort (0)   
   	   л   l  $          
   wlan_port_rx0   
      
            count    џџџ   
   џџџџ   
      list   	џџџ   
            	data rate   џџџ   
Adћ    џџџџ   
      packet formats   џџџ   
   all formatted   
      	bandwidth   џџџ   
@@     џџџџ   
      min frequency   џџџ   
@ЂФ     џџџџ   
      spreading code   џџџ   
дВI­%У}џџџџ   
      processing gain   џџџ   
дВI­%У}џџџџ   
   
  
          bpsk          ?№      џџџџ          
        џџџџ   
          
dra_ragain          
   wlan_alpha_2_power   
          dra_bkgnoise             
dra_inoise             dra_snr             dra_ber          
   	dra_error   
       
   wlan_ecc   
          ra_rx                       nd_radio_receiver         reception end time   џџџ   
           0.0   
џџџџ      sec              џџџџ              џџџџ              џџџџ         0.0           џџџџ         !THIS ATTRIBUTE SHOULD NOT BE SET    TO ANY VALUE EXCEPT 0.0. This    "attribute is used by the pipeline     stages to determine whether the    receiver is busy or not. The    value of the attribute will be    updated by the pipeline stages    dynamically during the    simulation. The value will    "indicate when the receiver became    idle or when it is expected to    become idle.      ф  $  $          
   wlan_port_tx0   
      
            count    џџџ   
   џџџџ   
      list   	џџџ   
            	data rate   џџџ   
Adћ    џџџџ   
      packet formats   џџџ   
   all formatted   
      	bandwidth   џџџ   
@@     џџџџ   
      min frequency   џџџ   
@ЂФ     џџџџ   
      spreading code   џџџ   
дВI­%У}џџџџ   
      power   џџџ   
?PbMвёЉќџџџџ   
   
  
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
          ra_tx                       nd_radio_transmitter          х   l   >          
   source   
       
   bursty_source_perso   
          	processor                   Packet Interarrival Arg1   џџџ   	?№      џџџџ   	      Start Time Packet Generation   џџџ   	        џџџџ   	   	   ш     $          
   rx_busy_tone   
      
            count    џџџ   
   џџџџ   
      list   	џџџ   
            	data rate   џџџ   
@I      џџџџ   
      packet formats   џџџ   
   unformatted   
      	bandwidth   џџџ   
?tzсGЎ{џџџџ   
      min frequency   џџџ   
@       џџџџ   
      spreading code   џџџ   
дВI­%У}џџџџ   
      processing gain   џџџ   
дВI­%У}џџџџ   
   
  
       
   bpsk   
       ?№      џџџџ          
        џџџџ   
          
dra_ragain          
   dra_power_alpha   
          dra_bkgnoise             
dra_inoise             dra_snr             dra_ber          
   	dra_error   
       
   dpt_ecc   
          ra_rx                       nd_radio_receiver         reception end time   џџџ   
           0.0   
џџџџ      sec              џџџџ              џџџџ              џџџџ         0.0           џџџџ         !THIS ATTRIBUTE SHOULD NOT BE SET    TO ANY VALUE EXCEPT 0.0. This    "attribute is used by the pipeline     stages to determine whether the    receiver is busy or not. The    value of the attribute will be    updated by the pipeline stages    dynamically during the    simulation. The value will    "indicate when the receiver became    idle or when it is expected to    become idle.      щ    $          
   tx_busy_tone   
      
            count    џџџ   
   џџџџ   
      list   	џџџ   .            	data rate   џџџ   .@шj     џџџџ   .      packet formats   џџџ   
   unformatted   
      	bandwidth   џџџ   
?zсGЎ{џџџџ   
      min frequency   џџџ   
@       џџџџ   
      spreading code   џџџ   
дВI­%У}џџџџ   
      power   џџџ   
?PbMвёЉќџџџџ   
   .  
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
          ra_tx                       nd_radio_transmitter                     ф      Ы   д    #   
          strm_3          
   src stream [0]   
       
   dest stream [0]   
       
   џџџџ   
               џџџџ             џџџџ                                           nd_packet_stream             л          x  ,   Ъ   д   
          strm_4          
   src stream [0]   
       
   dest stream [0]   
       
   џџџџ   
               џџџџ          
   0џџџџ   
                                        nd_packet_stream             Ю          Ь   x   Ь   Л   
          strm_12          
   src stream [0]   
       
   dest stream [5]   
       
   џџџџ   
               џџџџ             џџџџ                                           nd_packet_stream                 Ю      С   Л   С   x   
          strm_13          
   src stream [5]   
       
   dest stream [0]   
       
   џџџџ   
               џџџџ          
   0џџџџ   
                                        nd_packet_stream             х   Ю      x   =   М   =   Ц   b   
          strm_10          
   0   
       
   dest stream [1]   
       
   џџџџ   
               џџџџ          
   џџџџ   
                                        nd_packet_stream            л          r     У   Я          
   rxstat_11Mbps   
          channel [0]          
   radio receiver.received power   
          
instat [0]          
   џџџџ   
               џџџџ          
    џџџџ   
       
    џџџџ   
       
    џџџџ   
       
    џџџџ   
       
        џџџџ   
       
=4ЁвW1Рџџџџ   
       
   џџџџ   
                                        nd_statistic_wire            ф         #  "   Ъ   Ц          
   txstat_11Mbps   
       
   channel [0]   
       
   radio transmitter.busy   
       
   
instat [1]   
       
   џџџџ   
               џџџџ          
    џџџџ   
       
   џџџџ   
       
    џџџџ   
       
    џџџџ   
       
дВI­%У}џџџџ   
       
дВI­%У}џџџџ   
       
   џџџџ   
                                        nd_statistic_wire            ш            !   Р   У          
   rxstat_11Mbps_0   
          channel [0]          
   radio receiver.received power   
       
   
instat [2]   
       
   џџџџ   
               џџџџ          
    џџџџ   
       
    џџџџ   
       
    џџџџ   
       
    џџџџ   
       
        џџџџ   
       
=4ЁвW1Рџџџџ   
       
   џџџџ   
                                        nd_statistic_wire             ш            ,   С   Щ   
          strm_6          
   0   
       
   1   
       
   џџџџ   
               џџџџ          
   0џџџџ   
                                        nd_packet_stream             щ         w  &   Э   П          
   txstat_11Mbps_1   
          channel [0]          
   radio transmitter.busy   
       
   
instat [3]   
       
   џџџџ   
               џџџџ          
    џџџџ   
       
   џџџџ   
       
    џџџџ   
       
    џџџџ   
       
дВI­%У}џџџџ   
       
дВI­%У}џџџџ   
       
   џџџџ   
                                        nd_statistic_wire          !       щ      в   Л  ~  !   
          strm_15             1             0             џџџџ                  џџџџ             џџџџ                                           nd_packet_stream      ъ   "      &wireless_lan_mac.Backoff Slots (slots)   Backoff Slots (slots)           Wireless Lan   bucket/default total/sum   linear   Wireless Lan   0wireless_lan_mac.Control Traffic Rcvd (bits/sec)   Control Traffic Rcvd (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   0wireless_lan_mac.Control Traffic Sent (bits/sec)   Control Traffic Sent (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   -wireless_lan_mac.Data Traffic Rcvd (bits/sec)   Data Traffic Rcvd (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   -wireless_lan_mac.Data Traffic Sent (bits/sec)   Data Traffic Sent (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   0wireless_lan_mac.Data Traffic Sent (packets/sec)   Data Traffic Sent (packets/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   *wireless_lan_mac.Channel Reservation (sec)   Channel Reservation (sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   3wireless_lan_mac.Control Traffic Rcvd (packets/sec)   "Control Traffic Rcvd (packets/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   3wireless_lan_mac.Control Traffic Sent (packets/sec)   "Control Traffic Sent (packets/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   0wireless_lan_mac.Data Traffic Rcvd (packets/sec)   Data Traffic Rcvd (packets/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan    wireless_lan_mac.Load (bits/sec)   Load (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   wireless_lan_mac.Load (packets)   Load (packets)           Wireless Lan   bucket/default total/sum   linear   Wireless Lan   &wireless_lan_mac.Throughput (bits/sec)   Throughput (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   0wireless_lan_mac.Dropped Data Packets (bits/sec)   Dropped Data Packets (bits/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   2wireless_lan_mac.Retransmission Attempts (packets)   !Retransmission Attempts (packets)           Wireless Lan   bucket/default total/sum   linear   Wireless Lan   3wireless_lan_mac.Dropped Data Packets (packets/sec)   "Dropped Data Packets (packets/sec)           Wireless Lan   bucket/default total/sum_time   linear   Wireless Lan   )wireless_lan_mac.Media Access Delay (sec)   Media Access Delay (sec)           Wireless Lan    bucket/default total/sample mean   linear   Wireless Lan   )wireless_lan_mac.Hld Queue Size (packets)   Hld Queue Size (packets)           Wireless Lan   !bucket/default total/time average   linear   Wireless Lan   wireless_lan_mac.Delay (sec)   Delay (sec)           Wireless Lan    bucket/default total/sample mean   linear   Wireless Lan          machine type       station   Model Attributes      8.1.A-Feb18-2002                interface type      
IEEE802.11           wlan_port_tx<n>   wlan_port_rx<n>           