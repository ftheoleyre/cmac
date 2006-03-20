/* Process model C form file: wlan_mac_wifib.pr.c */
/* Portions of this file copyright 1992-2002 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
static const char wlan_mac_wifib_pr_c [] = "MIL_3_Tfile_Hdr_ 81A 30A modeler 7 441B48E8 441B48E8 1 ares-theo-1 ftheoley 0 0 none none 0 0 none 0 0 0 0 0 0                                                                                                                                                                                                                                                                                                                                                                                                                 ";
#include <string.h>



/* OPNET system definitions */
#include <opnet.h>

#if defined (__cplusplus)
extern "C" {
#endif
FSM_EXT_DECS
#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif


/* Header Block */

/* Include files.		*/
#include <math.h>
#include <string.h>
#include "oms_pr.h"
#include "oms_tan.h"
#include "oms_bgutil.h"
#include "wlan_support.h"
#include "oms_auto_addr_support.h"
#include "oms_dist_support.h"
#include "bridge_header.h"	




/* Incoming statistics and stream wires.							*/
#define 	TRANSMITTER_BUSY_INSTAT			4
#define		LOW_LAYER_INPUT_STREAM_CH4		3
#define 	LOW_LAYER_OUT_STREAM_CH1		0
#define 	LOW_LAYER_OUT_STREAM_CH2		1
#define 	LOW_LAYER_OUT_STREAM_CH3		2
#define 	LOW_LAYER_OUT_STREAM_CH4		3

/* Flags to load different variables based on attribute settings.	*/
#define		WLAN_AP						1
#define		WLAN_STA					0

/* Flags to indicate the medium access mode (PCF/DCF).				*/
#define		PCF_ACTIVE					1
#define		PCF_INACTIVE				0

/* Special value indicating BSS identification is not used.			*/
#define		WLAN_BSSID_NOT_USED			-1

/* Special value indicating radio tranceiver frequencies are set	*/
/* based on the BSS identification.									*/
#define 	WLAN_BSS_BASED_FREQ_USED	-1

/* Special value indicating that the number of back-off slots are	*/
/* not determined yet.												*/
#define		BACKOFF_SLOTS_UNSET			-1.0

/* Define a small value (= 1 psec), which will be used to recover	*/
/* from double arithmetic precision losts while doing time related	*/
/* precision sensitive computations.								*/
#define		PRECISION_RECOVERY			0.000000000001

/* Define the three possible values that a global variable takes				*/
/* to check and ensure that all the MAC modules are either BSS ID				*/
/* based or purely subnet based.The variable is intialized to					*/
/* Not_Set and the first wireless MAC process sets it to either					*/
/* Entire_Subnet (if BSS_Identifier is Not Used) or Bssid_Subnet				*/
/* (if the BSS_Identifier is set to some value other than Not					*/
/* Used).																		*/						  
typedef enum WlanT_Bssid_Approach
	{
	WlanC_Not_Set,						/* Type of network not set				*/
	WlanC_Entire_Subnet,				/* The network is a pure subnet			*/
	WlanC_Bss_Divided_Subnet			/* The network is a BSS based subnet	*/
	} WlanT_Bss_Identification_Approach;
 
/* Global variable to keep note of the nature of the subnet.					*/
/* This variable is intialized to not set.										*/
WlanT_Bss_Identification_Approach bss_id_type = WlanC_Not_Set;

/* Define interrupt codes for generating handling interrupts					*/
/* indicating changes in deference, frame timeout which infers         			*/
/* that the collision has occurred, random backoff and transmission 			*/
/* completion by the physical layer (self interrupts).							*/
typedef enum WlanT_Mac_Intrpt_Code
	{	
	WlanC_Deference_Off,  	/* Deference before frame transmission 				*/
	WlanC_Frame_Timeout,	/* No frame rcvd in set duration (infer collision)	*/
	WlanC_Backoff_Elapsed,  /* Backoff done before frame transmission			*/
	WlanC_CW_Elapsed,		/* Bakcoff done after successful frame transmission	*/	
	WlanC_Beacon_Tx_Time,	/* Time to transmit beacon frame                    */
	WlanC_Cfp_End			/* End of the Contention free period */
	} WlanT_Mac_Intrpt_Code;

/* Define codes for data and managment frames use in DCF		        		*/
/* The code defined is consistent with IEEE 802.11 format			    		*/
/* There are 6 bits used to define the code and in the following        		*/
/* enumeration the first 6 bits are used in the type field of the frame.		*/
typedef enum WlanT_Mac_Frame_Type
	{
	WlanC_Rts 	 	= 0x6C, /* Rts code set into the Rts control frame	 */
    WlanC_Cts  		= 0x70, /* Cts code set into the Cts control frame	 */
	WlanC_Ack  		= 0x74, /* Ack code set into the Ack control frame	 */
 	WlanC_Data 		= 0x80, /* Data code set into the Data frame      	 */
	WlanC_None 		= 0x00, /* None type 							  	 */
	WlanC_Beac		= 0x08, /* Beacon management frame					 */
	WlanC_Cf_End 	= 0xE4,	/* Frame indicates the ens of CFP during PCF */
	WlanC_Cf_End_A 	= 0xF4, /* CFP end ack frames sent by the sta's		 */	
							/* participating in the AP's poll			 */
	WlanC_Data_Ack	= 0x18,	/* During PCF sta's can piggy back ack with  */
							/* the data frames during PCF				 */
	WlanC_Data_Poll	= 0x28, /* Send data from AP when the addressed STA	 */
							/* is permitted to transmit during the PCF   */
	WlanC_Data_A_P	= 0x38, /* Data piggybacked along with the acknowleg */
							/*-ment for the Poll frame					 */
	WlanC_Data_Null	= 0x48,	/* If the station has been polled and no ack */
							/* pending to be sent, Data_Null will be	 */
							/* transmitted								 */
	WlanC_Cf_Ack 	= 0x58,	/* If a pending ack needs to be transmitted	 */
	WlanC_Cf_Poll 	= 0x68,	/* CFP poll frame							 */
	WlanC_Cf_A_P 	= 0x78  
	} WlanT_Mac_Frame_Type;

/* Defining codes for the physical layer characteristics type	*/
typedef enum WlanT_Phy_Char_Code
	{
	WlanC_Frequency_Hopping,			
	WlanC_Direct_Sequence,				
	WlanC_Infra_Red					
	} WlanT_Phy_Char_Code;

/* Define a structure to maintain data fragments received by each 	  */
/* station for the purpose of reassembly (or defragmentation)		  */
typedef struct WlanT_Mac_Defragmentation_Buffer_Entry
	{		
	int			tx_station_address    ;/* Store the station address of transmitting station  		*/	 
	double		time_rcvd		      ;/* Store time the last fragment for this frame was received	*/ 
	Sbhandle	reassembly_buffer_ptr ;/* Store data fragments for a particular packet       		*/  		 
	} WlanT_Mac_Defragmentation_Buffer_Entry;

/* Define a structure to maintain a copy of each unique data frame      */
/* received by the station. This is done so that the station can    	*/
/* discard any additional copies of the frame received by it. 	   		*/
typedef struct WlanT_Mac_Duplicate_Buffer_Entry
	{	
	int         tx_station_address;  /* store the station address of transmitting station	*/
	int 		sequence_id		  ;  /* rcvd packet sequence id 						 	*/	
	int		    fragment_number	  ;  /* rcvd packet fragment number                      	*/	 
	} WlanT_Mac_Duplicate_Buffer_Entry;

/* This structure contains all the flags used in this process model to determine	*/
/* various conditions as mentioned in the comments for each flag					*/
typedef struct WlanT_Mac_Flags
	{
	Boolean 	data_frame_to_send; /* Flag to check when station needs to transmit.		*/ 
	Boolean     backoff_flag;  	    /* Backoff flag is set when either the collision is		*/
	                                /* inferred or the channel switched from busy to idle	*/
	Boolean		rts_sent;   		/* Flag to indicate that wether the Rts for this		*/
								    /* particular data frame is sent						*/
	Boolean		rcvd_bad_packet;	/* Flag to indicate that the received packet is bad		*/
    Boolean	    receiver_busy;		/* Set this flag if receiver busy stat is enabled		*/	
    Boolean	    transmitter_busy;	/* Set this flag if we are transmitting something.		*/	
	Boolean		wait_eifs_dur;		/* Set this flag if the station needs to wait for eifs	*/
									/* duration.											*/	
	Boolean		gateway_flag;		/* Set this flag if the station is a gateway.			*/
	Boolean		bridge_flag;		/* Set this flag if the station is a bridge				*/
	Boolean		immediate_xmt;		/* Set this flag if the new frame can be transmitted	*/
									/* without deferring.									*/
	Boolean		forced_xmt;			/* Special case: resume with transmission regradless of	*/
									/* receiver status.										*/
	Boolean		cw_required;		/* Indicates that the MAC is in contention window		*/
									/* period following a successful transmission.			*/
	Boolean		perform_cw;			/* Flag that triggers backoff process for CW period.	*/
	Boolean		nav_updated;		/* Indicates a new NAV value since the last time when	*/
									/* self interrupt is scheduled for the end of deference.*/
	Boolean		collision;			/* Set this flag if a channel became busy while another	*/
									/* one busy.											*/
	Boolean		collided_packet;	/* Set this flag to drop the next received packet		*/
									/* because of collision.								*/
	Boolean		duration_zero;		/* Set this flag if duration should be zero in next ack	*/
	Boolean		ignore_busy;		/* Set this flag if the STA should ignor reciever busy	*/
	Boolean		use_eifs;			/* Set this flag if the station needs to use eifs.		*/
	Boolean		tx_beacon;          /* Set this flag if time to send a beacon               */
	Boolean		tx_cf_end;          /* Set this flag if time to send a CF End frame         */
	Boolean		pcf_active;         /* Set this flag for AP if PCF is currently in effect	*/
	Boolean		polled;		        /* Set this flag if the station has received a poll     */
	Boolean		more_data;			/* Set this flag if must poll for more data (MSDU)		*/
	Boolean		more_frag;			/* Set this flag if must poll for more fragments		*/
	Boolean		pcf_side_traf;		/* Set this flag if the AP detects STA to STA traffic   */
	Boolean		active_poll;		/* Set this flag if an active poll is outstanding		*/
	} WlanT_Mac_Flags;

/* This structure contains the destination address to which the received */
/* data packet needs to be sent and the contents of the recieved packet  */
/* from the higher layer.												 */
typedef struct WlanT_Hld_List_Elem
	{
	double		time_rcvd;  			/* Time packet is received by the higher layer	*/
	int			destination_address; 	/* Station to which this packet needs to be sent*/
	Packet*     pkptr;				 	/* store packet contents  					  	*/
	} WlanT_Hld_List_Elem;

/**	Macros	Definition														**/
/** The data frame send flag is set whenever there is a data to be send by	**/
/** the higher layer or the response frame needs to be sent. However, in 	**/
/** either case the flag will not be set if the receiver is busy			**/
/** Frames cannot be transmitted until medium is idle. Once, the medium 	**/
/** is available then the station is eligible to transmit provided there	**/
/** is a need for backoff. Once the transmission is complete then the		**/
/** station will wait for the response provided the frame transmitted  		**/
/** requires a response (such as RTS and Data frames). If response			**/
/** is not needed then the station will defer to transmit next packet		**/

/* After receiving a stream interrupt, we need to switch states from	*/
/* idle to defer or transmit if there is a frame to transmit and the	*/
/* receiver is not busy													*/ 
#define READY_TO_TRANSMIT		((intrpt_type == OPC_INTRPT_STRM && wlan_flags->receiver_busy == OPC_BOOLINT_DISABLED && \
								  wlan_flags->data_frame_to_send == OPC_BOOLINT_ENABLED && \
								  (pcf_flag == OPC_BOOLINT_DISABLED || (wlan_flags->pcf_active == OPC_BOOLINT_DISABLED && \
								   (ap_flag == OPC_BOOLINT_ENABLED || cfp_ap_medium_control == OPC_BOOLINT_DISABLED)))) || \
								 fresp_to_send != WlanC_None || \
								 wlan_flags->polled == OPC_BOOLINT_ENABLED || \
								 wlan_flags->tx_beacon == OPC_BOOLINT_ENABLED || \
								 (wlan_flags->pcf_active == OPC_BOOLINT_ENABLED && ap_flag == OPC_BOOLINT_ENABLED))

/* When we have a frame to transmit, we move to transmit state if the	*/
/* medium was idle for at least a DIFS time, otherwise we go to defer	*/
/* state.																*/
#define MEDIUM_IS_IDLE			(((current_time - nav_duration + PRECISION_RECOVERY >= difs_time) && \
	             				  wlan_flags->receiver_busy == OPC_BOOLINT_DISABLED && \
								  (current_time - rcv_idle_time + PRECISION_RECOVERY >= difs_time) && \
								  wlan_flags->pcf_active == OPC_BOOLINT_DISABLED) || \
								 wlan_flags->forced_xmt == OPC_BOOLINT_ENABLED)

/* Change state to Defer from Frm_End, if the input buffers are not empty or a frame needs	*/
/* to be retransmitted or the station has to respond to some frame.							*/		
#define FRAME_TO_TRANSMIT		(wlan_flags->data_frame_to_send == OPC_BOOLINT_ENABLED || fresp_to_send != WlanC_None || \
								 retry_count != 0 || wlan_flags->tx_beacon == OPC_BOOLINT_ENABLED || \
								 wlan_flags->cw_required == OPC_BOOLINT_ENABLED)
	
/* After defering for either collision avoidance or interframe gap      */
/* the channel will be available for transmission 						*/
#define DEFERENCE_OFF			(intrpt_type == OPC_INTRPT_SELF && \
								 intrpt_code == WlanC_Deference_Off && \
								 wlan_flags->receiver_busy == OPC_BOOLINT_DISABLED)

/* Isssue a transmission complete stat once the packet has successfully */
/* been transmitted from the source station								*/						 
#define TRANSMISSION_COMPLETE	(intrpt_type == OPC_INTRPT_STAT && \
								 op_intrpt_stat () == TRANSMITTER_BUSY_INSTAT)

/* Backoff is performed based on the value of the backoff flag.			*/
#define PERFORM_BACKOFF			(wlan_flags->backoff_flag == OPC_BOOLINT_ENABLED || wlan_flags->perform_cw == OPC_BOOLINT_ENABLED)

/* Need to start transmitting frame once the backoff (self intrpt) 		*/
/* completed															*/
#define BACKOFF_COMPLETED		(intrpt_type == OPC_INTRPT_SELF && intrpt_code == WlanC_Backoff_Elapsed && \
								 (wlan_flags->receiver_busy == OPC_BOOLINT_DISABLED || wlan_flags->forced_xmt == OPC_BOOLINT_ENABLED))

/* Contention Window period, which follows a successful packet			*/
/* transmission, is completed.											*/
#define CW_COMPLETED			(intrpt_type == OPC_INTRPT_SELF && intrpt_code == WlanC_CW_Elapsed && \
								 (wlan_flags->receiver_busy == OPC_BOOLINT_DISABLED || wlan_flags->forced_xmt == OPC_BOOLINT_ENABLED))

/* After transmission the station will wait for a frame response for    */
/* Data and Rts frames.												    */
#define WAIT_FOR_FRAME          (expected_frame_type != WlanC_None)

/* Need to retransmit frame if there is a frame timeout and the         */
/* required frame is not received									    */
#define FRAME_TIMEOUT           (intrpt_type == OPC_INTRPT_SELF && intrpt_code == WlanC_Frame_Timeout)

/* If the frame is received appropriate response will be transmitted    */
/* provided the medium is considered to be idle						    */
#define FRAME_RCVD			    (intrpt_type == OPC_INTRPT_STRM && bad_packet_rcvd == OPC_BOOLINT_DISABLED && \
		 						 i_strm <= LOW_LAYER_INPUT_STREAM_CH4)

/* Skip backoff if no backoff is needed								    */
#define TRANSMIT_FRAME			(!PERFORM_BACKOFF)

/* Expecting frame response	after data or Rts transmission			    */
#define EXPECTING_FRAME			(expected_frame_type != WlanC_None)

/* When the contention window period is over then we go to IDLE state	*/
/* if we don't have another frame to send at that moment. If we have	*/
/* one then we go to TRANSMIT state if we did not sense any activity	*/
/* on our receiver for a period that is greater than or equal to DIFS	*/
/* period; otherwise we go to DEFER state to defer and back-off before	*/
/* transmitting the new frame.											*/
#define	BACK_TO_IDLE			(CW_COMPLETED  && wlan_flags->data_frame_to_send == OPC_BOOLINT_DISABLED)
	
#define SEND_NEW_FRAME_AFTER_CW	(CW_COMPLETED && wlan_flags->data_frame_to_send == OPC_BOOLINT_ENABLED && MEDIUM_IS_IDLE)

#define DEFER_AFTER_CW			(CW_COMPLETED && wlan_flags->data_frame_to_send == OPC_BOOLINT_ENABLED && !MEDIUM_IS_IDLE)

/* Macros that check the change in the busy status of the receiver.	   	*/
#define	RECEIVER_BUSY_HIGH		(intrpt_type == OPC_INTRPT_STAT && intrpt_code < TRANSMITTER_BUSY_INSTAT && \
								 op_stat_local_read (intrpt_code) > rx_power_threshold && wlan_flags->collision == OPC_BOOLINT_DISABLED)

#define	RECEIVER_BUSY_LOW		(intrpt_type == OPC_INTRPT_STAT && intrpt_code < TRANSMITTER_BUSY_INSTAT && \
								 rcv_channel_status == 0)

#define	PERFORM_TRANSMIT		(BACKOFF_COMPLETED || SEND_NEW_FRAME_AFTER_CW)

#define	BACK_TO_DEFER			(FRAME_RCVD || DEFER_AFTER_CW || \
								 (wlan_flags->tx_beacon == OPC_BOOLINT_ENABLED && rcv_channel_status == 0)) 							

/* Macro to evaluate whether the MAC is in a contention free period.	*/
#define	IN_CFP					(pcf_flag == OPC_BOOLINT_ENABLED && \
								 (cfp_ap_medium_control == OPC_BOOLINT_ENABLED || wlan_flags->pcf_active == OPC_BOOLINT_ENABLED))

/* After receiving a packet that indicates the end of the current CFP	*/
/* go to back to IDLE state if there is no packet to transmit in the CP.*/
#define	IDLE_AFTER_CFP			(intrpt_type == OPC_INTRPT_STRM && !FRAME_TO_TRANSMIT && !IN_CFP)

/* Macro to cancel the self interrupt for end of deference. It is		*/
/* called at the state transition from DEFER to IDLE.					*/
#define	CANCEL_DEF_EVENT		(op_ev_cancel (deference_evh))

#define FRM_END_TO_IDLE			(!FRAME_TO_TRANSMIT && !EXPECTING_FRAME && !IN_CFP)
	
#define	FRM_END_TO_DEFER		(!EXPECTING_FRAME && (FRAME_TO_TRANSMIT || IN_CFP))



//-----------------------------------------------
//					TYPE OF BACKOFF
//-----------------------------------------------

#define		ORIGINAL					0
#define		TRAFFIC_ADAPTIVE			1
#define		BORDER_ADAPTIVE				2
#define		DISTANCE_SINK_ADAPTIVE		3


//The backoff is multiplied at most by MAX_BACKOFF_FACTOR
#define		MAX_BACKOFF_FACTOR			5

//The max ratio_traffic is timeouted every X (Perhaps it will change (traffic change, dead node...))
#define		MAX_RATIO_TRAFFIC_TIMEOUT	10.0




/* Function declarations.	*/
static void			wlan_mac_sv_init ();
static void			wlan_higher_layer_data_arrival ();
static void			wlan_physical_layer_data_arrival ();
static void			wlan_hlpk_enqueue (Packet* hld_pkptr, int dest_addr, Boolean polling);
Boolean 			wlan_tuple_find (int sta_addr, int seq_id, int frag_num);
static void			wlan_data_process (Packet* seg_pkptr,int dest_addr, int sta_addr, int final_dest_addr, int frag_num, int more_frag, int pkt_id);
static void			wlan_accepted_frame_stats_update (Packet* seg_pkptr);
static void			wlan_interrupts_process ();
static void 		wlan_prepare_frame_to_send (int frame_type);
static void			wlan_frame_transmit ();
static void			wlan_schedule_deference ();
static void			wlan_frame_discard ();
static void			wlan_mac_rcv_channel_status_update (int channel_id);
static void			wlan_mac_error (const char* msg1, const char* msg2, const char* msg3);

static void			wlan_pcf_frame_discard ();
int		 			wlan_hld_list_elem_add_comp (WlanT_Hld_List_Elem* hld_ptr1,  WlanT_Hld_List_Elem* hld_ptr2);
Boolean				wlan_poll_list_member_find (int dest_addr); 

static void			wlan_frame_type_conv(int frame_type, char* frame_type_name); 



//-----------------------------------------------
//					ROUTES
//-----------------------------------------------

List	*mac_all_routes;
double	max_ratio_traffic = 0;
double	max_ratio_traffic_time = 0;

/* End of Header Block */


#if !defined (VOSD_NO_FIN)
#undef	BIN
#undef	BOUT
#define	BIN		FIN_LOCAL_FIELD(last_line_passed) = __LINE__ - _block_origin;
#define	BOUT	BIN
#define	BINIT	FIN_LOCAL_FIELD(last_line_passed) = 0; _block_origin = __LINE__;
#else
#define	BINIT
#endif /* #if !defined (VOSD_NO_FIN) */



/* State variable definitions */
typedef struct
	{
	/* Internal state tracking for FSM */
	FSM_SYS_STATE
	/* State Variables */
	int	                    		retry_count;
	int	                    		intrpt_type;
	WlanT_Mac_Intrpt_Code	  		intrpt_code;
	int	                    		my_address;
	Objid	                  		my_objid;
	Objid	                  		my_node_objid;
	Objid	                  		my_subnet_objid;
	Objid	                  		tx_objid;
	Objid	                  		rx_objid;
	OmsT_Pr_Handle	         		own_process_record_handle;
	List*	                  		hld_list_ptr;
	double	                 		operational_speed;
	int	                    		frag_threshold;
	int	                    		packet_seq_number;
	int	                    		packet_frag_number;
	int	                    		destination_addr;
	Sbhandle	               		fragmentation_buffer_ptr;
	WlanT_Mac_Frame_Type	   		fresp_to_send;
	double	                 		nav_duration;
	int	                    		rts_threshold;
	int	                    		duplicate_entry;
	WlanT_Mac_Frame_Type	   		expected_frame_type;
	int	                    		remote_sta_addr;
	double	                 		backoff_slots;
	Stathandle	             		packet_load_handle;
	double	                 		intrpt_time;
	Packet *	               		wlan_transmit_frame_copy_ptr;
	Stathandle	             		backoff_slots_handle;
	int	                    		instrm_from_mac_if;
	int	                    		outstrm_to_mac_if;
	int	                    		num_fragments;
	int	                    		remainder_size;
	List*	                  		defragmentation_list_ptr;
	WlanT_Mac_Flags*	       		wlan_flags;
	OmsT_Aa_Address_Handle	 		oms_aa_handle;
	double	                 		current_time;
	double	                 		rcv_idle_time;
	WlanT_Mac_Duplicate_Buffer_Entry**			duplicate_list_ptr;
	Pmohandle	              		hld_pmh;
	int	                    		max_backoff;
	char	                   		current_state_name [32];
	Stathandle	             		hl_packets_rcvd;
	Stathandle	             		media_access_delay;
	Stathandle	             		ete_delay_handle;
	Stathandle	             		global_ete_delay_handle;
	Stathandle	             		global_throughput_handle;
	Stathandle	             		global_load_handle;
	Stathandle	             		global_dropped_data_handle;
	Stathandle	             		global_mac_delay_handle;
	Stathandle	             		ctrl_traffic_rcvd_handle_inbits;
	Stathandle	             		ctrl_traffic_sent_handle_inbits;
	Stathandle	             		ctrl_traffic_rcvd_handle;
	Stathandle	             		ctrl_traffic_sent_handle;
	Stathandle	             		data_traffic_rcvd_handle_inbits;
	Stathandle	             		data_traffic_sent_handle_inbits;
	Stathandle	             		data_traffic_rcvd_handle;
	Stathandle	             		data_traffic_sent_handle;
	double	                 		sifs_time;
	double	                 		slot_time;
	int	                    		cw_min;
	int	                    		cw_max;
	double	                 		difs_time;
	double	                 		plcp_overhead_control;
	double	                 		plcp_overhead_data;
	Stathandle	             		channel_reserv_handle;
	Stathandle	             		retrans_handle;
	Stathandle	             		throughput_handle;
	int	                    		long_retry_limit;
	int	                    		short_retry_limit;
	int	                    		retry_limit;
	WlanT_Mac_Frame_Type	   		last_frametx_type;
	Evhandle	               		deference_evh;
	Evhandle	               		backoff_elapsed_evh;
	Evhandle	               		frame_timeout_evh;
	double	                 		eifs_time;
	int	                    		i_strm;
	Boolean	                		wlan_trace_active;
	int	                    		pkt_in_service;
	Stathandle	             		bits_load_handle;
	int	                    		ap_flag;
	int	                    		bss_flag;
	int	                    		ap_mac_address;
	int	                    		hld_max_size;
	double	                 		max_receive_lifetime;
	Boolean	                		accept_large_packets;
	WlanT_Phy_Char_Code	    		phy_char_flag;
	OmsT_Aa_Address_Handle	 		oms_aa_wlan_handle;
	int	                    		total_hlpk_size;
	Stathandle	             		drop_packet_handle;
	Stathandle	             		drop_packet_handle_inbits;
	Log_Handle	             		drop_pkt_log_handle;
	int	                    		drop_pkt_entry_log_flag;
	int	                    		packet_size;
	double	                 		receive_time;
	Ici*	                   		llc_iciptr;
	int	                    		rcv_channel_status;
	double	                 		rx_power_threshold;
	int*	                   		bss_stn_list;
	int	                    		bss_stn_count;
	int	                    		bss_id;
	int	                    		pcf_retry_count;
	int	                    		poll_fail_count;
	int	                    		max_poll_fails;
	List*	                  		cfpd_list_ptr;
	int	                    		pcf_queue_offset;
	double	                 		beacon_int;
	Sbhandle	               		pcf_frag_buffer_ptr;
	Packet *	               		wlan_pcf_transmit_frame_copy_ptr;
	int	                    		pcf_num_fragments;
	int	                    		pcf_remainder_size;
	int*	                   		polling_list;
	int	                    		poll_list_size;
	int	                    		poll_index;
	double	                 		pifs_time;
	Evhandle	               		beacon_evh;
	Evhandle	               		cfp_end_evh;
	int	                    		pcf_pkt_in_service;
	int	                    		pcf_flag;
	Boolean	                		active_pc;
	int	                    		cfp_prd;
	int	                    		cfp_offset;
	double	                 		cfp_length;
	int	                    		ap_relay;
	int	                    		total_cfpd_size;
	int	                    		packet_size_dcf;
	int	                    		packet_size_pcf;
	double	                 		receive_time_dcf;
	double	                 		receive_time_pcf;
	Boolean	                		cfp_ap_medium_control;
	int	                    		pcf_network;
	List*	                  		my_route;
	Boolean	                		is_mac_border_node;
	int	                    		backoff_type;
	} wlan_mac_wifib_state;

#define pr_state_ptr            		((wlan_mac_wifib_state*) SimI_Mod_State_Ptr)
#define retry_count             		pr_state_ptr->retry_count
#define intrpt_type             		pr_state_ptr->intrpt_type
#define intrpt_code             		pr_state_ptr->intrpt_code
#define my_address              		pr_state_ptr->my_address
#define my_objid                		pr_state_ptr->my_objid
#define my_node_objid           		pr_state_ptr->my_node_objid
#define my_subnet_objid         		pr_state_ptr->my_subnet_objid
#define tx_objid                		pr_state_ptr->tx_objid
#define rx_objid                		pr_state_ptr->rx_objid
#define own_process_record_handle		pr_state_ptr->own_process_record_handle
#define hld_list_ptr            		pr_state_ptr->hld_list_ptr
#define operational_speed       		pr_state_ptr->operational_speed
#define frag_threshold          		pr_state_ptr->frag_threshold
#define packet_seq_number       		pr_state_ptr->packet_seq_number
#define packet_frag_number      		pr_state_ptr->packet_frag_number
#define destination_addr        		pr_state_ptr->destination_addr
#define fragmentation_buffer_ptr		pr_state_ptr->fragmentation_buffer_ptr
#define fresp_to_send           		pr_state_ptr->fresp_to_send
#define nav_duration            		pr_state_ptr->nav_duration
#define rts_threshold           		pr_state_ptr->rts_threshold
#define duplicate_entry         		pr_state_ptr->duplicate_entry
#define expected_frame_type     		pr_state_ptr->expected_frame_type
#define remote_sta_addr         		pr_state_ptr->remote_sta_addr
#define backoff_slots           		pr_state_ptr->backoff_slots
#define packet_load_handle      		pr_state_ptr->packet_load_handle
#define intrpt_time             		pr_state_ptr->intrpt_time
#define wlan_transmit_frame_copy_ptr		pr_state_ptr->wlan_transmit_frame_copy_ptr
#define backoff_slots_handle    		pr_state_ptr->backoff_slots_handle
#define instrm_from_mac_if      		pr_state_ptr->instrm_from_mac_if
#define outstrm_to_mac_if       		pr_state_ptr->outstrm_to_mac_if
#define num_fragments           		pr_state_ptr->num_fragments
#define remainder_size          		pr_state_ptr->remainder_size
#define defragmentation_list_ptr		pr_state_ptr->defragmentation_list_ptr
#define wlan_flags              		pr_state_ptr->wlan_flags
#define oms_aa_handle           		pr_state_ptr->oms_aa_handle
#define current_time            		pr_state_ptr->current_time
#define rcv_idle_time           		pr_state_ptr->rcv_idle_time
#define duplicate_list_ptr      		pr_state_ptr->duplicate_list_ptr
#define hld_pmh                 		pr_state_ptr->hld_pmh
#define max_backoff             		pr_state_ptr->max_backoff
#define current_state_name      		pr_state_ptr->current_state_name
#define hl_packets_rcvd         		pr_state_ptr->hl_packets_rcvd
#define media_access_delay      		pr_state_ptr->media_access_delay
#define ete_delay_handle        		pr_state_ptr->ete_delay_handle
#define global_ete_delay_handle 		pr_state_ptr->global_ete_delay_handle
#define global_throughput_handle		pr_state_ptr->global_throughput_handle
#define global_load_handle      		pr_state_ptr->global_load_handle
#define global_dropped_data_handle		pr_state_ptr->global_dropped_data_handle
#define global_mac_delay_handle 		pr_state_ptr->global_mac_delay_handle
#define ctrl_traffic_rcvd_handle_inbits		pr_state_ptr->ctrl_traffic_rcvd_handle_inbits
#define ctrl_traffic_sent_handle_inbits		pr_state_ptr->ctrl_traffic_sent_handle_inbits
#define ctrl_traffic_rcvd_handle		pr_state_ptr->ctrl_traffic_rcvd_handle
#define ctrl_traffic_sent_handle		pr_state_ptr->ctrl_traffic_sent_handle
#define data_traffic_rcvd_handle_inbits		pr_state_ptr->data_traffic_rcvd_handle_inbits
#define data_traffic_sent_handle_inbits		pr_state_ptr->data_traffic_sent_handle_inbits
#define data_traffic_rcvd_handle		pr_state_ptr->data_traffic_rcvd_handle
#define data_traffic_sent_handle		pr_state_ptr->data_traffic_sent_handle
#define sifs_time               		pr_state_ptr->sifs_time
#define slot_time               		pr_state_ptr->slot_time
#define cw_min                  		pr_state_ptr->cw_min
#define cw_max                  		pr_state_ptr->cw_max
#define difs_time               		pr_state_ptr->difs_time
#define plcp_overhead_control   		pr_state_ptr->plcp_overhead_control
#define plcp_overhead_data      		pr_state_ptr->plcp_overhead_data
#define channel_reserv_handle   		pr_state_ptr->channel_reserv_handle
#define retrans_handle          		pr_state_ptr->retrans_handle
#define throughput_handle       		pr_state_ptr->throughput_handle
#define long_retry_limit        		pr_state_ptr->long_retry_limit
#define short_retry_limit       		pr_state_ptr->short_retry_limit
#define retry_limit             		pr_state_ptr->retry_limit
#define last_frametx_type       		pr_state_ptr->last_frametx_type
#define deference_evh           		pr_state_ptr->deference_evh
#define backoff_elapsed_evh     		pr_state_ptr->backoff_elapsed_evh
#define frame_timeout_evh       		pr_state_ptr->frame_timeout_evh
#define eifs_time               		pr_state_ptr->eifs_time
#define i_strm                  		pr_state_ptr->i_strm
#define wlan_trace_active       		pr_state_ptr->wlan_trace_active
#define pkt_in_service          		pr_state_ptr->pkt_in_service
#define bits_load_handle        		pr_state_ptr->bits_load_handle
#define ap_flag                 		pr_state_ptr->ap_flag
#define bss_flag                		pr_state_ptr->bss_flag
#define ap_mac_address          		pr_state_ptr->ap_mac_address
#define hld_max_size            		pr_state_ptr->hld_max_size
#define max_receive_lifetime    		pr_state_ptr->max_receive_lifetime
#define accept_large_packets    		pr_state_ptr->accept_large_packets
#define phy_char_flag           		pr_state_ptr->phy_char_flag
#define oms_aa_wlan_handle      		pr_state_ptr->oms_aa_wlan_handle
#define total_hlpk_size         		pr_state_ptr->total_hlpk_size
#define drop_packet_handle      		pr_state_ptr->drop_packet_handle
#define drop_packet_handle_inbits		pr_state_ptr->drop_packet_handle_inbits
#define drop_pkt_log_handle     		pr_state_ptr->drop_pkt_log_handle
#define drop_pkt_entry_log_flag 		pr_state_ptr->drop_pkt_entry_log_flag
#define packet_size             		pr_state_ptr->packet_size
#define receive_time            		pr_state_ptr->receive_time
#define llc_iciptr              		pr_state_ptr->llc_iciptr
#define rcv_channel_status      		pr_state_ptr->rcv_channel_status
#define rx_power_threshold      		pr_state_ptr->rx_power_threshold
#define bss_stn_list            		pr_state_ptr->bss_stn_list
#define bss_stn_count           		pr_state_ptr->bss_stn_count
#define bss_id                  		pr_state_ptr->bss_id
#define pcf_retry_count         		pr_state_ptr->pcf_retry_count
#define poll_fail_count         		pr_state_ptr->poll_fail_count
#define max_poll_fails          		pr_state_ptr->max_poll_fails
#define cfpd_list_ptr           		pr_state_ptr->cfpd_list_ptr
#define pcf_queue_offset        		pr_state_ptr->pcf_queue_offset
#define beacon_int              		pr_state_ptr->beacon_int
#define pcf_frag_buffer_ptr     		pr_state_ptr->pcf_frag_buffer_ptr
#define wlan_pcf_transmit_frame_copy_ptr		pr_state_ptr->wlan_pcf_transmit_frame_copy_ptr
#define pcf_num_fragments       		pr_state_ptr->pcf_num_fragments
#define pcf_remainder_size      		pr_state_ptr->pcf_remainder_size
#define polling_list            		pr_state_ptr->polling_list
#define poll_list_size          		pr_state_ptr->poll_list_size
#define poll_index              		pr_state_ptr->poll_index
#define pifs_time               		pr_state_ptr->pifs_time
#define beacon_evh              		pr_state_ptr->beacon_evh
#define cfp_end_evh             		pr_state_ptr->cfp_end_evh
#define pcf_pkt_in_service      		pr_state_ptr->pcf_pkt_in_service
#define pcf_flag                		pr_state_ptr->pcf_flag
#define active_pc               		pr_state_ptr->active_pc
#define cfp_prd                 		pr_state_ptr->cfp_prd
#define cfp_offset              		pr_state_ptr->cfp_offset
#define cfp_length              		pr_state_ptr->cfp_length
#define ap_relay                		pr_state_ptr->ap_relay
#define total_cfpd_size         		pr_state_ptr->total_cfpd_size
#define packet_size_dcf         		pr_state_ptr->packet_size_dcf
#define packet_size_pcf         		pr_state_ptr->packet_size_pcf
#define receive_time_dcf        		pr_state_ptr->receive_time_dcf
#define receive_time_pcf        		pr_state_ptr->receive_time_pcf
#define cfp_ap_medium_control   		pr_state_ptr->cfp_ap_medium_control
#define pcf_network             		pr_state_ptr->pcf_network
#define my_route                		pr_state_ptr->my_route
#define is_mac_border_node      		pr_state_ptr->is_mac_border_node
#define backoff_type            		pr_state_ptr->backoff_type

/* This macro definition will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE
#define FIN_PREAMBLE	wlan_mac_wifib_state *op_sv_ptr = pr_state_ptr;


/* Function Block */

enum { _block_origin = __LINE__ };
static void
wlan_mac_sv_init ()
	{
	Objid					mac_params_comp_attr_objid;
	Objid					params_attr_objid;
	Objid					chann_params_comp_attr_objid;
	Objid					subchann_params_attr_objid;	
	Objid					pcf_params_comp_attr_objid;
	Objid					subpcf_params_attr_objid;	
	Objid					chann_objid;
	Objid					sub_chann_objid;
	int						num_chann;
	char					subnet_name [512];
	double					bandwidth;
	double					frequency;
	int						i;
	char                    bss_name[128];
	Log_Handle				bssid_changed_log_handle;
	Objid					statwire_objid;
	int						num_statwires;
	double					threshold;
	int						rts;
	
	/**	1. Initialize state variables.						**/
	/** 2. Read model attribute values in variables.	    **/
	/** 3. Create global lists								**/
	/** 4. Register statistics handlers						**/
	FIN (wlan_mac_sv_init ());

	/* object id of the surrounding processor.				*/
	my_objid = op_id_self ();

	/* Obtain the node's object identifier					*/
	my_node_objid = op_topo_parent (my_objid);

	/* Obtain subnet objid.									*/
	my_subnet_objid = op_topo_parent (my_node_objid);

	/* Obtain the values assigned to the various attributes	*/
	op_ima_obj_attr_get (my_objid, "Wireless LAN Parameters", &mac_params_comp_attr_objid);
    params_attr_objid = op_topo_child (mac_params_comp_attr_objid, OPC_OBJTYPE_GENERIC, 0);

	/* Determine the assigned MAC address.					*/
	op_ima_obj_attr_get (my_objid, "Address", &my_address);
	
	/* Obtain an address handle for resolving WLAN MAC addresses.				*/
	oms_aa_handle = oms_aa_address_handle_get ("MAC Addresses", "Address");

 	/* Obtain the BSS_Id attribute to determine if BSS based network is used	*/ 
	op_ima_obj_attr_get (params_attr_objid, "BSS Identifier", &bss_id);
	
	/* Update the global variable if this is the first node to come up. If not the	*/ 
	/* first node, then check for mismatches. A subnet can be a traditional subnet	*/
	/* (i.e. a subnet with one BSS,this is the existing model) or a  BSS based		*/
	/* subnet where for every node the attribute BSS_Id is set to indicate to which */
	/* BSS a node belongs. If the global is set to traditional subnet and the this	*/
	/* node has its BSS_Id attribute set then log a warning message and recover		*/
	/* by considering the BSS_Id attribute setting as not used.If the global is		*/
	/* set to BSS based subnet and this node is not using its BSS_Id attribute 		*/
	/* then log an error message and stop the simulation.					 		*/
	if (bss_id_type == WlanC_Not_Set )
		{
		if (bss_id ==  WLAN_BSSID_NOT_USED )
			{
			bss_id_type = WlanC_Entire_Subnet ;
			}
		else
			{
			bss_id_type = WlanC_Bss_Divided_Subnet ;
			}
		}
	else
		{
		if (bss_id_type == WlanC_Entire_Subnet && bss_id != WLAN_BSSID_NOT_USED)
			{
			/* Recoverable mismatch, log warning and continue by enforcing			*/
			/* traditional subnet, i.e. force the bss_id variable to not used.		*/

			/* Register the log handle. 											*/
			bssid_changed_log_handle = op_prg_log_handle_create (OpC_Log_Category_Configuration, "Wireless Lan", "BSS ID Changed", 128);

			/* Write the warning message.											*/
			op_prg_log_entry_write(bssid_changed_log_handle,
				"WARNING:\n"
				" A node with an explicit BSS \n"
				" assigment was found in a pure \n"
				" subnet.\n"
				"ACTION:\n"
				" The BSS identifier is set to\n"
				" the default value.\n"
				"CAUSE:\n"
				" There are some nodes in the\n"
				" network which have their BSS\n"
				" identifiers set to the default\n"
				" while the others have the\n"
				" default setting.\n"
				"SUGGESTION:\n"
				" Ensure that all nodes have the\n"
				" BSS identifier set to the default\n"
				" value or all of them are explicitly\n"
				" assigned.\n"
				);
		    }
		}
	
	/* Create the pool of station addresses of the same BSS using BSS ID if this 	*/
	/* attribute is set.															*/
	if (bss_id_type == WlanC_Bss_Divided_Subnet )
		{
		/* Get a string based on the BSS id and generate pool of station addresses	*/
		/* using this string register with this new string bss name.				*/
		sprintf ( bss_name , "bss_%d", bss_id);
		oms_aa_wlan_handle = oms_aa_address_handle_get (bss_name, "Address");
		}
	else
		{
		/* If traditional subnet create a pool of station addresses for each		*/
		/* subnet based on subnet name.												*/
		/* oms_tan_hname_get (my_subnet_objid, subnet_name);						*/
		sprintf (subnet_name , "wireless_lan_subnet_%.0f\0", (double) my_subnet_objid);
	    oms_aa_wlan_handle = oms_aa_address_handle_get (subnet_name, "Address");
		}
	
   	/* Get model attributes.	*/
	op_ima_obj_attr_get (params_attr_objid, "Data Rate", &operational_speed);
	op_ima_obj_attr_get (params_attr_objid, "Fragmentation Threshold", &frag_threshold);
	op_ima_obj_attr_get (params_attr_objid, "Rts Threshold", &rts_threshold);
	op_ima_obj_attr_get (params_attr_objid, "Short Retry Limit", &short_retry_limit);
	op_ima_obj_attr_get (params_attr_objid, "Long Retry Limit", &long_retry_limit);
	op_ima_obj_attr_get (params_attr_objid, "Access Point Functionality", &ap_flag);	
	op_ima_obj_attr_get (params_attr_objid, "Buffer Size", &hld_max_size);
	op_ima_obj_attr_get (params_attr_objid, "Max Receive Lifetime", &max_receive_lifetime);
	op_ima_obj_attr_get (params_attr_objid, "Large Packet Processing", &accept_large_packets);

	//RTS activated or desactivated in the global simulation
	op_ima_sim_attr_get(OPC_IMA_INTEGER , "RTS" , &rts);
	if (rts)
		rts_threshold = 1;
	else
		rts_threshold = -1;
	
	/* Initialize the retry limit for the current frame to long retry limit.	*/
	retry_limit = long_retry_limit;
	
	/* Get the Channel Settings.										*/
	/* Extracting Channel 0,1,2,3 (i.e. 1,2,5.5 and 11Mbps) Settings	*/
	op_ima_obj_attr_get (params_attr_objid, "Channel Settings", &chann_params_comp_attr_objid);
	subchann_params_attr_objid = op_topo_child (chann_params_comp_attr_objid, OPC_OBJTYPE_GENERIC, 0);
	op_ima_obj_attr_get (subchann_params_attr_objid, "Bandwidth", &bandwidth);	
	op_ima_obj_attr_get (subchann_params_attr_objid, "Min Frequency", &frequency);	

	/* If the frequency is BSS based then assign frequency as a			*/
	/* function of the BSS Identifier.								 	*/
	if (frequency == WLAN_BSS_BASED_FREQ_USED)
		{
		/* Frequency in KHz as a funtion of the BSS identifier.		 	*/		
		/*frequency = (bss_id + 2) * 30 ;*/
		
		frequency = 2402 + 1 * (bss_id + 1);	
		}
	
	/* Extract beacon and PCF parameters        */
	op_ima_obj_attr_get (params_attr_objid, "PCF Parameters", &pcf_params_comp_attr_objid);
	subpcf_params_attr_objid = op_topo_child (pcf_params_comp_attr_objid, OPC_OBJTYPE_GENERIC, 0);
	op_ima_obj_attr_get (subpcf_params_attr_objid, "PCF Functionality", &pcf_flag);	
	op_ima_obj_attr_get (subpcf_params_attr_objid, "CFP Beacon Multiple", &cfp_prd);	
	op_ima_obj_attr_get (subpcf_params_attr_objid, "CFP Offset", &cfp_offset);	
	op_ima_obj_attr_get (subpcf_params_attr_objid, "CFP Interval", &cfp_length);	
	op_ima_obj_attr_get (subpcf_params_attr_objid, "Max Failed Polls", &max_poll_fails);	
	op_ima_obj_attr_get (subpcf_params_attr_objid, "Beacon Interval", &beacon_int);	

	ap_relay = OPC_BOOLINT_ENABLED;
		
	/* Check if there is an active AP controlling the medium during the CFP.*/
	if ((ap_flag == OPC_BOOLINT_ENABLED) && (pcf_flag == OPC_BOOLINT_ENABLED))
		active_pc= OPC_BOOLINT_ENABLED;
	else
		active_pc= OPC_BOOLINT_DISABLED;

	/* Load the appropriate physical layer characteristics.					*/	
	op_ima_obj_attr_get (params_attr_objid, "Physical Characteristics", &phy_char_flag);

	/* Obtain the receiver valid packet power threshold value used by the	*/
	/* statwires from the receiver into the MAC module.						*/
	op_ima_obj_attr_get (params_attr_objid, "Packet Reception-Power Threshold", &rx_power_threshold);
	
	/* Based on physical charateristics settings set appropriate values to	*/
	/* the variables.														*/
	switch (phy_char_flag)
		{
		case WlanC_Frequency_Hopping:
			{
			/* Slot duration in terms of seconds.							*/
			slot_time = 50E-06;

			/* Short interframe gap in terms of seconds.					*/
			sifs_time = 28E-06;
			
			/* PLCP overheads, which include the preamble and header, in	*/
			/* terms of seconds.											*/
			plcp_overhead_control = 128E-06;
			plcp_overhead_data    = 128E-06;
			
			/* Minimum contention window size for selecting backoff slots.	*/
			cw_min = 15;

			/* Maximum contention window size for selecting backoff slots.	*/
			cw_max = 1023;
			break;
			}

		case WlanC_Direct_Sequence:
			{
			/* Slot duration in terms of seconds.							*/
			slot_time = 20E-06;

			/* Short interframe gap in terms of seconds.					*/
			sifs_time = 10E-06;

			/* PLCP overheads, which include the preamble and header, in	*/
			/* terms of seconds.											*/
			plcp_overhead_control = 192E-06;
			plcp_overhead_data    = 192E-06;
			
			/* Minimum contention window size for selecting backoff slots.	*/
			cw_min = 31;

			/* Maximum contention window size for selecting backoff slots.	*/
			cw_max = 1023;
			break;
			}

		case WlanC_Infra_Red:
			{
			/* Slot duration in terms of seconds.							*/
			slot_time = 8E-06;

			/* Short interframe gap in terms of seconds.					*/
			sifs_time = 7E-06;

			/* PLCP overheads, which include the preamble and header, in	*/
			/* terms of seconds. Infra-red supports transmission of parts	*/
			/* of the PLCP header at the regular data transmission rate,	*/
			/* which can be higher than mandatory lowest data rate.			*/
			plcp_overhead_control = 57E-06;
			plcp_overhead_data    = 25E-06 + (ceil (32000000.0 / operational_speed) / 1E6);
	  
			/* Minimum contention window size for selecting backoff slots.	*/
			cw_min = 63;

			/* Maximum contention window size for selecting backoff slots.	*/
			cw_max = 1023;
			break;
			}

		default:
			{
			wlan_mac_error ("Unexpected Physical Layer Characteristic encountered.", OPC_NIL, OPC_NIL);
			break;
			}
		}

	/** By default stations are configured for IBSS unless an Access Point is found,**/
	/** then the network will have an infrastructure BSS configuration.				**/
	bss_flag = OPC_BOOLINT_DISABLED;

	/* Computing DIFS interval which is interframe gap between successive	*/
	/* frame transmissions.													*/
	difs_time = sifs_time + 2 * slot_time;

	/* If the receiver detects that the received frame is erroneous then it	*/
	/* will set the network allocation vector to EIFS duration. 			*/
	eifs_time = difs_time + sifs_time + WLAN_ACK_DURATION + plcp_overhead_control;
	
	/** PIFS duration is used by the AP operating under PCF to gain		**/
	/** priority to access the medium **/
	pifs_time = sifs_time + slot_time;
	
	/* Creating list to store data arrived from higher layer.	*/	
	hld_list_ptr = op_prg_list_create ();

	/* If the station is an AP, and PCF supported, creat separate PCF queue list for higher layer. */
	if ((ap_flag == OPC_BOOLINT_ENABLED) && (pcf_flag == OPC_BOOLINT_ENABLED))
		cfpd_list_ptr = op_prg_list_create ();
	else
		cfpd_list_ptr = OPC_NIL;

	
	/* Initialize segmentation and reassembly buffer.	*/
	defragmentation_list_ptr = op_prg_list_create ();
	fragmentation_buffer_ptr = op_sar_buf_create (OPC_SAR_BUF_TYPE_SEGMENT, OPC_SAR_BUF_OPT_PK_BNDRY);

	/* Initialize PCF segmentation and reassembly buffer. (only used by AP)	*/
	pcf_frag_buffer_ptr = op_sar_buf_create (OPC_SAR_BUF_TYPE_SEGMENT, OPC_SAR_BUF_OPT_PK_BNDRY);

	/* Registering local statistics.	*/
	packet_load_handle				= op_stat_reg ("Wireless Lan.Load (packets)", 					  OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	bits_load_handle				= op_stat_reg ("Wireless Lan.Load (bits/sec)", 					  OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	hl_packets_rcvd					= op_stat_reg ("Wireless Lan.Hld Queue Size (packets)", 		  OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	backoff_slots_handle			= op_stat_reg ("Wireless Lan.Backoff Slots (slots)", 			  OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	data_traffic_sent_handle 		= op_stat_reg ("Wireless Lan.Data Traffic Sent (packets/sec)", 	  OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);	
	data_traffic_rcvd_handle		= op_stat_reg ("Wireless Lan.Data Traffic Rcvd (packets/sec)", 	  OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL); 
	data_traffic_sent_handle_inbits	= op_stat_reg ("Wireless Lan.Data Traffic Sent (bits/sec)", 	  OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	data_traffic_rcvd_handle_inbits	= op_stat_reg ("Wireless Lan.Data Traffic Rcvd (bits/sec)", 	  OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	ctrl_traffic_sent_handle	 	= op_stat_reg ("Wireless Lan.Control Traffic Sent (packets/sec)", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	ctrl_traffic_rcvd_handle		= op_stat_reg ("Wireless Lan.Control Traffic Rcvd (packets/sec)", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL); 
	ctrl_traffic_sent_handle_inbits	= op_stat_reg ("Wireless Lan.Control Traffic Sent (bits/sec)",    OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	ctrl_traffic_rcvd_handle_inbits	= op_stat_reg ("Wireless Lan.Control Traffic Rcvd (bits/sec)", 	  OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL); 
	drop_packet_handle       		= op_stat_reg ("Wireless Lan.Dropped Data Packets (packets/sec)", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL); 
	drop_packet_handle_inbits		= op_stat_reg ("Wireless Lan.Dropped Data Packets (bits/sec)", 	  OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL); 
	retrans_handle					= op_stat_reg ("Wireless Lan.Retransmission Attempts (packets)",  OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL); 
	media_access_delay				= op_stat_reg ("Wireless Lan.Media Access Delay (sec)", 		  OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	ete_delay_handle				= op_stat_reg ("Wireless Lan.Delay (sec)", 					 	  OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	channel_reserv_handle			= op_stat_reg ("Wireless Lan.Channel Reservation (sec)", 		  OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	throughput_handle				= op_stat_reg ("Wireless Lan.Throughput (bits/sec)", 			  OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);

	/* Registering global statistics.		*/
	global_ete_delay_handle 		= op_stat_reg ("Wireless LAN.Delay (sec)", 	  		    OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	global_load_handle 				= op_stat_reg ("Wireless LAN.Load (bits/sec)", 		    OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	global_throughput_handle 		= op_stat_reg ("Wireless LAN.Throughput (bits/sec)",    OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	global_dropped_data_handle		= op_stat_reg ("Wireless LAN.Data Dropped (bits/sec)",  OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	global_mac_delay_handle			= op_stat_reg ("Wireless LAN.Media Access Delay (sec)", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);

	/* Registering log handles and flags.	*/
	drop_pkt_log_handle	= op_prg_log_handle_create (OpC_Log_Category_Configuration, "Wireless Lan", "Data packet Drop", 128);
    drop_pkt_entry_log_flag = 0;

	/* Allocating memory for the flags used in this process model. */
	wlan_flags = (WlanT_Mac_Flags *) op_prg_mem_alloc (sizeof (WlanT_Mac_Flags));

	/* Disabling all flags as a default.	*/
	wlan_flags->data_frame_to_send 	= OPC_BOOLINT_DISABLED;
	wlan_flags->backoff_flag       	= OPC_BOOLINT_DISABLED;
	wlan_flags->rts_sent		   	= OPC_BOOLINT_DISABLED;
	wlan_flags->rcvd_bad_packet		= OPC_BOOLINT_DISABLED;
	wlan_flags->receiver_busy		= OPC_BOOLINT_DISABLED;
	wlan_flags->transmitter_busy	= OPC_BOOLINT_DISABLED;
	wlan_flags->gateway_flag		= OPC_BOOLINT_DISABLED;
	wlan_flags->bridge_flag			= OPC_BOOLINT_DISABLED;
	wlan_flags->wait_eifs_dur		= OPC_BOOLINT_DISABLED;
	wlan_flags->immediate_xmt		= OPC_BOOLINT_DISABLED;
	wlan_flags->forced_xmt		    = OPC_BOOLINT_DISABLED;
	wlan_flags->cw_required			= OPC_BOOLINT_DISABLED;
	wlan_flags->perform_cw			= OPC_BOOLINT_DISABLED;
	wlan_flags->nav_updated			= OPC_BOOLINT_DISABLED;
	wlan_flags->collision			= OPC_BOOLINT_DISABLED;
	wlan_flags->collided_packet		= OPC_BOOLINT_DISABLED;

	wlan_flags->duration_zero		= OPC_BOOLINT_DISABLED;
	wlan_flags->ignore_busy			= OPC_BOOLINT_DISABLED;
	wlan_flags->use_eifs			= OPC_BOOLINT_DISABLED;
	wlan_flags->tx_beacon			= OPC_BOOLINT_DISABLED;
	wlan_flags->tx_cf_end			= OPC_BOOLINT_DISABLED;
	wlan_flags->pcf_active			= OPC_BOOLINT_DISABLED;
	wlan_flags->polled				= OPC_BOOLINT_DISABLED;
	wlan_flags->more_data			= OPC_BOOLINT_DISABLED;
	wlan_flags->more_frag			= OPC_BOOLINT_DISABLED;
	wlan_flags->pcf_side_traf		= OPC_BOOLINT_DISABLED;
	wlan_flags->active_poll			= OPC_BOOLINT_DISABLED;

	/* Intialize polling index. 								*/
	poll_index = -1;

	/* Intialize pcf retry count. 								*/
	pcf_retry_count = 0;

	/* Intialize pcf retry count. 								*/
	poll_fail_count = 0;

	/* Intialize pcf queue offset. 								*/
	pcf_queue_offset = 0;

	/* Intialize retry and back-off slot counts.				*/
	retry_count   = 0;
	backoff_slots = BACKOFF_SLOTS_UNSET;

	/* Initialize the packet pointers that holds the last		*/
	/* transmitted packets to be used for retransmissions when	*/
	/* necessary.												*/
	wlan_transmit_frame_copy_ptr     = OPC_NIL;
    wlan_pcf_transmit_frame_copy_ptr = OPC_NIL;
	
	/* Initialize NAV duration.									*/
	nav_duration = 0;
	
	/* Initialize receiver idle timer. 							*/
	rcv_idle_time = -2.0 * difs_time;

	/* Initializing the sum of sizes of the packets in the higher layer queue.	*/
	total_hlpk_size = 0;

	/* Initializing the sum of sizes of the packets in the Contention Free Period queue.	*/
	total_cfpd_size = 0;
	
	/* Initialize the state variables related with the current frame that is being handled.	*/
	packet_size_dcf  = 0;
	packet_size_pcf  = 0;
	receive_time_dcf = 0.0;
	receive_time_pcf = 0.0;
	
	/* Initializing frame response to send to none.				*/
	fresp_to_send = WlanC_None;

	/* Determines if the ap is controlling the medium. This		*/
	/* variable is used to determine when the NAV's can be		*/
	/* updates.													*/
	cfp_ap_medium_control = OPC_BOOLINT_DISABLED;

	/* Initiailizing expected frame type to none.				*/
	expected_frame_type = WlanC_None;
		
	/* Initialize the receiver channel status.					*/
	rcv_channel_status = 0;
	
	/* Data arrived from higher layer is queued in the buffer. Pool memory is used for		*/
	/* allocating data structure for the higher layer packet and the random destination		*/
	/* for the packet. This structure is then inserted in the higher layer arrival queue.	*/
	hld_pmh = op_prg_pmo_define ("WLAN hld list elements", sizeof (WlanT_Hld_List_Elem), 32);

	/* Obtaining transmitter objid for accessing channel data rate attribute.	*/
	tx_objid = op_topo_assoc (my_objid, OPC_TOPO_ASSOC_OUT, OPC_OBJTYPE_RATX, 0);

	/* If no receiver is attach then generate error message and abort the simulation.	*/
	if (tx_objid == OPC_OBJID_INVALID)
		{
		wlan_mac_error ("No transmitter attached to this MAC process", OPC_NIL, OPC_NIL);	
		}

	/* Obtaining number of channels available.	*/
	op_ima_obj_attr_get (tx_objid, "channel", &chann_objid);
	num_chann = op_topo_child_count (chann_objid, OPC_OBJTYPE_RATXCH);
	
	/* Generate error message and terminate simulation if no channel is available for transmission.	*/
	if (num_chann == 0)
		{
		wlan_mac_error (" No channel is available for transmission", OPC_NIL, OPC_NIL);
		}

	/* Setting the Frequency and Bandwidth for the transmitting channels.	*/
	for (i = 0; i < num_chann; i++)
		{ 
		/* Accessing channel to set the frequency and bandwidth.	*/
		sub_chann_objid = op_topo_child (chann_objid, OPC_OBJTYPE_RATXCH, i);

		/* Setting the operating freqeuncy and channel bandwidth for the transmitting channels.	*/	
		op_ima_obj_attr_set (sub_chann_objid, "bandwidth", bandwidth);
		op_ima_obj_attr_set (sub_chann_objid, "min frequency", frequency);
		}

	/* Obtaining receiver's objid for accessing channel data rate attribute.	*/
	rx_objid = op_topo_assoc (my_objid, OPC_TOPO_ASSOC_IN, OPC_OBJTYPE_RARX, 0);

	/* If no receiver is attach then generate error message and abort the simulation.	*/
	if (rx_objid == OPC_OBJID_INVALID)
		{
		wlan_mac_error ("No receiver attached to this MAC process", OPC_NIL, OPC_NIL);	
		}

	/* Obtaining number of channels available.	*/
	op_ima_obj_attr_get (rx_objid, "channel", &chann_objid);
	num_chann = op_topo_child_count (chann_objid, OPC_OBJTYPE_RARXCH);
	
	/* Generate error message and terminate simulation if no channel is available for reception.	*/
	if (num_chann == 0)
		{
		wlan_mac_error (" No channel is available for reception", OPC_NIL, OPC_NIL);
		}

	/* Setting the Frequency and Bandwidth for the transmitting channels.	*/
	for (i = 0; i < num_chann; i++)
		{ 	
		/* Accessing channel to set the frequency and bandwidth.	*/
		sub_chann_objid = op_topo_child (chann_objid, OPC_OBJTYPE_RARXCH, i);
	
		/* Setting the operating freqeuncy and channel bandwidth for the receiving channels.	*/
		op_ima_obj_attr_set (sub_chann_objid, "bandwidth", bandwidth);
		op_ima_obj_attr_set (sub_chann_objid, "min frequency", frequency);
		
		/* Set the "state" of the receiver to define the reception-power		*/
		/* threshold for valid WLAN packets.									*/
		op_ima_obj_state_set (sub_chann_objid, &rx_power_threshold);
		}
	
	/* Also overwrite the high threshold trigger attribute values of the		*/
	/* statwires that come into the MAC from the radio receiver by using the	*/
	/* reception power threshold. First determine the total count of incoming	*/
	/* statwires.																*/
	num_statwires = op_topo_assoc_count (my_objid, OPC_TOPO_ASSOC_IN, OPC_OBJTYPE_STATWIRE);
	for (i = 0; i < num_statwires; i++)
		{
		/* Access the next statwire. Skip it if it is coming from the			*/
		/* transmitter.															*/
		statwire_objid = op_topo_assoc (my_objid, OPC_TOPO_ASSOC_IN, OPC_OBJTYPE_STATWIRE, i);
		op_ima_obj_attr_get (statwire_objid, "high threshold trigger", &threshold);
		
		/* If the trigger is not disabled then the statwire is from the			*/
		/* receiver. Overwrite the attribute value unless they are already same.*/
		if (threshold != OPC_BOOLDBL_DISABLED && threshold != rx_power_threshold)
			op_ima_obj_attr_set (statwire_objid, "high threshold trigger", rx_power_threshold);			
		}
			
	llc_iciptr = op_ici_create ("wlan_mac_ind");

	if (llc_iciptr == OPC_NIL)
		{
		wlan_mac_error ("Unable to create ICI for communication with LLC.", OPC_NIL, OPC_NIL);
		}

	/* Initialize the variable which keeps track of number 	*/
	/* of PCF enabled nodes in the network 					*/	
	pcf_network = 0;
	
	FOUT;
	}

static void
wlan_higher_layer_data_arrival ()
	{
	Packet*					hld_pkptr;
	int						data_size, frag_size;
	int						i;
	int						dest_addr;
	Boolean					stn_det_flag;
	int						large_packet_bit = 0x1;
	int						full_buffer_bit  = 0x2;
	Ici*					ici_ptr;
	Boolean					polling;

	/** Queue the packet as it arrives from higher layer.	**/
	/** Also, store the destination address of the packet	**/
	/** in the queue and the arrival time.					**/
	FIN (wlan_higher_layer_data_arrival ());

	/* Get packet from the incomming stream from higher layer and	*/
	/* obtain the packet size 										*/
	hld_pkptr = op_pk_get (op_intrpt_strm ());	
	
	/* If we are in a bridge/switch node, then we don't accept any	*/
	/* higher layer packet unless we are AP enabled.				*/
	if ((wlan_flags->bridge_flag == OPC_BOOLINT_ENABLED) && 
		(ap_flag == OPC_BOOLINT_DISABLED))
		{
		op_pk_destroy (hld_pkptr);
		FOUT;
		}

	/* Get the size of the packet arrived from higher layer.		*/
	data_size = op_pk_total_size_get (hld_pkptr);		
	
	/* Read ICI parameters at the stream interrupt.	*/
	ici_ptr = op_intrpt_ici ();

	/* Retrieve destination address from the ici set by the interface layer.	*/
	if (ici_ptr == OPC_NIL || op_ici_attr_get (ici_ptr, "dest_addr", &dest_addr) == OPC_COMPCODE_FAILURE)
		{
		/* Generate error message.	*/
		wlan_mac_error ("Destination address in not valid.", OPC_NIL, OPC_NIL);
		}

	/* The queueing of the packets in the AP is based on the node type	*/
	/* (DCF/PCF) of the destination. If the destination node is a PCF	*/
	/* node, then the packets are queued into the cfpd_list_ptr and		*/
	/* transmitted only during the PCF period. Likewise packets for a	*/
	/* DCF destination node will be inserted into the hld_list_ptr and	*/
	/* transmitted only during DCF.										*/
	polling = wlan_poll_list_member_find (dest_addr);
	
	/* Maintaining total packet size of the packets in the higer layer queue.	*/
	/* The total size is calculated from the respective list based on whether	*/
	/* the station has been polled by the AP during the start of the simulation */
	if (polling)
		total_hlpk_size = total_cfpd_size + data_size;
	else
		total_hlpk_size = total_hlpk_size + data_size;

	/* If fragmentation is enabled and packet size is greater than the threshold		*/
	/* then MSDU length will not be more than fragmentation threshold, hence			*/
	/* the packet will be fragmented into the size less than or equal to fragmentaion   */
	/* threshold.																		*/
	if ((data_size > frag_threshold * 8) && (frag_threshold != -1))
		frag_size = frag_threshold * 8;
	else
		frag_size = data_size;
	
	/* Destroy packet if it is more than max msdu length or its		*/
	/* size zero. Also, if the size of the higher layer queue  		*/
	/* will exceed its maximum after the insertion of this packet, 	*/
	/* then discard the arrived packet. 							*/
	/* The higher layer is reponsible for the retransmission of 	*/
	/* this packet.													*/ 
	if ((data_size > WLAN_MAXMSDU_LENGTH && accept_large_packets == OPC_FALSE) ||
		frag_size > WLAN_MAXMSDU_LENGTH || 
		data_size == 0 ||
        total_hlpk_size > hld_max_size)
		{
  		/* Write an appropriate simulation log message unless the	*/
		/* same message is written before.							*/
		if (drop_pkt_entry_log_flag < full_buffer_bit + large_packet_bit)
			{
			if (total_hlpk_size > hld_max_size && !(drop_pkt_entry_log_flag & full_buffer_bit))
				{
				/* Writing log message for dropped packets.			*/
				op_prg_log_entry_write (drop_pkt_log_handle, 
				"SYMPTOM(S):\n"
			    " Wireless LAN MAC layer discarded some packets due to\n "
			    " insufficient buffer capacity. \n"
				"\n"
			    " This may lead to: \n"
  			    " - application data loss.\n"
			    " - higher layer packet retransmission.\n"
			    "\n"
			    " REMEDIAL ACTION(S): \n"
			    " 1. Reduce Network laod. \n"
			    " 2. User higher speed wireless lan. \n"
			    " 3. Increase buffer capacity\n");
				drop_pkt_entry_log_flag += full_buffer_bit;
				}
			
			else if (total_hlpk_size <= hld_max_size && frag_size > 0 && !(drop_pkt_entry_log_flag & large_packet_bit))
				{
				/* Writing log message for dropped packets due to	*/
				/* packet size.										*/
				op_prg_log_entry_write (drop_pkt_log_handle, 
				"SYMPTOM(S):\n"
			    " Wireless LAN MAC layer discarded some packets due to \n"
			    " their large sizes. This is an expected protocol \n"
				" behavior. \n"	
				"\n"
			    " This may lead to: \n"
  			    " - application data loss.\n"
			    " - higher layer packet retransmission.\n"
			    "\n"
			    " REMEDIAL ACTION(S): \n"
			    " 1. Set the higher layer packet size to \n"
				"    be smaller than max MSDU size (2304 bytes). \n"
			    " 2. Enable fragmentation threshold and large packet \n"
				"    processing. \n");
				drop_pkt_entry_log_flag += large_packet_bit;
				}
			}

		/* Change the total hld queue size to original value	*/
		/* as this packet will not be added to the queue.		*/
		total_hlpk_size = total_hlpk_size - data_size;

		/* Destroy the dropped packet.							*/
		op_pk_destroy (hld_pkptr);
		
		/* Report stat when data packet is dropped due to overflow buffer.	*/
		op_stat_write (drop_packet_handle, 1.0);
	    op_stat_write (drop_packet_handle, 0.0);

		/* Report stat when data packet is dropped due to overflow buffer.	*/
		op_stat_write (drop_packet_handle_inbits, (double) (data_size));
	    op_stat_write (drop_packet_handle_inbits, 0.0);
		op_stat_write (global_dropped_data_handle, (double) (data_size));
		op_stat_write (global_dropped_data_handle, 0.0);
		
		FOUT; 
		}
		
	/* Check for an AP bridge that whether the destined stations exist in the BSS or not	*/
	/* if not then no need to broadcast the packet.											*/
	if (wlan_flags->bridge_flag == OPC_BOOLINT_ENABLED && ap_flag == OPC_BOOLINT_ENABLED)
		{
		stn_det_flag = OPC_FALSE;
		for (i = 0; i < bss_stn_count; i++)
			{
			if (dest_addr == bss_stn_list [i])
				{
				stn_det_flag = OPC_TRUE;
				}
			}
		
		/* If the destination station doesn't exist in the BSS then */
		/* no need to broadcast the packet.							*/
		if (stn_det_flag == OPC_FALSE)
			{
			/* change the total hld queue size to original value	*/
			/* as this packet will not be added to the queue.		*/
			total_hlpk_size = total_hlpk_size - data_size;
		
			op_pk_destroy (hld_pkptr);
		
			FOUT;	
			}
		}

	/* Stamp the packet with the current time. This information will remain		*/
	/* unchanged even if the packet is copied for retransmissions, and			*/
	/* eventually it will be used by the destination MAC to compute the end-to-	*/
	/* end delay.																*/
	op_pk_stamp (hld_pkptr);
	
	/* Insert the arrived packet in higher layer queue.				*/	
	wlan_hlpk_enqueue (hld_pkptr, dest_addr, polling);

	FOUT;
	}

static void
wlan_hlpk_enqueue (Packet* hld_pkptr, int dest_addr, Boolean polling)
	{
	char					msg_string [120];
	char					msg_string1 [120];
	WlanT_Hld_List_Elem*	hld_ptr;
	double					data_size;
	
	int 					pk_size;
	int						list_size;
	
	/* Enqueuing data packet for transmission.	*/
	FIN (wlan_hlpk_enqueue (Packet* hld_pkptr, int dest_addr, polling));

	/* Allocating pool memory to the higher layer data structure type. */	
	hld_ptr = (WlanT_Hld_List_Elem *) op_prg_pmo_alloc (hld_pmh);

	/* Generate error message and abort simulation if no memory left for data received from higher layer.	*/
	if (hld_ptr == OPC_NIL)
		{
		wlan_mac_error ("No more memory left to assign for data received from higher layer", OPC_NIL, OPC_NIL);
		}

	/* Updating higher layer data structure fields.	*/
	hld_ptr->time_rcvd           = current_time;
	hld_ptr->destination_address = dest_addr;
	hld_ptr->pkptr               = hld_pkptr;

	/* Get the total packet size 					*/
	pk_size = op_pk_total_size_get (hld_pkptr);
	
	/* Check for PCF terminal and also if this station has been polled  */
	if (polling == OPC_TRUE)
		{
		/* Insert the packet sorted in the order of the MAC addressess */
		op_prg_list_insert_sorted (cfpd_list_ptr, hld_ptr, wlan_hld_list_elem_add_comp);	
		
		list_size = op_prg_list_size (cfpd_list_ptr);
		}
	else
		{
		/* Insert a packet to the list.*/
		op_prg_list_insert (hld_list_ptr, hld_ptr, OPC_LISTPOS_TAIL);	
		
		list_size = op_prg_list_size (hld_list_ptr);
		
		/* Enable the flag indicating that there is a data frame to transmit.	*/
		wlan_flags->data_frame_to_send = OPC_BOOLINT_ENABLED;
		}

	/* Printing out information to ODB.	*/
	if (wlan_trace_active == OPC_TRUE)
		{
		sprintf (msg_string, "Just arrived outbound Data packet id %f ", op_pk_id (hld_ptr->pkptr));
		sprintf	(msg_string1, "The outbound Data queue size is %d", list_size); 	
		op_prg_odb_print_major (msg_string, msg_string1, OPC_NIL);
		}

	/* Report stat when outbound data packet is received.	*/
	op_stat_write (packet_load_handle, 1.0);

	/* Report stat in bits when outbound data packet is received.	*/
	data_size = (double) op_pk_total_size_get (hld_pkptr);
	op_stat_write (bits_load_handle, data_size);
    op_stat_write (bits_load_handle, 0.0);
	
	/* Update the global statistics as well.						*/
	op_stat_write (global_load_handle, data_size);
    op_stat_write (global_load_handle, 0.0);

	op_stat_write (hl_packets_rcvd, (double) list_size);
	
	FOUT;
	}

static void 
wlan_frame_transmit ()
	{
	char						msg_string  [120];
	char						msg_string1 [120];
	WlanT_Hld_List_Elem*		hld_ptr;
	double						pkt_tx_time;
	int 						list_high_index;
	int							list_low_index;
	int							type;
	
	/** Main procedure to invoke function for preparing and  **/
	/** transmiting the appropriate frames.				     **/
	FIN (wlan_frame_transmit());

	/* Check if PCF is currently active and if time to transmit */
	/* the CFP end frame. If so check if more framgments have 	*/
	/* to be transmitted. If none then, prepare to send the 	*/
	/* cfp_end frame to indicate the end of the CFP period		*/
	if ((wlan_flags->pcf_active == OPC_BOOLINT_ENABLED) && 
		(ap_flag == OPC_BOOLINT_ENABLED) &&
		((wlan_flags->tx_beacon == OPC_BOOLINT_DISABLED) || 
		(wlan_flags->tx_cf_end == OPC_BOOLINT_ENABLED) ||
		 (op_sar_buf_size (pcf_frag_buffer_ptr) != 0)))
		{
		/* Check if the transmission of the cf end frame has been */
		/* enabled. If so, make sure there are no more fragments  */
		/* pending and the PCF fragmentation buffer is empty      */
		if ((wlan_flags->tx_cf_end == OPC_BOOLINT_ENABLED) && 
			(wlan_flags->more_frag == OPC_BOOLINT_DISABLED)
			 && (op_sar_buf_size (pcf_frag_buffer_ptr) == 0))
			{
			/* If the AP needs to ACK to a previously received       */
			/* frame send a CF_end_Ack frame, if not transmit CF end */
			if (fresp_to_send == WlanC_Ack) 
				wlan_prepare_frame_to_send (WlanC_Cf_End_A);
			else  
				wlan_prepare_frame_to_send (WlanC_Cf_End);
						
			FOUT;
			}

		/* Allocating pool memory to the higher layer data structure type. */	
		hld_ptr = (WlanT_Hld_List_Elem *) op_prg_pmo_alloc (hld_pmh);
	
		/* Generate error message and abort simulation if no memory left for data received from higher layer.	*/
		if (hld_ptr == OPC_NIL)
			{
			wlan_mac_error ("No more memory left to assign for data received from higher layer", OPC_NIL, OPC_NIL);
			}
		
		/* Set up dummy element to see if any more data for station currently being polled */
		if (poll_index > -1)
			hld_ptr->destination_address = polling_list [poll_index];
		else 
			hld_ptr->destination_address = -1;
		
		/* Set search bound for pcf higher layer data queue */
		list_high_index = op_prg_list_size (cfpd_list_ptr);
		list_low_index = 0;

		/* If a poll fail count reached the max poll fail count or		 */
		/* the previous poll was successful and no more data from this   */
		/* station and last data tx was successful and no more           */
		/* fragments exist and no more data exist in the hlk queue       */
		/* for this station then next station will start transmission	 */
		if ((poll_fail_count > max_poll_fails) ||
			(((poll_fail_count == 0) && 
			 (wlan_flags->more_data == OPC_BOOLINT_DISABLED) &&
			 (wlan_flags->more_frag == OPC_BOOLINT_DISABLED)) &&
			((pcf_retry_count == 0) && (op_sar_buf_size (pcf_frag_buffer_ptr) == 0) &&
			(op_prg_list_elem_find(cfpd_list_ptr, wlan_hld_list_elem_add_comp, hld_ptr, 
			&list_low_index, &list_high_index) == OPC_NIL))))
			{
			/* Increment polling index to next user 		  */
			poll_index++;
			
			/* Poll has reached the specified limit. */
			/* Updated the relevant variables        */
			if (poll_fail_count > max_poll_fails)
				{
				wlan_flags->pcf_side_traf = OPC_BOOLINT_DISABLED;
				wlan_flags->active_poll = OPC_BOOLINT_DISABLED;
			
				/*Reset the  retry count and drop of packet */
				pcf_retry_count = retry_limit;
				wlan_pcf_frame_discard ();
				
				/* Initialize the poll fail count           */
				poll_fail_count = 0;
				}
			}
			
		/* Re init dummy element for new poll index */
		hld_ptr->destination_address = polling_list [poll_index];

		/* Set destination address */				
		destination_addr = hld_ptr->destination_address;
		

		/* All the stations from the polling list has finished 	*/
		/* transmission. End the CFP.							*/
		if (poll_index == poll_list_size)
			{
			/* Completed polling list, transmit ack if necessary and end CFP */
			if (fresp_to_send == WlanC_Ack) 
				wlan_prepare_frame_to_send (WlanC_Cf_End_A);
			else 
				wlan_prepare_frame_to_send (WlanC_Cf_End);

			op_prg_mem_free (hld_ptr);
			
			FOUT;
			}
					
		/* First check if this is a retry */
		else if (pcf_retry_count != 0)
			{
			/* Set type to last frame type and send frame */
			op_pk_nfd_access (wlan_pcf_transmit_frame_copy_ptr, "Type", &type);

			wlan_prepare_frame_to_send (type);
			
			FOUT;
			}
		
		/* Check if fragmenetation buffer is empty and if there is any data to send */
	   	/* to this station.  If no data,  send  ack / poll as needed.		 		*/ 
		else if ((op_sar_buf_size (pcf_frag_buffer_ptr) == 0) && 
				(op_prg_list_elem_find(cfpd_list_ptr, wlan_hld_list_elem_add_comp, hld_ptr, 
				&list_low_index, &list_high_index) == OPC_NIL))
			{
			/* Set active poll flag since poll will be transmitted*/
			wlan_flags->active_poll = OPC_BOOLINT_ENABLED;

			/* If the AP has a pending ACK to transmit, send Ack-CF poll frame. */
			/* If no pending ACK for this station transmit the poll frame		*/
			if (fresp_to_send == WlanC_Ack) 
				{
				wlan_prepare_frame_to_send (WlanC_Cf_A_P);
				}
			else
				{
				wlan_prepare_frame_to_send (WlanC_Cf_Poll);
				}
		
			op_prg_mem_free (hld_ptr);
			
			FOUT;
			}

		/* If we've come this far, there must be data for this user.		*/
		/* If the fragmentation buffer is empty, get a new packet and 	 	*/
		/* setup fragmentation buffer.  Tx of frame is queued outside		*/
		/* this else if.													*/
		else if (op_sar_buf_size (pcf_frag_buffer_ptr) == 0) 
			{
			do
				{
				/* Get next packet for transmission from the higher layer queue */
				hld_ptr = (WlanT_Hld_List_Elem*) op_prg_list_access (cfpd_list_ptr, pcf_queue_offset);

				/* Make sure destination address matches polling address */
				if (hld_ptr->destination_address != polling_list 	[poll_index])
					{
					/* A packet must have been inserted into the queue by the 	*/
					/* upper layers after I started polling for a lower 		*/
					/* address.  Increment an offset to track packets at the 	*/
					/* head of the queue that have missed their opportunity 	*/
					/* to transmit this CFP.  Restore the packet to the 		*/
					/* point where it was stored, and get the next packet for 	*/
					/* transmission.											*/
					pcf_queue_offset++;
				
					if (pcf_queue_offset > list_high_index)
						wlan_mac_error ("Polling routine error. \n Destination not on list.", OPC_NIL, OPC_NIL);
					}
				} 
			while (hld_ptr->destination_address != polling_list [poll_index] );
			
			/* Remove packet from higher layer queue. */	
			hld_ptr = (WlanT_Hld_List_Elem *) op_prg_list_remove (cfpd_list_ptr, pcf_queue_offset);

			/* Setting destination address state variable	*/				
			destination_addr = hld_ptr->destination_address;

			/* Determine packet size - required to determine fragmentation	*/
			packet_size_pcf = op_pk_total_size_get (hld_ptr->pkptr);
	
			/* Updating the total packet size of the higher layer buffer.	*/
			total_cfpd_size = total_cfpd_size - packet_size_pcf;

			/* Packet seq number modulo 4096 counter.	*/
			packet_seq_number = (packet_seq_number + 1) % 4096;  

			/* Packet fragment number is initialized.	*/	
			packet_frag_number = 0;				
						
			/* Packet needs to be fragmented if it is more than			*/
			/* fragmentation threshold, provided fragmentation is		*/
			/* enabled. Broadcast packets are not fragmented regardless	*/
			/* of their sizes.											*/
			if (frag_threshold != -1 && destination_addr >= 0 && packet_size_pcf > (frag_threshold * 8))
				{
				/* Determine number of fragments for the packet	*/
				/* and the size of the last fragment			*/							
				pcf_num_fragments =  (int) (packet_size_pcf / (frag_threshold * 8));
				pcf_remainder_size = packet_size_pcf - (pcf_num_fragments * frag_threshold * 8);

				/* If the remainder size is non zero it means that the	*/
				/* last fragment is fractional but since the number 	*/
				/* of fragments is a whole number we need to transmit	*/	
				/* one additional fragment to ensure that all of the	*/
				/* data bits will be transmitted						*/
				if (pcf_remainder_size != 0)
					{
					pcf_num_fragments = pcf_num_fragments + 1;									 
					}
				}
			else
				{			
				/* If no fragments needed then number of	*/
				/* packets to be transmitted is set to 1	*/								
				pcf_num_fragments = 1;
				pcf_remainder_size = packet_size_pcf;
				}

			/* Storing Data packet id for debugging purposes.	*/			
			pcf_pkt_in_service = op_pk_id (hld_ptr->pkptr);		

			/* Insert packet to fragmentation buffer	*/					
			op_sar_segbuf_pk_insert (pcf_frag_buffer_ptr, hld_ptr->pkptr, 0);

			/* Computing packet duration in the queue in seconds	*/
			/* and reporting it to the statistics					*/
			pkt_tx_time = (current_time - hld_ptr->time_rcvd);

			/* Printing out information to ODB.	*/
			if (wlan_trace_active == OPC_TRUE)
				{
				sprintf (msg_string, "Data packet %d is removed from pcf queue", pcf_pkt_in_service);
				sprintf	(msg_string1, "The queueing delay for data packet %d is %fs", 	
							pcf_pkt_in_service, pkt_tx_time);	
				op_prg_odb_print_major (msg_string, msg_string1, OPC_NIL);
				}

			/* Store the arrival time of the pcf packet.	*/
			receive_time_pcf = hld_ptr->time_rcvd;
			
			/* Freeing up allocated memory for the data packet removed from the higher layer queue.	*/
			op_prg_mem_free (hld_ptr);
	
			/* Set up retry limit - no RTS so always short */
			retry_limit = short_retry_limit;
			}
			
		/* Set active poll flag since poll will be transmitted*/
		wlan_flags->active_poll = OPC_BOOLINT_ENABLED;

		/* Time to transmit fragment - Retrys happen automatically */
		if (fresp_to_send == WlanC_Ack) 
			wlan_prepare_frame_to_send (WlanC_Data_A_P);
		else  
			wlan_prepare_frame_to_send (WlanC_Data_Poll);
		}
	else
		{

		/* The order of else if statements here is very important, as	*/
		/* the code uses it to enforce the proper premption of various	*/
		/* valid frame sequences while preventing the premtion of others*/

		/* If not PCF, an Ack needs to be sent for the data */
		/* prepare Ack for transmission	                    */
		if ((wlan_flags->polled == OPC_BOOLINT_DISABLED) && 
			(fresp_to_send == WlanC_Ack))
			{
			wlan_prepare_frame_to_send (fresp_to_send);
			
			/* Break the routine once Ack is prepared to tranmsit */
			FOUT;
			}

		/* If Time to send beacon, takes priority over other sequences	*/
		else if ((wlan_flags->tx_beacon == OPC_BOOLINT_ENABLED) && 
			(op_sar_buf_size (fragmentation_buffer_ptr) == 0))
			{
			/* Reset any pending responses since beacon will terminate sequence anyway */
			fresp_to_send = WlanC_None;

			/* Prepare beacon frame to be transmitted */
			wlan_prepare_frame_to_send (WlanC_Beac);
		
			/* Break the routine once beacon prepared to tranmsit */
			FOUT;
			}

		/* DCF Transmission processing */
		
		/* If Ack and Cts needs to be sent then prepare the appropriate	*/
		/* frame type for transmission									*/
		else if ((fresp_to_send == WlanC_Cts) || 
			(fresp_to_send == WlanC_Ack && wlan_flags->polled == OPC_BOOLINT_DISABLED))
			{
			wlan_prepare_frame_to_send (fresp_to_send);
			
			/* Break the routine if Cts or Ack is already prepared to tranmsit */
			FOUT;
			}
	
		/* If it is a retransmission then check which type of frame needs to be	*/
		/* retransmitted and then prepare and transmit that frame.				*/
		else if (retry_count != 0)
			{
			/* If the last frame unsuccessfully transmitted was Rts then transmit it again.	*/
			if (last_frametx_type == WlanC_Rts && wlan_flags->rts_sent == OPC_BOOLINT_DISABLED && 
				wlan_flags->polled == OPC_BOOLINT_DISABLED)
				{
				/* Retransmit the Rts frame.	*/
				wlan_prepare_frame_to_send (WlanC_Rts);
				}

			/* For the retransmission of data frame first check whether RTS needs to be sent or not.	*/
			/* If it RTS needs to be sent and it is not already sent then first transmit RTS and then	*/
			/* transmit data frame.																		*/
			else if (last_frametx_type == WlanC_Data && wlan_flags->polled == OPC_BOOLINT_DISABLED)
				{
				if ((rts_threshold != -1) && (wlan_flags->rts_sent == OPC_BOOLINT_DISABLED) && 
					(op_pk_total_size_get (wlan_transmit_frame_copy_ptr) - 
					 plcp_overhead_data * operational_speed - WLANC_MSDU_HEADER_SIZE > 8 * rts_threshold))
					{
					/* Retransmit the RTS frame to again contend for the data .	*/
					wlan_prepare_frame_to_send (WlanC_Rts);		
					}
				else
					{
					wlan_prepare_frame_to_send (WlanC_Data);
					}
				}
			else
				{
				/* We continue with the retransmission process. Either we have	*/
				/* received the expected CTS for our last RTS before and now we	*/
				/* can retransmit our data frame, or we moved from DCF period	*/
				/* into PCF period and have been polled by the AP for			*/
				/* transmission. In case of PCF, also check whether we have an	*/
				/* ACK to append to our data packet.							*/
				if (fresp_to_send == WlanC_Ack && wlan_flags->polled == OPC_BOOLINT_ENABLED)
					wlan_prepare_frame_to_send (WlanC_Data_Ack);
				else 
					wlan_prepare_frame_to_send (WlanC_Data);
				}
			
			FOUT;
			}

		/* If higher layer queue is not empty then dequeue a packet	*/
		/* from the higher layer and insert it into fragmentation 	*/
		/* buffer check whether fragmentation and Rts-Cts exchange 	*/
		/* is needed  based on thresholds							*/
		/* Check if fragmenetation buffer is empty. If it is empty  */
		/* then dequeue a packet from the higher layer queue.		*/ 
		else if ((op_prg_list_size (hld_list_ptr) != 0) && (op_sar_buf_size (fragmentation_buffer_ptr) == 0))
			{
	
			/* If rts is already sent then transmit data otherwise	*/
			/* check if rts needs to be sent or not.				*/
			if (wlan_flags->rts_sent == OPC_BOOLINT_DISABLED)
				{
				/* Remove packet from higher layer queue. */
				hld_ptr = (WlanT_Hld_List_Elem *) op_prg_list_remove (hld_list_ptr, 0);
			
				/* Update the higher layer queue size statistic.				*/
				op_stat_write (hl_packets_rcvd, (double) (op_prg_list_size (hld_list_ptr)));
			
				/* Determine packet size to determine later wether fragmentation	*/
				/* and/or rts-cts exchange is needed.								*/
				packet_size_dcf = op_pk_total_size_get (hld_ptr->pkptr);

				/* Updating the total packet size of the higher layer buffer.	*/
				total_hlpk_size = total_hlpk_size - packet_size_dcf;

				/* Setting destination address state variable	*/				
				destination_addr = hld_ptr->destination_address;
						
				/* Packet seq number modulo 4096 counter.	*/
				packet_seq_number = (packet_seq_number + 1) % 4096;  
				
				/* Packet fragment number is initialized.	*/
				packet_frag_number = 0;				
						
				/* Packet needs to be fragmented if it is more than			*/
				/* fragmentation threshold, provided fragmentation is		*/
				/* enabled. Broadcast packets are not fragmented regardless	*/
				/* of their sizes.											*/
				if (frag_threshold != -1 && destination_addr >= 0 && packet_size_dcf > (frag_threshold * 8))
					{
					/* Determine number of fragments for the packet	*/
					/* and the size of the last fragment			*/							
					num_fragments =  (int) (packet_size_dcf / (frag_threshold * 8));
					remainder_size = packet_size_dcf - (num_fragments * frag_threshold * 8);

					/* If the remainder size is non zero it means that the	*/
					/* last fragment is fractional but since the number 	*/
					/* of fragments is a whole number we need to transmit	*/
					/* one additional fragment to ensure that all of the	*/
					/* data bits will be transmitted						*/
					if (remainder_size != 0)
						{
						num_fragments = num_fragments + 1;									 
						}
					}
				else
					{			
					/* If no fragments needed then number of	*/
					/* packets to be transmitted is set to 1	*/								
					num_fragments = 1;
					remainder_size = packet_size_dcf;
					}

				/* Storing Data packet id for debugging purposes.	*/			
				pkt_in_service = op_pk_id (hld_ptr->pkptr);		
				
				/* Insert packet to fragmentation buffer	*/					
				op_sar_segbuf_pk_insert (fragmentation_buffer_ptr, hld_ptr->pkptr, 0);

				/* Computing packet duration in the queue in seconds	*/
				/* and reporting it to the statistics					*/
				pkt_tx_time = (current_time - hld_ptr->time_rcvd);

				/* Printing out information to ODB.	*/
				if (wlan_trace_active == OPC_TRUE)
					{
					sprintf (msg_string, "Data packet %d is removed from higher layer buffer", pkt_in_service);
					sprintf	(msg_string1, "The queueing delay for data packet %d is %fs", 	
						pkt_in_service, pkt_tx_time);	
					op_prg_odb_print_major (msg_string, msg_string1, OPC_NIL);
					}

				/* Store the arrival time of the packet.	*/
				receive_time_dcf = hld_ptr->time_rcvd;
			
				/* Freeing up allocated memory for the data packet removed from the higher layer queue.	*/
				op_prg_mem_free (hld_ptr);

				/* Send rts if RTS is enabled and packet size is more than RTS threshold.	*/
				/* No RTS message is sent for broadcast packets regradless of their sizes.	*/
				if (rts_threshold != -1 && destination_addr >= 0 && packet_size_dcf > (rts_threshold * 8) &&
					wlan_flags->polled == OPC_BOOLINT_DISABLED)			
					{						 		
					retry_limit = long_retry_limit;
				
					/* Prepare Rts frame for transmission	*/
					wlan_prepare_frame_to_send (WlanC_Rts);
					
					/* Break the routine as Rts is already prepared	*/
					FOUT;
					}
				else
					{
					retry_limit = short_retry_limit;
					}
				}
			}
		
		/* Prepare data frame to transmit. First check whether the station	*/
		/* has been polled (if it is in CFP).								*/
		if (wlan_flags->polled == OPC_BOOLINT_ENABLED)
			{
			/* If there is no data to send select frame response			*/
			/* accordingly if we need to send an ACK back.					*/
			if (op_sar_buf_size (fragmentation_buffer_ptr) == 0)
				{
				if (fresp_to_send == WlanC_Ack) 
					wlan_prepare_frame_to_send (WlanC_Cf_Ack);
				else 
					wlan_prepare_frame_to_send (WlanC_Data_Null);
				}
			else  
				{
				/* We have data to respond to the poll. Also append the ACK	*/
				/* if we have an ACK to respond.							*/
				if (fresp_to_send == WlanC_Ack)
					wlan_prepare_frame_to_send (WlanC_Data_Ack);
				else 
					wlan_prepare_frame_to_send (WlanC_Data);
				}
			}		
		else
			{
			/* This is a normal DCF transmission. Prepare the frame for		*/
			/* for transmission.											*/
			wlan_prepare_frame_to_send (WlanC_Data);
			}
		}
	
	FOUT;
	}
		
static void 
wlan_prepare_frame_to_send (int frame_type)
	{
	char							msg_string [120];
	Packet*							seg_pkptr;
	int								protocol_type = -1;	
	int								tx_datapacket_size;
	int								type;
	int								outstrm_to_phy;
	double							duration, mac_delay;
	WlanT_Data_Header_Fields*		pk_dhstruct_ptr;
	WlanT_Control_Header_Fields*	pk_chstruct_ptr;
	WlanT_Beacon_Body_Fields*		pk_bbstruct_ptr;
	Packet*							wlan_transmit_frame_ptr;
	
	/** Prepare frames to transmit by setting appropriate fields in the 	**/
    /** packet format for Data,Cts,Rts or Ack.  If data or Rts packet needs **/
    /** to be retransmitted then the copy of the packet is resent.          **/
	FIN (wlan_prepare_frame_to_send (int frame_type));

	outstrm_to_phy = LOW_LAYER_OUT_STREAM_CH1;

	/* It this is a CP period and the frame to be transmitted is a data/ACK.	*/
	if ((wlan_flags->pcf_active == OPC_BOOLINT_DISABLED) && 
	   ((frame_type == WlanC_Data) || (frame_type == WlanC_Data_Ack)))
		{
		/* Choose the output stream based on the operational speed.				*/
		if (operational_speed == 2000000)
			{
			outstrm_to_phy = LOW_LAYER_OUT_STREAM_CH2;
			}
		else if (operational_speed == 5500000)
			{
			outstrm_to_phy = LOW_LAYER_OUT_STREAM_CH3;
			}
		else if (operational_speed == 11000000)
			{
			outstrm_to_phy = LOW_LAYER_OUT_STREAM_CH4;
			}

		/* Set the variable which keeps track of the last transmitted frame.	*/
		last_frametx_type = (WlanT_Mac_Frame_Type) frame_type;
					
		/* If it is a retransmission of a packet. Obtain the frame from the 	*/
		/* the copy pointer which was stored during the previous transmission	*/
		if ((retry_count > 0) && (wlan_transmit_frame_copy_ptr != OPC_NIL))
			{
			/* If it is a retransmission then just transmit the previous frame	*/			
			wlan_transmit_frame_ptr = op_pk_copy (wlan_transmit_frame_copy_ptr);

			/* Reset header type in case Ack status has changed for frame */
			op_pk_nfd_set (wlan_transmit_frame_ptr, "Type", frame_type);

			/* If retry count is non-zero means that the frame is a */
			/* retransmission of the last transmitted frame			*/
			op_pk_nfd_access (wlan_transmit_frame_ptr, "Wlan Header", &pk_dhstruct_ptr);
			pk_dhstruct_ptr->retry = 1;
			
			/* Reset more_data bit in case queue status has changed since last transmission			*/
			/* If this STA has been polled, and there are additional packets remaining				*/
			if ((wlan_flags->polled == OPC_BOOLINT_ENABLED) && (op_prg_list_size (hld_list_ptr) != 0))
				{
				/* Set more data bit to tell AP that STA has more packets */
				pk_dhstruct_ptr->more_data = 1;
				}

			/* Printing out information to ODB.	*/
			if (wlan_trace_active == OPC_TRUE)
				{
				sprintf (msg_string, "Data fragment %d for packet %d is retransmitted",
					     pk_dhstruct_ptr->fragment_number, pkt_in_service);							
				op_prg_odb_print_major (msg_string, OPC_NIL);
				}					

			/* Calculate nav duration till the channel will be occupied by 	*/
			/* station. The duration is SIFS time plus the ack frame time  	*/
			/* which the station needs in response to the data frame.		*/		
			duration = sifs_time + plcp_overhead_control + WLAN_ACK_DURATION;		

			/* Since the number of fragments for the last transmitted frame is	*/
			/* already decremented, there will be more fragments to transmit  	*/
			/* if number of fragments is more than zero.					  	*/
			if (num_fragments != 1)	
				{
				/* If more fragments need to be transmitted then the station 	*/
				/* need to broadcast the time until the receipt of the       	*/
				/* the acknowledgement for the next fragment. 224 bits (header	*/
				/* size) is the length of the control fields in the data  		*/
				/* frame and needs to be accounted in the duration calculation	*/
				duration = 2 * duration + sifs_time + plcp_overhead_data + 
						   ((frag_threshold * 8) + WLANC_MSDU_HEADER_SIZE) / operational_speed;
				}

			/* Set the type of the expected response to "ACK".	*/			
			expected_frame_type = WlanC_Ack;
			
			/* Station update its own nav_duration during CP   	*/
			/* NAV should be updated only during the CP period 	*/
			/* During CFP NAV duration is updated only during	*/
			/* the transmission of the beacon frames			*/
			if (cfp_ap_medium_control == OPC_BOOLINT_DISABLED)
				nav_duration = current_time + duration + (double) (op_pk_total_size_get (wlan_transmit_frame_ptr)) / operational_speed ;
			}
		else
			{
			/* Creating transmit data packet type.							*/
			wlan_transmit_frame_ptr = op_pk_create_fmt ("wlan_mac");
				
			/* Add some bulk to the packet to model the transmission delay	*/
			/* of PLCP fields accurately which are always transmitted at	*/
			/* 1 Mbps regardless of the actual data rate used for data		*/
			/* frames.														*/
			//op_pk_bulk_size_set (wlan_transmit_frame_ptr, plcp_overhead_data * operational_speed - WLAN_DEFAULT_PLCP_OVERHEAD);
		
			/* Prepare data frame fields for transmission.					*/		
			pk_dhstruct_ptr = wlan_mac_pk_dhstruct_create ();

			type = (WlanT_Mac_Frame_Type) frame_type;
			
			pk_dhstruct_ptr->retry = 0;				
			pk_dhstruct_ptr->order = 1;
			pk_dhstruct_ptr->sequence_number = packet_seq_number;

			/* Calculate nav duration till the channel will be occupied by  */
			/* station. The duration is SIFS time plus the ack frame time   */
			/* which the station needs in response to the data frame.		*/		
			duration = sifs_time + plcp_overhead_control + WLAN_ACK_DURATION;

			/* If there is more than one fragment to transmit and there are  	*/
			/* equal sized fragments then remove fragmentation threshold size	*/
			/* length of data from the buffer for transmission.					*/
			if  ((num_fragments > 1) || (remainder_size == 0))
				{
				/* Remove next fragment from the fragmentation buffer for 	*/
				/* transmission and set the appropriate fragment number.  	*/
				seg_pkptr = op_sar_srcbuf_seg_remove (fragmentation_buffer_ptr, frag_threshold * 8);
			
				/* Indicate in transmission frame that more fragments need to be sent	*/
				/* if more than one fragments are left								 	*/
				if (num_fragments != 1)	
					{
					pk_dhstruct_ptr->more_frag = 1;
				
					/* If more fragments need to be transmitted then the station	*/
					/* need to broadcast the time until the receipt of the       	*/
					/* the acknowledgement for the next fragment. 224 bits (header	*/
					/* size) is the length of control fields in the data frame  	*/
					/* and need to be accounted for in the duration calculation		*/
					duration = 2 * duration + sifs_time + plcp_overhead_data + 
					           ((frag_threshold * 8) + WLANC_MSDU_HEADER_SIZE) / operational_speed;
					}
				else
					{
					/* If no more fragments to transmit then set more fragment field to be 0 */
					pk_dhstruct_ptr->more_frag = 0;
					}
						
				/* Set fragment number in packet field	 */
				pk_dhstruct_ptr->fragment_number = packet_frag_number ;

				/* Printing out information to ODB.	*/
				if (wlan_trace_active == OPC_TRUE)
					{
					sprintf (msg_string, "Data fragment %d for packet %d is transmitted",packet_frag_number, pkt_in_service);							
					op_prg_odb_print_major (msg_string, OPC_NIL);
					}

				/* Setting packet fragment number for next fragment to be transmitted */
				packet_frag_number = packet_frag_number + 1;    	
				}
			else
				{
				/* Remove last fragments (if any left) from the fragmentation buffer for */
				/* transmission and disable more fragmentation bit.				         */
												
				seg_pkptr = op_sar_srcbuf_seg_remove (fragmentation_buffer_ptr, remainder_size);					
			
				pk_dhstruct_ptr->more_frag = 0;
				
				/* Printing out information to ODB.	*/
				if (wlan_trace_active == OPC_TRUE)
					{
					sprintf (msg_string, "Data fragment %d for packet %d is transmitted",packet_frag_number, pkt_in_service);								
					op_prg_odb_print_major (msg_string, OPC_NIL);
					}

				pk_dhstruct_ptr->fragment_number = packet_frag_number;
				}	

			/* Setting the Header field structure.	*/

			/** if this is the CFPeriod and the STA has been polled **/
			/** then set the duration to the standard value			**/
			if (wlan_flags->polled == OPC_BOOLINT_ENABLED)
				{
				/* Duration should be set to 32768 during CFP.										*/
				duration = 32768.0;
				pk_dhstruct_ptr->duration  = duration;
				}
			else
				{
				/* This is the CP, so set duration field.											*/
				pk_dhstruct_ptr->duration  = duration;
				}
			
			pk_dhstruct_ptr->address1  = destination_addr;
			pk_dhstruct_ptr->address2  = my_address;

			/* In the BSS network the Data frame is going from AP to sta then fromds bit is set.	*/
    		if (ap_flag == OPC_BOOLINT_ENABLED)
				pk_dhstruct_ptr->fromds	 = 1;
			else
				pk_dhstruct_ptr->fromds	 = 0;

			/* if in the BSS network the Data frame is going from sta to AP then tods bit is set.	*/					
    		if ((bss_flag == OPC_BOOLINT_ENABLED) && 
				(ap_flag == OPC_BOOLINT_DISABLED) &&
				(ap_relay == OPC_BOOLINT_ENABLED))
				{
				pk_dhstruct_ptr->tods = 1;
				
				/* If Infrastructure BSS then the immediate destination will be Access point, which 	*/
				/* then forward the frame to the appropriate destination.								*/
				pk_dhstruct_ptr->address1 = ap_mac_address ;
				pk_dhstruct_ptr->address3 = destination_addr;
				}
			else
				{
				pk_dhstruct_ptr->tods = 0;
				}

			/* If this STA has been polled, and there are additional packets remaining				*/
			if ((wlan_flags->polled == OPC_BOOLINT_ENABLED) && (op_prg_list_size (hld_list_ptr) != 0))
				{
				/* Set more data bit to tell AP that STA has more packets */
				pk_dhstruct_ptr->more_data = 1;
				}
			
			/* If we are sending the first fragment of the data fragment for the first	*/
			/* time, then this is the end of media access duration, hence we must		*/
			/* update the media access delay statistics.								*/
			if (packet_size_dcf == op_pk_total_size_get (seg_pkptr) + op_sar_buf_size (fragmentation_buffer_ptr))
				{
				mac_delay = current_time - receive_time_dcf;
				op_stat_write (media_access_delay, mac_delay);
				op_stat_write (global_mac_delay_handle, mac_delay);
				}
			
			/* Populate the packet fields.								*/
			op_pk_nfd_set (wlan_transmit_frame_ptr, "Type", type);
			op_pk_nfd_set (wlan_transmit_frame_ptr, "Accept", OPC_TRUE);
			op_pk_nfd_set (wlan_transmit_frame_ptr, "Data Packet ID", pkt_in_service);
				
			/* Set the frame control field and nav duration.		   	*/
			op_pk_nfd_set (wlan_transmit_frame_ptr, "Wlan Header", pk_dhstruct_ptr,	
			wlan_mac_pk_dhstruct_copy, wlan_mac_pk_dhstruct_destroy, sizeof (WlanT_Data_Header_Fields));

			/* The actual data is placed in the Frame Body field.		*/
			op_pk_nfd_set (wlan_transmit_frame_ptr, "Frame Body", seg_pkptr);

			/* Only expect Acknowledgement for directed frames.			*/
			if (destination_addr < 0)
				{
				expected_frame_type = WlanC_None;
				}
			else
				{
				/* Ack frame is expected in response to data frame.		*/
				expected_frame_type = WlanC_Ack;
				}

			/* Make copy of the frame before transmission -- make sure	*/
			/* that a packet destined for broadcast addresses is not	*/
			/* copied as that would never to destroyed (due to unACKing	*/
			/* nature of broadcast traffic).							*/
			if (destination_addr != -1)
				wlan_transmit_frame_copy_ptr = op_pk_copy (wlan_transmit_frame_ptr);
			else
				wlan_transmit_frame_copy_ptr = (Packet *) OPC_NIL;

		    /* Station update of its own nav_duration.					*/
			if (cfp_ap_medium_control == OPC_BOOLINT_DISABLED)
				nav_duration = current_time + duration + (double) (op_pk_total_size_get (wlan_transmit_frame_ptr)) / operational_speed ;
			}
		
		/* Reporting total number of bits in a data frame.				*/
		op_stat_write (data_traffic_sent_handle_inbits, (double) op_pk_total_size_get (wlan_transmit_frame_ptr));
		op_stat_write (data_traffic_sent_handle_inbits, 0.0);
		
		/* Update data traffic sent stat when the transmission is complete.		*/
		op_stat_write (data_traffic_sent_handle, 1.0);
		op_stat_write (data_traffic_sent_handle, 0.0);
		
		/* We can be sending this data message as a response to a CTS message	*/
		/* we received. Therefore reset the "frame respond to send" variable.	*/
		fresp_to_send = WlanC_None;
		
		/* If there is nothing in the higher layer data queue and fragmentation buffer	*/
		/* then disable the data frame flag which will indicate to the station to wait	*/
		/* for the higher layer packet.													*/
		if (op_prg_list_size (hld_list_ptr) == 0 && op_sar_buf_size (fragmentation_buffer_ptr) == 0)
			wlan_flags->data_frame_to_send = OPC_BOOLINT_DISABLED;			
		}

	/* If this is a contention free period and need to send a data/ack/poll.	*/
	else if ((wlan_flags->pcf_active == OPC_BOOLINT_ENABLED) &&
			((frame_type == WlanC_Data)	|| (frame_type == WlanC_Data_Ack) || 
			(frame_type == WlanC_Data_Poll)	|| (frame_type == WlanC_Data_A_P))) 
		{
		/* Preserve the frame type being transmitted */
		last_frametx_type = (WlanT_Mac_Frame_Type) frame_type;
		
		/* If this is PCF active and data frames corresponding to poll or ack's.*/
		if (operational_speed == 2000000)
			{
			outstrm_to_phy = LOW_LAYER_OUT_STREAM_CH2;
			}
		else if (operational_speed == 5500000)
			{
			outstrm_to_phy = LOW_LAYER_OUT_STREAM_CH3;
			}
		else if (operational_speed == 11000000)
			{
			outstrm_to_phy = LOW_LAYER_OUT_STREAM_CH4;
			}

		/* Set active poll flag if this is a poll frame */
		if ((frame_type == WlanC_Data_Poll)	|| (frame_type == WlanC_Data_A_P))
			{
			wlan_flags->active_poll = OPC_BOOLINT_ENABLED;
			}
				
		/* If it is a retransmission of a packet then no need 	*/
        /* to prepare data frame.							    */
		if (pcf_retry_count == 0)
			{
			/* Creating transmit data packet type.							*/
			wlan_transmit_frame_ptr = op_pk_create_fmt ("wlan_mac");
				
			/* Add some bulk to the packet to model the transmission delay	*/
			/* of PLCP fields accurately which are always transmitted at	*/
			/* 1 Mbps regardless of the actual data rate used for data		*/
			/* frames.														*/
			//op_pk_bulk_size_set (wlan_transmit_frame_ptr, plcp_overhead_data * operational_speed - WLAN_DEFAULT_PLCP_OVERHEAD);
		
			/* Prepare data frame fields for transmission.					*/		
			pk_dhstruct_ptr = wlan_mac_pk_dhstruct_create ();

			pk_dhstruct_ptr->retry = 0;				
			pk_dhstruct_ptr->order = 1;
			pk_dhstruct_ptr->sequence_number = packet_seq_number;

			/* If there is more than one fragment to transmit and there are  	*/
			/* equal sized fragments then remove fragmentation threshold size	*/
			/* length of data from the buffer for transmission.					*/
			if  ((pcf_num_fragments > 1) || (pcf_remainder_size == 0))
				{
				/* Remove next fragment from the fragmentation buffer for 	*/
				/* transmission and set the appropriate fragment number.  	*/
				seg_pkptr = op_sar_srcbuf_seg_remove (pcf_frag_buffer_ptr, frag_threshold * 8);
		
				/* Indicate in transmission frame that more fragments need to be sent	*/
				/* if more than one fragments are left								 	*/
				if (pcf_num_fragments != 1)	
					{
					pk_dhstruct_ptr->more_frag = 1;
					}
				else
					{
					/* If no more fragments to transmit then set more fragment field to be 0 */
					pk_dhstruct_ptr->more_frag = 0;
					}
						
				/* Set fragment number in packet field	 */
				pk_dhstruct_ptr->fragment_number = packet_frag_number;

				/* Printing out information to ODB.	*/
				if (wlan_trace_active == OPC_TRUE)
					{
					sprintf (msg_string, "Data fragment %d for packet %d is transmitted",
						packet_frag_number, pcf_pkt_in_service);							
					op_prg_odb_print_major (msg_string, OPC_NIL);
					}

				/* Setting packet fragment number for next fragment to be transmitted */
				packet_frag_number = packet_frag_number + 1;    	
				}
			else
				{
				/* Remove last fragments (if any left) from the fragmentation buffer for */
				/* transmission and disable more fragmentation bit.				         */
				seg_pkptr = op_sar_srcbuf_seg_remove (pcf_frag_buffer_ptr, pcf_remainder_size);					

				pk_dhstruct_ptr->more_frag = 0;

				/* Printing out information to ODB.	*/
				if (wlan_trace_active == OPC_TRUE)
					{
					sprintf (msg_string, "Data fragment %d for packet %d is transmitted",
							 packet_frag_number, pcf_pkt_in_service);								
					op_prg_odb_print_major (msg_string, OPC_NIL);
					}

				pk_dhstruct_ptr->fragment_number = packet_frag_number;
				}	

			/* Set duration feild */
			/* During CFP the duration feild should read 32768. (Section 7.1.3.2 of spec) */
			duration = 32768.0;

			/* Setting the Header field structure.	*/
			pk_dhstruct_ptr->duration  = duration;
			pk_dhstruct_ptr->address1  = destination_addr;
			pk_dhstruct_ptr->address2  = my_address;

			/* In the BSS network the Data frame is going from AP to sta then fromds bit is set.	*/
    		if (ap_flag == OPC_BOOLINT_ENABLED)
				{
				pk_dhstruct_ptr->fromds	 = 1;
				}
			else
				{
				pk_dhstruct_ptr->fromds	 = 0;
				}

			/* if in the BSS network the Data frame is going from sta to AP then tods bit is set.	*/					
    		if ((bss_flag == OPC_BOOLINT_ENABLED) && (ap_flag == OPC_BOOLINT_DISABLED) &&
				(ap_relay == OPC_BOOLINT_ENABLED))
				{
				pk_dhstruct_ptr->tods = 1;

				/* If Infrastructure BSS then the immediate destination will be Access point, which 	*/
				/* then forward the frame to the appropriate destination.								*/
				pk_dhstruct_ptr->address1 = ap_mac_address;
				pk_dhstruct_ptr->address3 = destination_addr;
				}
			else
				{
				pk_dhstruct_ptr->tods = 0;
				}
	
			/* If we are sending the first fragment of the data fragment for the first	*/
			/* time, then this is the end of media access duration, hence we must		*/
			/* update the media access delay statistics.								*/
			if (packet_size_pcf == op_pk_total_size_get (seg_pkptr) + op_sar_buf_size (pcf_frag_buffer_ptr))
				{
				mac_delay = current_time - receive_time_pcf;
			
				op_stat_write (media_access_delay, mac_delay);
				op_stat_write (global_mac_delay_handle, mac_delay);
				}
			
			op_pk_nfd_set (wlan_transmit_frame_ptr, "Type", frame_type);
			op_pk_nfd_set (wlan_transmit_frame_ptr, "Accept", OPC_TRUE);
			op_pk_nfd_set (wlan_transmit_frame_ptr, "Data Packet ID", pcf_pkt_in_service);
				
			/* Set the frame control field.				*/
			op_pk_nfd_set (wlan_transmit_frame_ptr, "Wlan Header", pk_dhstruct_ptr,	
			wlan_mac_pk_dhstruct_copy, wlan_mac_pk_dhstruct_destroy, sizeof (WlanT_Data_Header_Fields));

			/* The actual data is placed in the Frame Body field	*/
			op_pk_nfd_set (wlan_transmit_frame_ptr, "Frame Body", seg_pkptr);

			/* Make copy of the frame before transmission	*/
			wlan_pcf_transmit_frame_copy_ptr = op_pk_copy (wlan_transmit_frame_ptr);
			}
		else
			{
			/* If it is a retransmission then just transmit the previous frame	*/			
			wlan_transmit_frame_ptr = op_pk_copy (wlan_pcf_transmit_frame_copy_ptr);

			/* If retry count is non-zero means that the frame is a */
			/* retransmission of the last transmitted frame			*/
			op_pk_nfd_access (wlan_transmit_frame_ptr, "Wlan Header", &pk_dhstruct_ptr);
			pk_dhstruct_ptr->retry = 1;

			/* Reset header type in case Ack status has changed for frame */
			op_pk_nfd_set (wlan_transmit_frame_ptr, "Type", frame_type);
			
			/* read back duration feild for debug stuff */
			duration =	pk_dhstruct_ptr->duration;

			/* Printing out information to ODB.	*/
			if (wlan_trace_active == OPC_TRUE)
				{
				sprintf (msg_string, "Data fragment %d for packet %d is retransmitted", 
						pk_dhstruct_ptr->fragment_number, pcf_pkt_in_service);							
				op_prg_odb_print_major (msg_string, OPC_NIL);
				}					
			}
			
		/* Reporting total number of bits in a data frame.   			*/
		op_stat_write (data_traffic_sent_handle_inbits, (double) op_pk_total_size_get (wlan_transmit_frame_ptr));
		op_stat_write (data_traffic_sent_handle_inbits, 0.0);

		/* Update data traffic sent stat when the transmission is		*/
		/* complete.													*/
		op_stat_write (data_traffic_sent_handle, 1.0);
		op_stat_write (data_traffic_sent_handle, 0.0);

		/* Only expect Acknowledgement for directed frames.				*/
		if (destination_addr < 0)
			expected_frame_type = WlanC_None;
		else
			/* ACK frame is expected in response to data frame.			*/
			expected_frame_type = WlanC_Ack;
		}
	
	/* Check if the frame type to be transmitted is a data null/cf acf/cf poll */
	else if ((frame_type == WlanC_Data_Null)|| (frame_type == WlanC_Cf_Ack)	|| 
			(frame_type == WlanC_Cf_Poll)	|| (frame_type == WlanC_Cf_A_P))
		{			
		/* Preserve the frame being transmitted */
		last_frametx_type = (WlanT_Mac_Frame_Type) frame_type;
		
		if (operational_speed == 2000000)
			{
			outstrm_to_phy = LOW_LAYER_OUT_STREAM_CH2;
			}
		else if (operational_speed == 5500000)
			{
			outstrm_to_phy = LOW_LAYER_OUT_STREAM_CH3;
			}
		else if (operational_speed == 11000000)
			{
			outstrm_to_phy = LOW_LAYER_OUT_STREAM_CH4;
			}

		/* Set active poll flag if this is a poll frame */
		if ((frame_type == WlanC_Cf_Poll)	|| (frame_type == WlanC_Data_A_P))
			{
			wlan_flags->active_poll = OPC_BOOLINT_ENABLED;
			}
		
		/* If it is a retransmission of a packet then no need 	*/
     	/* of preparing data frame.				*/
		if	(((frame_type == WlanC_Data_Null) || (frame_type == WlanC_Cf_Ack)) || 
			((pcf_retry_count == 0) && (poll_fail_count == 0)))
			{
			/* Creating transmit data packet type.							*/
			wlan_transmit_frame_ptr = op_pk_create_fmt ("wlan_mac");
				
			/* Add some bulk to the packet to model the transmission delay	*/
			/* of PLCP fields accurately which are always transmitted at	*/
			/* 1 Mbps regardless of the actual data rate used for data		*/
			/* frames.														*/
			//op_pk_bulk_size_set (wlan_transmit_frame_ptr, plcp_overhead_data * operational_speed - WLAN_DEFAULT_PLCP_OVERHEAD);
		
			/* Prepare data frame fields for transmission.					*/		
			pk_dhstruct_ptr = wlan_mac_pk_dhstruct_create ();

			pk_dhstruct_ptr->retry = 0;				
			pk_dhstruct_ptr->order = 1;
			pk_dhstruct_ptr->sequence_number = packet_seq_number;

			/* Set packet fragment fields  	*/
			pk_dhstruct_ptr->more_frag = 0;
			pk_dhstruct_ptr->fragment_number = 0;

			/* Set duration feild */
			/* During PCF the duration field should read 32768. (Section 7.1.3.2 of spec) */
			duration = 32768.0;

			/* Setting the Header field structure.	*/
			pk_dhstruct_ptr->duration  = duration;
			pk_dhstruct_ptr->address1  = destination_addr;
			pk_dhstruct_ptr->address2  = my_address;

			/* In the BSS network the Data frame is going from AP to sta then fromds bit is set.	*/
    		if (ap_flag == OPC_BOOLINT_ENABLED)
				{
				pk_dhstruct_ptr->fromds	 = 1;
				}
			else
				{
				pk_dhstruct_ptr->fromds	 = 0;
				}

			/* if in the BSS network the Data frame is going from sta to AP then tods bit is set.	*/					
    		if ((bss_flag == OPC_BOOLINT_ENABLED) && (ap_flag == OPC_BOOLINT_DISABLED))
				{
				pk_dhstruct_ptr->tods = 1;

				/* If Infrastructure BSS then the immediate destination will be Access point, which 	*/
				/* then forward the frame to the appropriate destination.								*/
				pk_dhstruct_ptr->address1 = ap_mac_address;
				pk_dhstruct_ptr->address3 = destination_addr;
				}
			else
				{
				pk_dhstruct_ptr->tods = 0;
				}
	
			op_pk_nfd_set (wlan_transmit_frame_ptr, "Type", frame_type);
			op_pk_nfd_set (wlan_transmit_frame_ptr, "Accept", OPC_TRUE);
			op_pk_nfd_set (wlan_transmit_frame_ptr, "Data Packet ID", pcf_pkt_in_service);
				
			/* Set the frame control field.				*/
			op_pk_nfd_set (wlan_transmit_frame_ptr, "Wlan Header", pk_dhstruct_ptr,	
			wlan_mac_pk_dhstruct_copy, wlan_mac_pk_dhstruct_destroy, sizeof (WlanT_Data_Header_Fields));

			/* Need to creat dummy "Frame Body" so use beacon frame and set size to zero */
			/* Create packet container for beacon body */
			seg_pkptr = op_pk_create_fmt ("wlan_beacon_body");
			op_pk_total_size_set (seg_pkptr, 0);

			/* The actual data is placed in the Frame Body field	*/
			op_pk_nfd_set (wlan_transmit_frame_ptr, "Frame Body", seg_pkptr);

			/* Make copy of the frame before transmission	*/
			if ((frame_type != WlanC_Data_Null) && (frame_type != WlanC_Cf_Ack))
				{
				wlan_pcf_transmit_frame_copy_ptr = op_pk_copy (wlan_transmit_frame_ptr);
				}
			}
		else 
			{
			/* If it is a retransmission then just transmit the previous frame	*/			
			wlan_transmit_frame_ptr = op_pk_copy (wlan_pcf_transmit_frame_copy_ptr);

			/* If retry count is non-zero means that the frame is a */
			/* retransmission of the last transmitted frame			*/
			op_pk_nfd_access (wlan_transmit_frame_ptr, "Wlan Header", &pk_dhstruct_ptr);
			pk_dhstruct_ptr->retry = 1;

			/* read back duration feild for debug stuff */
			duration =	pk_dhstruct_ptr->duration;
						
			/* Printing out information to ODB.	*/
			if (wlan_trace_active == OPC_TRUE)
				{
				sprintf (msg_string, "Data fragment %d for packet %d is retransmitted", 
						 pk_dhstruct_ptr->fragment_number, pcf_pkt_in_service);							
				op_prg_odb_print_major (msg_string, OPC_NIL);
				}					
			}

		/* Reporting total number of bits in a data frame.   			*/
		op_stat_write (ctrl_traffic_sent_handle_inbits, (double) op_pk_total_size_get (wlan_transmit_frame_ptr));
		op_stat_write (ctrl_traffic_sent_handle_inbits, 0.0);

		/* Update data traffic sent stat when the transmission is		*/
		/* complete.													*/
		op_stat_write (ctrl_traffic_sent_handle, 1.0);
		op_stat_write (ctrl_traffic_sent_handle, 0.0);
		
		/* No ACK expected for non-data poll frames but do expect some	*/
		/* type of Data frame in response.								*/
		if	((frame_type == WlanC_Cf_Poll)	|| (frame_type == WlanC_Cf_A_P))
			expected_frame_type = WlanC_Data;
		else
			expected_frame_type = WlanC_None;

		/* Once Ack is transmitted in response to Data frame then set	*/
		/* the frame response indicator to none frame as the response	*/
		/* is already generated.										*/
		fresp_to_send = WlanC_None;					
		}

	else if (frame_type == WlanC_Beac)
		{			
		last_frametx_type = (WlanT_Mac_Frame_Type) frame_type;

		wlan_flags->pcf_active = OPC_BOOLINT_ENABLED;

		/* Create packet container for beacon body */
		seg_pkptr = op_pk_create_fmt ("wlan_beacon_body");

		/* Create Beacon body */
		pk_bbstruct_ptr = wlan_mac_pk_bbstruct_create ();

		/* Timestamp should be set to reference 1st bit of timestamp in message at antenna (11.1.2.1) */
		/* To reduce processing, it is currently set for first bit of MAC frame at antenna  */
		/* (assuming no PHY delay) */
		pk_bbstruct_ptr->timestamp  = op_sim_time();
		pk_bbstruct_ptr->beacon_intv = beacon_int;
		
		/* if no PCF, No beacon starts a CFP */ 
		if (pcf_flag == OPC_BOOLINT_DISABLED)           
			{
			pk_bbstruct_ptr->cf_par.cfp_count = 1;   

			pk_bbstruct_ptr->cf_par.cfp_period = 0;    

			pk_bbstruct_ptr->cf_par.cfp_maxduration	= 0.0; 
			
			pk_bbstruct_ptr->cf_par.cfp_durremaining = 0.0;
			}
		else											
			/* PCF implmented */
			{
			pk_bbstruct_ptr->cf_par.cfp_count = 
				(((int) (current_time/beacon_int)+ cfp_offset) % cfp_prd);    	
			
			/* set CFP period */
			pk_bbstruct_ptr->cf_par.cfp_period 	= cfp_prd;   	
			
			/* Set CFP maximum duration*/
			pk_bbstruct_ptr->cf_par.cfp_maxduration	 = cfp_length;	

			/* if begining a cfp */
			if (pk_bbstruct_ptr->cf_par.cfp_count == 0)           	
				{ 							
				/* Find time remaining in current cfp */
				pk_bbstruct_ptr->cf_par.cfp_durremaining = 
					cfp_length - (current_time -((int) (current_time/beacon_int))*beacon_int); 
				}
			else 
				pk_bbstruct_ptr->cf_par.cfp_durremaining = 0;    /* No CFP so no time remaining */
			}
		
		/* Use data frame format for beacon frame since need frame body.	*/

		/* Creating transmit data packet type.								*/
		wlan_transmit_frame_ptr = op_pk_create_fmt ("wlan_mac");
		
		/* Adjust the packet size if necessary to model the PLCP overhead	*/
		/* accurately, which is physical layer technology dependent. The	*/
		/* default value is set for infra-red technology.					*/
		//if (phy_char_flag != WlanC_Infra_Red)
		//	op_pk_bulk_size_set (wlan_transmit_frame_ptr, plcp_overhead_control * WLAN_MAN_DATA_RATE - WLAN_DEFAULT_PLCP_OVERHEAD);
		
		/* Set destination address to broadcast since unicast not supported.*/
		destination_addr = -1;

		/* Prepare data frame fields for transmission.	*/		
		pk_dhstruct_ptr = wlan_mac_pk_dhstruct_create ();

		pk_dhstruct_ptr->retry = 0;				
		pk_dhstruct_ptr->order = 1;
		pk_dhstruct_ptr->sequence_number = packet_seq_number;

		/* During CFP the duration feild should read 32768. (Section 7.1.3.2 of spec) */
		/* During CP should read zero since broadcast (Section 7.2.3)  */
		if (pk_bbstruct_ptr->cf_par.cfp_count == 0)	
			duration = 32768.0;
		else 
			duration = 0.0;
				
		/* Setting the Header field structure.	*/
		pk_dhstruct_ptr->duration  = duration;
		pk_dhstruct_ptr->address1  = destination_addr; /* Always set for broadcast for now */
		pk_dhstruct_ptr->address2  = my_address;
		
		/* This value is checked at the receving end to see if this frame was intended for this bss id */
		pk_dhstruct_ptr->address3  = bss_id;

		/* Management frames (Beacon) never involve DS.	*/
		pk_dhstruct_ptr->fromds	 = 0;
		pk_dhstruct_ptr->tods    = 0;
	
		op_pk_nfd_set (wlan_transmit_frame_ptr, "Type", frame_type);
		op_pk_nfd_set (wlan_transmit_frame_ptr, "Accept", OPC_TRUE);

		/* Data Packet in service not really meaningful for beacon so set to -1 */
		op_pk_nfd_set (wlan_transmit_frame_ptr, "Data Packet ID", -1);
				
		/* Set the frame control field.				*/
		op_pk_nfd_set (wlan_transmit_frame_ptr, "Wlan Header", pk_dhstruct_ptr,	
		wlan_mac_pk_dhstruct_copy, wlan_mac_pk_dhstruct_destroy, sizeof (WlanT_Data_Header_Fields));

		/* If this is the start of the CFP, reset the NAV 				*/
		/* Any frame sequences in progress will be interrupted anyway.	*/
		if(pk_bbstruct_ptr->cf_par.cfp_count == 0)
			nav_duration = current_time;
		
		/* The beacon body is placed in the Packet container	*/
		op_pk_nfd_set (seg_pkptr, "Beacon Body", pk_bbstruct_ptr,	
		wlan_mac_pk_bbstruct_copy, wlan_mac_pk_bbstruct_destroy, sizeof (WlanT_Beacon_Body_Fields));

		/* The beacon body "packet" is placed in the Frame Body field	*/
		op_pk_nfd_set (wlan_transmit_frame_ptr, "Frame Body", seg_pkptr);
	
		/* Clear expected frame time since any existing valid frame sequences have been interupted anyway.	*/
		expected_frame_type = WlanC_None;

		/* Printing out information to ODB.	*/
		if (wlan_trace_active == OPC_TRUE)
			{
			op_prg_odb_print_major ("Beacon is being transmitted by the Acces Point", OPC_NIL);
			}
		
		/* Reporting total number of bits in a control frame.   		*/
		op_stat_write (ctrl_traffic_sent_handle_inbits, (double) op_pk_total_size_get (wlan_transmit_frame_ptr));
		op_stat_write (ctrl_traffic_sent_handle_inbits, 0.0);

		/* Update control traffic sent stat when the transmission is complete.	*/
		op_stat_write (ctrl_traffic_sent_handle, 1.0);
		op_stat_write (ctrl_traffic_sent_handle, 0.0);

		/* This Beacon frame we are sending may have paused the transmission	*/
		/* process of a regular frame. As a further case, it may have blocked	*/
		/* the "re-transmission" of this regular frame. If this is the			*/
		/* situation, then adjust the upper bound of contention window, so that	*/
		/* we don't increase it even further as a result of this Beacon's		*/
		/* transmission for the re-transmission of the regular packet.			*/
		if (retry_count != 0)
			max_backoff = (cw_min) / 2;
		
		/* Clear tx beacon flag.												*/
		wlan_flags->tx_beacon = OPC_BOOLINT_DISABLED;
		
		/* If there are no PCF enabled nodes in the network, schedule a CFP end */
		/* after SIFS duration.	And cancel the previously scheduled CFP end.	*/			
		if (poll_list_size == 0)
			{
			if (op_ev_valid (cfp_end_evh) == OPC_TRUE)
				op_ev_cancel (cfp_end_evh);
			
			cfp_end_evh = op_intrpt_schedule_self (current_time + sifs_time, WlanC_Cfp_End);
			}
		}

	else if (frame_type == WlanC_Rts)
		{		
		last_frametx_type = (WlanT_Mac_Frame_Type) frame_type;
		
		/* Creating Rts packet format type.									*/
		wlan_transmit_frame_ptr = op_pk_create_fmt ("wlan_control");

		/* Adjust the packet size to model the RTS message and the PLCP		*/
		/* overhead, which is physical layer technology dependent,			*/
		/* accurately. The default value for PLCP overhead is set for		*/
		/* infra-red technology.											*/
		if (phy_char_flag != WlanC_Infra_Red)
			op_pk_total_size_set (wlan_transmit_frame_ptr, plcp_overhead_control * WLAN_MAN_DATA_RATE + WLAN_RTS_LENGTH);
		else
			op_pk_total_size_set (wlan_transmit_frame_ptr, WLAN_DEFAULT_PLCP_OVERHEAD + WLAN_RTS_LENGTH);
		
		/* Initializing RTS frame fields.									*/
		pk_chstruct_ptr = wlan_mac_pk_chstruct_create ();
		
		/* Type of frame */
	   	type = WlanC_Rts;   						

		/* if in the infrastructure BSS network then the immediate receipient for the transmitting	*/
		/* station will always be an Access point. Otherwise the frame is directly sent to the 		*/
		/* final destination.																		*/
    	if ((bss_flag == OPC_BOOLINT_ENABLED) && (ap_flag == OPC_BOOLINT_DISABLED))
			{
			/* If Infrastructure BSS then the immediate destination will be Access point, which 	*/
			/* then forward the frame to the appropriate destination.								*/
			pk_chstruct_ptr->rx_addr = ap_mac_address;
			}
		else
			{
			/* Otherwise set the final destination address.	*/				   
			pk_chstruct_ptr->rx_addr = destination_addr;
			}

		/* Source station address.	*/
		pk_chstruct_ptr->tx_addr = my_address;

		/* Setting the Rts frame type.	*/
		op_pk_nfd_set (wlan_transmit_frame_ptr, "Type", type);

		/* Setting the accept field to true, meaning the frame is a good frame.	*/
		op_pk_nfd_set (wlan_transmit_frame_ptr, "Accept", OPC_TRUE);
				
		/* Setting the variable which keeps track of the last transmitted frame that needs response.	*/
		last_frametx_type = (WlanT_Mac_Frame_Type)type;
					
		/* Determining the size of the first data fragment or frame that need */
		/* to be transmitted following the Rts transmission.				  */				
		if (num_fragments > 1)
			{
			/* If there are more than one fragment to transmit then the */
			/* data segment of the first data frame will be the size of */
			/* fragmentation threshold. The total packet size will be   */
			/* data plus the overhead (which is 224 bits).				*/
			tx_datapacket_size = frag_threshold * 8 + WLANC_MSDU_HEADER_SIZE;
			}
		else
			/* If there is one data frame to transmit then the          */
			/* data segment of the first data frame will be the size of */
			/* the remainder computed earlier. The total packet size    */
			/* will be data plus the overhead (which is 224 bits).		*/
			{
			tx_datapacket_size = remainder_size + WLANC_MSDU_HEADER_SIZE;
			}

		/* Station is reserving channel bandwidth by using RTS frame, so    */
		/* in RTS the station will broadcast the duration it needs to send  */ 		 		
		/* one data frame and receive ACK for it. The total duration is the */
		/* the time required to transmit one data frame, plus one CTS frame */
		/* plus one ACK frame, and plus three SIFS intervals.				*/
		duration = WLAN_CTS_DURATION + WLAN_ACK_DURATION + ((double) tx_datapacket_size / operational_speed) + 
				   3 * sifs_time + 2 * plcp_overhead_control + plcp_overhead_data;            
		pk_chstruct_ptr->duration = duration;
				
		/* Setting RTS frame fields.	*/
		op_pk_nfd_set (wlan_transmit_frame_ptr, "Wlan Header", pk_chstruct_ptr, wlan_mac_pk_chstruct_copy, wlan_mac_pk_chstruct_destroy, sizeof (WlanT_Control_Header_Fields));				
				
		/* Station update of its own nav_duration	*/
		if (cfp_ap_medium_control == OPC_BOOLINT_DISABLED)
			nav_duration = current_time + duration + (double) (op_pk_total_size_get (wlan_transmit_frame_ptr)) / WLAN_MAN_DATA_RATE; 								 						
			
		/* CTS is expected in response to RTS.	*/						
		expected_frame_type = WlanC_Cts;

		/* Printing out information to ODB.	*/
		if (wlan_trace_active == OPC_TRUE)
			{
			sprintf (msg_string, "Rts is being transmitted for data packet %d", pkt_in_service);
			op_prg_odb_print_major (msg_string, OPC_NIL);
			}

		/* Reporting total number of bits in a control frame.   			*/
		op_stat_write (ctrl_traffic_sent_handle_inbits, (double) op_pk_total_size_get (wlan_transmit_frame_ptr));
		op_stat_write (ctrl_traffic_sent_handle_inbits, 0.0);

		/* Update control traffic sent stat when the transmission is complete	*/
		op_stat_write (ctrl_traffic_sent_handle, 1.0);
		op_stat_write (ctrl_traffic_sent_handle, 0.0);
		}
	
	else if (frame_type == WlanC_Cts)
		{
		if ((pcf_flag == OPC_BOOLINT_ENABLED) || (wlan_flags->pcf_active == OPC_BOOLINT_ENABLED))
			last_frametx_type = (WlanT_Mac_Frame_Type) frame_type;
				
		/** Preparing CTS frame in response to the received Rts frame.		*/
			
		/* Creating Cts packet format type.									*/
		wlan_transmit_frame_ptr = op_pk_create_fmt ("wlan_control");

		/* Adjust the packet size if necessary to model the PLCP overhead	*/
		/* accurately, which is physical layer technology dependent. The	*/
		/* default value is set for infra-red technology.					*/
		//if (phy_char_flag != WlanC_Infra_Red)
		//	op_pk_bulk_size_set (wlan_transmit_frame_ptr, plcp_overhead_control * WLAN_MAN_DATA_RATE - WLAN_DEFAULT_PLCP_OVERHEAD);

		/* Initializing Rts frame fields.									*/
		pk_chstruct_ptr = wlan_mac_pk_chstruct_create ();
		
		/* Type of frame.													*/
   		type = WlanC_Cts;

		/* Destination station address.										*/
		pk_chstruct_ptr->rx_addr = remote_sta_addr;
			
		/* Station is reserving channel bandwidth by using RTS frame, so    */
		/* in RTS the station will broadcast the duration it needs to send  */ 		 		
		/* one data frame and receive ACK for it. Just subtract the			*/
		/* transmission of the CTS frame from updated NAV. Already waited	*/
		/* SIFS is subtracted within "current_time".						*/
		duration = nav_duration - (plcp_overhead_control + WLAN_CTS_DURATION + current_time);
		pk_chstruct_ptr->duration = duration;

		/* Setting Cts frame type.	*/
		op_pk_nfd_set (wlan_transmit_frame_ptr, "Type", type);

		/* Setting the accept field to true, meaning the frame is a good frame.	*/
		op_pk_nfd_set (wlan_transmit_frame_ptr, "Accept", OPC_TRUE);
				
		/* Setting Cts frame fields.	*/
		op_pk_nfd_set (wlan_transmit_frame_ptr, "Wlan Header", pk_chstruct_ptr, wlan_mac_pk_chstruct_copy, 
					   wlan_mac_pk_chstruct_destroy, sizeof (WlanT_Control_Header_Fields));

		/* Once Cts is transmitted in response to Rts then set the frame    		*/
		/* response indicator to none frame as the response is already generated	*/
		fresp_to_send = WlanC_None;										
			
		/* No frame is expected once Cts is transmitted	*/
		expected_frame_type = WlanC_None;	

		/* Printing out information to ODB.	*/
		if (wlan_trace_active == OPC_TRUE)
			{
			op_prg_odb_print_major ("Cts is being transmitted in response to Rts", OPC_NIL);
			}

		/* Reporting total number of bits in a control frame	*/
		op_stat_write (ctrl_traffic_sent_handle_inbits, (double) op_pk_total_size_get (wlan_transmit_frame_ptr));
		op_stat_write (ctrl_traffic_sent_handle_inbits, 0.0);

		/* Update control traffic sent stat when the transmission is complete	*/
		op_stat_write (ctrl_traffic_sent_handle, 1.0);
		op_stat_write (ctrl_traffic_sent_handle, 0.0);
		}
	
	else if	(frame_type == WlanC_Ack)			
		{	
		if ((pcf_flag == OPC_BOOLINT_ENABLED) || (wlan_flags->pcf_active == OPC_BOOLINT_ENABLED))
			last_frametx_type = (WlanT_Mac_Frame_Type) frame_type;
				
		/* Preparing acknowledgement frame in response to the data frame	*/
		/* received from the remote stations.								*/
			
		/* Creating ACK packet format type.									*/
		wlan_transmit_frame_ptr = op_pk_create_fmt ("wlan_control");

		/* Adjust the packet size if necessary to model the PLCP overhead	*/
		/* accurately, which is physical layer technology dependent. The	*/
		/* default value is set for infra-red technology.					*/
		//if (phy_char_flag != WlanC_Infra_Red)
		//	op_pk_bulk_size_set (wlan_transmit_frame_ptr, plcp_overhead_control * WLAN_MAN_DATA_RATE - WLAN_DEFAULT_PLCP_OVERHEAD);

		/* Setting ack frame fields.										*/
		pk_chstruct_ptr = wlan_mac_pk_chstruct_create ();
   		
		type = WlanC_Ack;   		
		pk_chstruct_ptr->retry = duplicate_entry;

		/* If there are more fragments to transmit then broadcast the remaining duration for which	*/
		/* the station will be using the channel.													*/
		if ((wlan_flags->duration_zero == OPC_BOOLINT_DISABLED) || (pcf_flag == OPC_BOOLINT_DISABLED))
			duration = nav_duration - (current_time + plcp_overhead_control + WLAN_ACK_DURATION);
		else 
			duration = 0;
		
		pk_chstruct_ptr->duration = duration;

		/* Destination station address.	*/
		pk_chstruct_ptr->rx_addr = remote_sta_addr;

		/* Setting Ack type.	*/
		op_pk_nfd_set (wlan_transmit_frame_ptr, "Type", type);
			
		/* Setting the accept field to true, meaning the frame is a good frame.	*/
		op_pk_nfd_set (wlan_transmit_frame_ptr, "Accept", OPC_TRUE);

		op_pk_nfd_set (wlan_transmit_frame_ptr, "Wlan Header", pk_chstruct_ptr, wlan_mac_pk_chstruct_copy, 
					   wlan_mac_pk_chstruct_destroy, sizeof (WlanT_Control_Header_Fields));

		/* since no frame is expected,the expected frame type field */
		/* to nil.                                                  */
		expected_frame_type = WlanC_None;	
	
		/* Once Ack is transmitted in response to Data frame then set the frame		*/
		/* response indicator to none frame as the response is already generated	*/
		fresp_to_send = WlanC_None;			

		/* Printing out information to ODB.	*/
		if (wlan_trace_active == OPC_TRUE)
			{
			op_prg_odb_print_major ("Ack is being transmitted for data packet received", OPC_NIL);
			}

		/* Reporting total number of bits in a control frame.   */
		op_stat_write (ctrl_traffic_sent_handle_inbits, (double) op_pk_total_size_get (wlan_transmit_frame_ptr));
		op_stat_write (ctrl_traffic_sent_handle_inbits, 0.0);

		/* Update control traffic sent stat when the transmission is complete*/
		op_stat_write (ctrl_traffic_sent_handle, 1.0);
		op_stat_write (ctrl_traffic_sent_handle, 0.0);
	
		}
	/* Preparing Contension Free end frame if no more stations */
	/* to poll or Cfp_End interrupt.						   */
	else if	((frame_type == WlanC_Cf_End)  ||  (frame_type == WlanC_Cf_End_A))
		{	
	   	if ((pcf_flag == OPC_BOOLINT_ENABLED) || (wlan_flags->pcf_active == OPC_BOOLINT_ENABLED))
			last_frametx_type = (WlanT_Mac_Frame_Type) frame_type;
			
		/* Creating Cf_End packet format type.								*/
		wlan_transmit_frame_ptr = op_pk_create_fmt ("wlan_control");

		/* Adjust the packet size to model the Cf_End message and the PLCP	*/
		/* overhead, which is physical layer technology dependent,			*/
		/* accurately. The default value for PLCP overhead is set for		*/
		/* infra-red technology. Also note that the size of CF End message	*/
		/* is equal to the size of the RTS message.							*/
		if (phy_char_flag != WlanC_Infra_Red)
			op_pk_total_size_set (wlan_transmit_frame_ptr, plcp_overhead_control * WLAN_MAN_DATA_RATE + WLAN_RTS_LENGTH);
		else
			op_pk_total_size_set (wlan_transmit_frame_ptr, WLAN_DEFAULT_PLCP_OVERHEAD + WLAN_RTS_LENGTH);

		/* Setting ack frame fields.										*/
		pk_chstruct_ptr = wlan_mac_pk_chstruct_create ();

		/* Set duration feild */
		/* CF_End duration should always read zero.(Section 7.2.1.6 of spec)*/
		duration = 0;
		pk_chstruct_ptr->duration = duration;

		/* CF End is a broadcast, so set destination address to -1.	*/
		pk_chstruct_ptr->rx_addr = -1;

		/* Spoof BSS ID for now using Source station address.	*/
		pk_chstruct_ptr->tx_addr = ap_mac_address;

		/* Setting frame type.	*/
		op_pk_nfd_set (wlan_transmit_frame_ptr, "Type", frame_type);
			
		/* Setting the accept field to true, meaning the frame is a good frame.	*/
		op_pk_nfd_set (wlan_transmit_frame_ptr, "Accept", OPC_TRUE);

		op_pk_nfd_set (wlan_transmit_frame_ptr, "Wlan Header", pk_chstruct_ptr, wlan_mac_pk_chstruct_copy, 
					   wlan_mac_pk_chstruct_destroy, sizeof (WlanT_Control_Header_Fields));

		/* since no frame is expected,the expected frame type field */
		/* to nil.                                                  */
		expected_frame_type = WlanC_None;	
	
		/* No response is expected so set indicator accordingly		*/
		fresp_to_send = WlanC_None;			

		/* Printing out information to ODB.	*/
		if (wlan_trace_active == OPC_TRUE)
			{
			op_prg_odb_print_major ("CF_END or CF_END+ACK is being transmitted ", OPC_NIL);
			}

		/* Since CFP over, clean up indicators */
		if ((op_ev_valid (cfp_end_evh) == OPC_TRUE) && (wlan_flags->tx_cf_end == OPC_BOOLINT_DISABLED))
			{op_ev_cancel (cfp_end_evh);}

		/* Check if a PCF beacon has been overrun before clearing pcf_active flag */
		if ((wlan_flags->tx_beacon == OPC_BOOLINT_ENABLED) && 
			((((int) (current_time/beacon_int) +cfp_offset) % cfp_prd) == 0))
			{
			/* PCF beacon has been overrun so don't clear flag */
			}
		else 
			{
			wlan_flags->pcf_active = OPC_BOOLINT_DISABLED;
			}

		wlan_flags->tx_cf_end			= OPC_BOOLINT_DISABLED;
		wlan_flags->pcf_side_traf		= OPC_BOOLINT_DISABLED;
		wlan_flags->active_poll			= OPC_BOOLINT_DISABLED;
		wlan_flags->more_data			= OPC_BOOLINT_DISABLED;
		poll_fail_count = 0;

		/* Reporting total number of bits in a control frame.   */
		op_stat_write (ctrl_traffic_sent_handle_inbits, (double) op_pk_total_size_get (wlan_transmit_frame_ptr));
		op_stat_write (ctrl_traffic_sent_handle_inbits, 0.0);

		/* Update control traffic sent stat when the transmission is complete*/
		op_stat_write (ctrl_traffic_sent_handle, 1.0);
		op_stat_write (ctrl_traffic_sent_handle, 0.0);
		}
	else
		{
		wlan_mac_error ("Transmission request for unexpected frame type.", OPC_NIL, OPC_NIL);
		}

	//op_pk_print(wlan_transmit_frame_ptr);
	
	
			
	/* Sending packet to the transmitter */
	op_pk_send (wlan_transmit_frame_ptr, outstrm_to_phy);
	
	wlan_flags->transmitter_busy = OPC_BOOLINT_ENABLED;
	
	/* Clear ignor busy flag in case it was set */
	wlan_flags->ignore_busy = OPC_BOOLINT_DISABLED;
	
	/* Clear PCF side traffic flag in case it was set */
	wlan_flags->pcf_side_traf = OPC_BOOLINT_DISABLED;
	
	/* Clear polled flag in case it was set */
	wlan_flags->polled = OPC_BOOLINT_DISABLED;
	
	FOUT;
	}

void 
wlan_interrupts_process ()
	{
	/** This routine handles the appropriate processing need for each type 	**/
	/** of remote interrupt. The type of interrupts are: stream interrupts	**/
	/** (from lower and higher layers), stat interrupts (from receiver and 	**/
	/** transmitter).                                                      	**/
	FIN (wlan_interrupts_process ());

	/* Check if debugging is enabled.										*/
	wlan_trace_active = op_prg_odb_ltrace_active ("wlan");

	/* Determine the current simualtion time.								*/	
	current_time = op_sim_time ();
	
	/* Determine interrupt type and code to divide treatment along the		*/
	/* lines of interrupt type.						  						*/
	intrpt_type = op_intrpt_type ();
	intrpt_code = (WlanT_Mac_Intrpt_Code) op_intrpt_code ();

	/* Stream interrupts are either arrivals from the higher layer, or		*/
	/* from the physical layer.												*/
	if (intrpt_type == OPC_INTRPT_STRM)
		{
		/* Determine the stream on which the arrival occurred.				*/
		i_strm = op_intrpt_strm ();

		/* If the event arrived from higher layer then queue the packet	and	*/
		/* the destination address.											*/
		if (i_strm == instrm_from_mac_if)
			{
			/* Process stream interrupt received from higher layer.			*/
			wlan_higher_layer_data_arrival ();
			}

		/* If the event was an arrival from the physical layer,	*/
		/* accept the packet and decapsulate it			 		*/
		else 
			{
			/* Process stream interrupt received from physical layer	*/			 
			wlan_physical_layer_data_arrival ();		
   			}
	 	}	

	/* Handle stat interrupt received from the receiver	*/
	else if (intrpt_type == OPC_INTRPT_STAT)
		{
		/* Make sure it is not a stat interrupt from the transmitter.	*/
		if (intrpt_code < TRANSMITTER_BUSY_INSTAT)
			{
			/* One of receiver channels is changing its status.		*/
			/* Update the channel status vector.					*/
			wlan_mac_rcv_channel_status_update (intrpt_code);
						
			/* Update the flag value based on the new status of the	*/
			/* receiver channels.									*/
			if (rcv_channel_status == 0)
				{				
				wlan_flags->receiver_busy = OPC_BOOLINT_DISABLED;

				/* Reset the receiver idle timer to the current time since	*/
				/* it became available.										*/
				rcv_idle_time = current_time;
				}
			else
				{
				wlan_flags->receiver_busy = OPC_BOOLINT_ENABLED;
				}
			}
		}

	/* Handle interrupt from Beacon timer */
	else if (intrpt_type == OPC_INTRPT_SELF && intrpt_code == WlanC_Beacon_Tx_Time)
		{
		/* If AP and time to transmit beacon then set the flag	  */
		if (ap_flag == OPC_BOOLINT_ENABLED) 
			{
			wlan_flags->tx_beacon = OPC_BOOLINT_ENABLED;

			/* Set timer for next beacon Tx.				*/
			beacon_evh = op_intrpt_schedule_self (current_time + beacon_int , WlanC_Beacon_Tx_Time);	
			}
		
		/* Make the initializations for CFP if the BSS has	*/
		/* at least one PCF enabled station and if if we	*/
		/* are not already in CFP.							*/
		if (active_pc == OPC_BOOLINT_ENABLED && wlan_flags->pcf_active == OPC_BOOLINT_DISABLED)
			{
			if ((((int) (current_time / beacon_int) + cfp_offset) % cfp_prd) == 0)
				{
				/* If this is an AP.						*/
				if (ap_flag == OPC_BOOLINT_ENABLED)
					{
					cfp_end_evh = op_intrpt_schedule_self (current_time + cfp_length , WlanC_Cfp_End);

					/* For current polling implmentation, the fragmentation buffer is assumed	*/
					/* empty at the start of the CFP, and polling always starts with the first 	*/	
					/* station. This variable will be incremented for first station.  			*/
					poll_index = -1;

					/* Reset failed poll counter.			*/
					poll_fail_count = 0;

					/* Reset poll for more data flag.		*/
					wlan_flags->more_data = OPC_BOOLINT_DISABLED;
					
					/* Reset pcf queue offset.				*/
					pcf_queue_offset = 0;
					}
				else 
					{
					/* Update the NAV duration for a non-AP station */
					if (nav_duration < (cfp_length + current_time))
						{
						nav_duration = cfp_length + current_time;
						}
					}
				}
			} 
		}
	/* Handle interrupt from CFP end timer */
	else if (intrpt_type == OPC_INTRPT_SELF && intrpt_code == WlanC_Cfp_End)
		{
		/* Set Transmit Contension Free (CF) Period End flag */
		wlan_flags->tx_cf_end = OPC_BOOLINT_ENABLED;
		
		/* Don't clear CFP Flag till after transmitting CF End	  */
		/* Don't touch NAV - It should clear itself anyway */
		}
	
	FOUT;
	}

static void 
wlan_physical_layer_data_arrival ()
	{
	char										msg_string [120];
	int											dest_addr;
	int											accept;
	int											data_pkt_id;
	int											final_dest_addr;
	WlanT_Data_Header_Fields*					pk_dhstruct_ptr;
	WlanT_Control_Header_Fields*				pk_chstruct_ptr;
	WlanT_Mac_Frame_Type						rcvd_frame_type;	
	Packet*										wlan_rcvd_frame_ptr;
	Packet*										seg_pkptr;
	WlanT_Beacon_Body_Fields*					pk_bbstruct_ptr;
	int											rcvd_sta_bssid;
	int											temp;
	int											temp2;
	char										actual_frame_name [256];
	
	/** Process the frame received from the lower layer.          **/
	/** This routine decapsulate the frame and set appropriate    **/
	/** flags if the station 4needs to generate a response to the **/
	/** received frame.											  **/
	FIN (wlan_physical_layer_data_arrival ());

	/*  Access received packet from the physical layer stream.	*/
	wlan_rcvd_frame_ptr = op_pk_get (i_strm);	

	op_pk_nfd_access (wlan_rcvd_frame_ptr, "Accept", &accept);
				
	/* If the packet is received while the station is in transmission,	*/
	/* or if the packet is collided with another packet received via	*/
	/* another channel (which has the same frequency but a different	*/
	/* data rate) or if the accept field is set to false, then the		*/
	/* packet will not be processed and if needed the station will		*/
	/* retransmit the packet.											*/
	if ((wlan_flags->rcvd_bad_packet == OPC_BOOLINT_ENABLED) || (accept == OPC_FALSE) ||
		(wlan_flags->collided_packet == OPC_BOOLINT_ENABLED))
		{
		/* If the pipeline stage set the accept flag to be false or if it is 		*/
		/* collided then it means that the packet is erroneous.  Enable the EIFS	*/
		/* duration flag and set nav duration to be EIFS duration.					*/ 	
		if (((accept == OPC_FALSE) || 
			(wlan_flags->collided_packet == OPC_BOOLINT_ENABLED)) && 
				wlan_flags->pcf_active == OPC_BOOLINT_DISABLED)
			{
			wlan_flags->wait_eifs_dur = OPC_BOOLINT_ENABLED;
     
			/* Setting nav duration to EIFS.	*/
			if (cfp_ap_medium_control == OPC_BOOLINT_DISABLED)
				{
				nav_duration = current_time + eifs_time - difs_time; 
				}

			/* Reporting the amount of time the channel will be busy.	*/	   
			op_stat_write (channel_reserv_handle, (nav_duration - current_time));
			op_stat_write (channel_reserv_handle, 0.0);
			}
		
		/* Check if we were waiting for a poll response and increment poll fail accordingly */
		if	(((last_frametx_type == WlanC_Data_Poll)	||	(last_frametx_type == WlanC_Data_A_P)	||
			(last_frametx_type == WlanC_Cf_Poll)		||	(last_frametx_type == WlanC_Cf_A_P))	&&
			(wlan_flags->active_poll == OPC_BOOLINT_ENABLED))
			{
			/* Poll failed.  Increment Failed Poll counter */
			poll_fail_count ++;
			wlan_flags->active_poll = OPC_BOOLINT_DISABLED;
			}
		
		/* We may have experienced a collision during transmission. We	*/
		/* could be transmitting a packet which requires a response (an	*/
		/* Rts or a data frame requiring an Ack). Even, this is the		*/
		/* case, we do not take any action right now and wait for the	*/
		/* related timers to expire; then we will retransmit the frame.	*/
		/* This is the approach described in the standard, and it is	*/
		/* necessary because of the slight possibility that our peer	*/
		/* may receive the frame without collision and send us the		*/
		/* response back, which we should be still expecting.			*/

		/* Check whether the timer for the expected response has		*/
		/* already expired. If yes, we must initiate the retransmission.*/
		if ((expected_frame_type != WlanC_None) && 
			(wlan_flags->transmitter_busy == OPC_BOOLINT_DISABLED) &&
			(op_ev_valid (frame_timeout_evh) == OPC_FALSE))
			{
			if ((last_frametx_type == WlanC_Data) || (last_frametx_type == WlanC_Data_Ack))
				{
				retry_count = retry_count + 1;

				/* Check if more retrys permited, if not discard */
				wlan_frame_discard ();
				}
			else
				{
				/* If last frame a data frame, Increment retry counter */
				if ((last_frametx_type == WlanC_Data_Poll)	||	(last_frametx_type == WlanC_Data_A_P))
					{
					pcf_retry_count = pcf_retry_count ++;
					}

				/* Check if more retrys permited, if not discard */
				wlan_pcf_frame_discard ();
				}

			/* If Rts sent flag was enable then disable it as the station will recontend for the channel.	*/
			if (wlan_flags->rts_sent == OPC_BOOLINT_ENABLED)
				{
				wlan_flags->rts_sent = OPC_BOOLINT_DISABLED;
				}

			/* Set expected frame type flag to none as the		*/
			/* station needs to retransmit the frame.			*/
			expected_frame_type = WlanC_None;
			
			/* Reset the NAV duration so that the				*/
			/* retransmission is not unnecessarily delayed.		*/
			if (cfp_ap_medium_control == OPC_BOOLINT_DISABLED)
				nav_duration = current_time;
			}

		/* Reset the bad packet receive flag for subsequent		*/
		/* receptions.											*/
		wlan_flags->rcvd_bad_packet = OPC_BOOLINT_DISABLED;		

		/* Similarly reset the collided packet flag.			*/
		wlan_flags->collided_packet = OPC_BOOLINT_DISABLED;
			
		/* Printing out information to ODB.	*/
		if (wlan_trace_active == OPC_TRUE)
			{
			op_prg_odb_print_major ("Received bad packet. Discarding received packet", OPC_NIL);
			}
		
		/* Destroy the bad packet.									*/
		op_pk_destroy (wlan_rcvd_frame_ptr);

		/* Break the routine as no further processing is needed.	*/
		FOUT;
		}

	/* If waiting for EIFS duration then set the nav duration such that	*/
	/* the normal operation is resumed.									*/
	if (wlan_flags->wait_eifs_dur == OPC_BOOLINT_ENABLED)
		{
		if (cfp_ap_medium_control == OPC_BOOLINT_DISABLED)
			nav_duration = current_time;
		
		wlan_flags->wait_eifs_dur = OPC_BOOLINT_DISABLED;	
		}	

	/* Getting frame control field and duration information from	*/
    /* the received packet.											*/		
	op_pk_nfd_access (wlan_rcvd_frame_ptr, "Type", &rcvd_frame_type) ;
	
	if	((rcvd_frame_type == WlanC_Data)		|| (rcvd_frame_type == WlanC_Data_Ack)	|| 
		(rcvd_frame_type == WlanC_Data_Poll)	|| (rcvd_frame_type == WlanC_Data_A_P)	|| 
		(rcvd_frame_type == WlanC_Data_Null)	|| (rcvd_frame_type == WlanC_Cf_Ack)	|| 
		(rcvd_frame_type == WlanC_Cf_Poll)		|| (rcvd_frame_type == WlanC_Cf_A_P))
		{
		/* First check that wether the station is expecting	 	*/
		/* any frame or not. If not then decapsulate relevant 	*/
		/* information from the packet fields and set the frame	*/
		/* response variable with appropriate frame type.		*/
		if	((rcvd_frame_type == WlanC_Data)		|| (rcvd_frame_type == WlanC_Data_Ack)	|| 
			(rcvd_frame_type == WlanC_Data_Poll)	|| (rcvd_frame_type == WlanC_Data_A_P))
			{
			/*  Data traffic received report in terms of number of bits.	*/
			op_stat_write (data_traffic_rcvd_handle_inbits, (double) op_pk_total_size_get (wlan_rcvd_frame_ptr));
			op_stat_write (data_traffic_rcvd_handle_inbits, 0.0);

			/*  Data traffic received report in terms of number of packets.	*/
			op_stat_write (data_traffic_rcvd_handle, 1.0);
			op_stat_write (data_traffic_rcvd_handle, 0.0);
			}
		else
			{
			/* Received frame is a control frame using data format, update control		*/
			/* statistics. Data traffic received report in terms of number of bits.		*/
			op_stat_write (ctrl_traffic_rcvd_handle_inbits, (double) op_pk_total_size_get (wlan_rcvd_frame_ptr));
			op_stat_write (ctrl_traffic_rcvd_handle_inbits, 0.0);

			/*  Data traffic received report in terms of number of packets.				*/
			op_stat_write (ctrl_traffic_rcvd_handle, 1.0);
			op_stat_write (ctrl_traffic_rcvd_handle, 0.0);
			}
		
		/* In case of Beacon colliding with other packets being received, the stations	*/
		/* will not be indicated on the PCF period. If this happens,and we recieve a	*/
		/* poll packet, then we need to initalize variables that indicate the start of	*/
		/* PCF duration.							 	  								*/
		if ((cfp_ap_medium_control == OPC_BOOLINT_DISABLED) && 
			(rcvd_frame_type == WlanC_Data_Poll) || (rcvd_frame_type == WlanC_Data_A_P) ||
			(rcvd_frame_type == WlanC_Cf_Poll) || (rcvd_frame_type == WlanC_Cf_A_P))
			{
			/* Indicate the medium controlled by the active AP.							*/
			cfp_ap_medium_control =  OPC_BOOLINT_ENABLED;
			}
		
		/* Address information, sequence control fields,	*/
		/* and the data is extracted from the rcvd packet.	*/
		op_pk_nfd_access (wlan_rcvd_frame_ptr, "Wlan Header",	&pk_dhstruct_ptr);

		/* Data packet id of the received data frame is extracted.	*/
		op_pk_nfd_access (wlan_rcvd_frame_ptr, "Data Packet ID", &data_pkt_id);

		/* Obtain the destination this packet id addressed to */
		dest_addr = pk_dhstruct_ptr->address1;	
		remote_sta_addr = pk_dhstruct_ptr->address2;
		
		/* Check if PCF active and this is an AP */
		if ((wlan_flags->pcf_active == OPC_BOOLINT_ENABLED) && 
			(ap_flag == OPC_BOOLINT_ENABLED))
			{
			/* Check if frame is from station polled and increment failed poll count if necessary	*/
			if (remote_sta_addr == polling_list [poll_index]) 
				{
				poll_fail_count = 0;
				}
			else if (wlan_flags->active_poll == OPC_BOOLINT_ENABLED) 
				{
				poll_fail_count++;
				}
			
	
			/* Disable active poll flag since it is now satisfied one way or another */
			wlan_flags->active_poll = OPC_BOOLINT_DISABLED;
			
			if ((dest_addr != ap_mac_address)  && (dest_addr > -1))
				{
				/* If data not addressed to AP and not broadcast then set side	*/
				/* traffic flag and set NAV to wait for expected ack 			*/
				wlan_flags->pcf_side_traf = OPC_BOOLINT_ENABLED;
				
				if (nav_duration < (current_time + sifs_time + plcp_overhead_control + WLAN_ACK_DURATION)) 
					nav_duration = current_time + sifs_time + plcp_overhead_control + WLAN_ACK_DURATION;

				/* Set poll more fragments flag if necessary */
				if (pk_dhstruct_ptr->more_frag == 1) 
					wlan_flags->more_frag = OPC_BOOLINT_ENABLED;
				else 
					wlan_flags->more_frag = OPC_BOOLINT_DISABLED;

				/* Set poll more data (MSDU's) flag if necessary */
				if ((pk_dhstruct_ptr->more_data == 0)  || (wlan_flags->tx_beacon == OPC_BOOLINT_ENABLED) ||
					(wlan_flags->tx_cf_end == OPC_BOOLINT_ENABLED))
					{
					/* No more polls for new MDSU will be sent to this STA so reset flag		*/
					wlan_flags->more_data = OPC_BOOLINT_DISABLED;
					}
				else 
					{
					/* need to poll this STA at least one more time, so set flag				*/
					wlan_flags->more_data = OPC_BOOLINT_ENABLED;
					}
				}
			}
		
		/* If the station is an AP then it will need to forward the receiving data	*/ 
		/* to this address.Otherwise this field will be zero and will be ignored.  	*/
		final_dest_addr = pk_dhstruct_ptr->address3;

		/* Process frame only if it is destined for this station.	*/
		/* or it is a broadcast frame.								*/
		if	(((dest_addr == my_address) || (dest_addr < 0)) &&
			((rcvd_frame_type == WlanC_Data)|| (rcvd_frame_type == WlanC_Data_Ack)	|| 
			(rcvd_frame_type == WlanC_Data_Poll)|| (rcvd_frame_type == WlanC_Data_A_P)))
			{	
			/* Extracting the MSDU from the packet only if the packet	*/
			/* is destined for this station.							*/		
			op_pk_nfd_get (wlan_rcvd_frame_ptr, "Frame Body", &seg_pkptr);

			/* Only send acknowledgement if the data frame is destined for 	*/
			/* this station.No Acks for broadcast frame.					*/
			if (dest_addr == my_address)
				{
				/* Send the acknowledgement to any received data frame.	*/
				fresp_to_send = WlanC_Ack;
				
				/* If no more fragments, then duration value in ack must be zero */
				if (pk_dhstruct_ptr->more_frag == 0)
					wlan_flags->duration_zero = OPC_BOOLINT_ENABLED;
				else 
					wlan_flags->duration_zero = OPC_BOOLINT_DISABLED;
							
				}
			             	
			/* If PCF is active and this is an AP */ 
			if ((wlan_flags->pcf_active == OPC_BOOLINT_ENABLED) && 
				(ap_flag == OPC_BOOLINT_ENABLED))
				{
				/* Set poll more fragments flag if necessary */
				if (pk_dhstruct_ptr->more_frag == 1)
					wlan_flags->more_frag = OPC_BOOLINT_ENABLED;
				else 
					wlan_flags->more_frag = OPC_BOOLINT_DISABLED;

				/* Set poll more data (MSDU's) flag if necessary */
				if ((pk_dhstruct_ptr->more_data == 0)  || (wlan_flags->tx_beacon == OPC_BOOLINT_ENABLED) ||
					(wlan_flags->tx_cf_end == OPC_BOOLINT_ENABLED))
					{
					/* no more polls for new MDSU will be sent to this STA so reset flag		*/
					wlan_flags->more_data = OPC_BOOLINT_DISABLED;
					}
				else 
					{
					/* need to poll this STA at least one more time, so set flag				*/
					wlan_flags->more_data = OPC_BOOLINT_ENABLED;
					}
				}
			
									
			/* If its a duplicate packet then destroy it and do nothing otherwise, 	*/
			/* insert it in the defragmentation list.								*/
			if (wlan_tuple_find (remote_sta_addr, pk_dhstruct_ptr->sequence_number, pk_dhstruct_ptr->fragment_number) == OPC_FALSE)
				{
				wlan_data_process (seg_pkptr,dest_addr, remote_sta_addr, final_dest_addr, pk_dhstruct_ptr->fragment_number, 
					pk_dhstruct_ptr->more_frag, data_pkt_id);
				}
			}
				
		else 
			{
			/* Printing out information to ODB.	*/
			if (wlan_trace_active == OPC_TRUE)
				{
				sprintf (msg_string, "Data packet %d is received and discarded", data_pkt_id);
				op_prg_odb_print_major (msg_string, OPC_NIL);
			 	}
			}
			
		if (((rcvd_frame_type == WlanC_Data_Poll)|| (rcvd_frame_type == WlanC_Data_A_P) ||
			(rcvd_frame_type == WlanC_Cf_Poll)	|| (rcvd_frame_type == WlanC_Cf_A_P))	&&
			(dest_addr == my_address))
			{
			/* If frame contained a poll for my address, set polled flag */
			wlan_flags->polled = OPC_BOOLINT_ENABLED;
			
			/* Printing out information to ODB.	*/
			if (wlan_trace_active == OPC_TRUE)
				{
				wlan_frame_type_conv(rcvd_frame_type, actual_frame_name) ;
				
				sprintf (msg_string, "%s is received from AP", actual_frame_name);
				op_prg_odb_print_major (msg_string, OPC_NIL);
			 	}
			}
		
		if (expected_frame_type == WlanC_Ack)
			{
			/* If an Ack is expected, and a PCF Ack is recieved, regardless of whom	*/  
			/* the frame is	addressed to, process the received ACK.					*/
			if ((rcvd_frame_type == WlanC_Data_Ack)|| (rcvd_frame_type == WlanC_Data_A_P)	||
				(rcvd_frame_type == WlanC_Cf_Ack)	|| (rcvd_frame_type == WlanC_Cf_A_P)) 
				{
				/* Printing out information to ODB.	*/
				if (wlan_trace_active == OPC_TRUE)
					{
					if ((last_frametx_type == WlanC_Data) || (last_frametx_type == WlanC_Data_Ack))
						{
						sprintf (msg_string, "Ack received for data packet %d", pkt_in_service);
						op_prg_odb_print_major (msg_string, OPC_NIL);
						}
					else
						{
						sprintf (msg_string, "Ack received for data packet %d", pcf_pkt_in_service);
						op_prg_odb_print_major (msg_string, OPC_NIL);
						}
					}			

				op_stat_write (retrans_handle, (double) (pcf_retry_count * 1.0));

				if ((last_frametx_type == WlanC_Data) || (last_frametx_type == WlanC_Data_Ack))
					{
					/* Reset the retry counter as the expected frame is received.		*/
					retry_count = 0;
					
					/* Similarly reset the variables that may have been set and used	*/
					/* for this transmission during the DCF period that proceded the	*/
					/* current PCF period.												*/
					wlan_flags->rts_sent = OPC_BOOLINT_DISABLED;
					if (wlan_flags->cw_required != OPC_BOOLINT_ENABLED)
						backoff_slots = BACKOFF_SLOTS_UNSET;
					
					/* Decrement number of fragment count because one fragment is		*/
					/* successfully transmitted.										*/
					num_fragments = num_fragments - 1;				

					/* Data packet is successfully delivered to remote station,			*/
					/* since no further retransmission is needed the copy of the data	*/
					/* packet will be destroyed.										*/
					if (wlan_transmit_frame_copy_ptr != OPC_NIL) 
						{
						op_pk_destroy (wlan_transmit_frame_copy_ptr);
						wlan_transmit_frame_copy_ptr = OPC_NIL;
						}
					}
				else
					{
					/* Reset the retry counter as the expected frame is received  */
					pcf_retry_count = 0;

					/* Decrement number of fragment count because one fragment    */
					/* is successfully transmitted.							      */
					pcf_num_fragments = pcf_num_fragments - 1;				

					/* Data packet is successfully delivered to remote station,			*/
					/* since no further retransmission is needed the copy of the data	*/
					/* packet will be destroyed.										*/
					if (wlan_pcf_transmit_frame_copy_ptr != OPC_NIL)
						{
						op_pk_destroy (wlan_pcf_transmit_frame_copy_ptr);
						wlan_pcf_transmit_frame_copy_ptr = OPC_NIL;
						}
					}
				}
			else
				{
				/* A frame has not been properly acknowledged and requires retransmission    */
				/* If the last transmitted frame was a data frame must be a DCF transmission */
				/* Else must be a PCF transmission 											 */
				if ((last_frametx_type == WlanC_Data) || (last_frametx_type == WlanC_Data_Ack)) 
					{
				    retry_count++;
					}
				else 
					{
					pcf_retry_count = pcf_retry_count++;
					}
				}
			}

		if 	((expected_frame_type == WlanC_Data) && 
			((rcvd_frame_type == WlanC_Data) ||	(rcvd_frame_type == WlanC_Data_Null)))
			{
			/* If PCF Data is expected, and some type of PCF Data is received regardless of who	*/  
			/* 	addressed to, poll is satisfied so destroy copy of tx packet                    */
			if (((last_frametx_type == WlanC_Data)  || 
				(last_frametx_type == WlanC_Data_Ack)) && 
				(wlan_transmit_frame_copy_ptr != OPC_NIL)) 
				{
				op_pk_destroy (wlan_transmit_frame_copy_ptr);
				wlan_transmit_frame_copy_ptr = OPC_NIL;
				}
			else if (wlan_pcf_transmit_frame_copy_ptr != OPC_NIL)
				{
				op_pk_destroy (wlan_pcf_transmit_frame_copy_ptr);
				wlan_pcf_transmit_frame_copy_ptr = OPC_NIL;
				}
			}
		
		if (expected_frame_type == WlanC_Cts) 
			{
			/* Since the station did not receive the expected frame	*/
			/* it has to retransmit the packet.						*/
			retry_count = retry_count + 1;
						
			/* If Rts sent flag was enable then disable it as the station will recontend for the channel.	*/
			if (wlan_flags->rts_sent == OPC_BOOLINT_ENABLED)
				{
				wlan_flags->rts_sent = OPC_BOOLINT_DISABLED;
				}
			
			if (cfp_ap_medium_control == OPC_BOOLINT_DISABLED)
				nav_duration = current_time;
			}
			
		/* Update NAV duration if the received NAV duration is greater	*/
		/* than the current NAV duration.							  	*/    	
		if ((pk_dhstruct_ptr->duration < 32768) && 
			(nav_duration < (pk_dhstruct_ptr->duration + current_time)))
			{
			nav_duration = pk_dhstruct_ptr->duration + current_time;

			/* Set the flag that indicates updated NAV value.			*/
			wlan_flags->nav_updated = OPC_BOOLINT_ENABLED;
			}
		else if (nav_duration < current_time)
			{
			/* Update NAV to current time in case ack is required (otherwise negative duration may be	*/
			/* be computed.  Should not effect deference processing, so don't set nav_updated flag.		*/
			nav_duration = current_time;
			}			
		}

	else if	(rcvd_frame_type == WlanC_Beac)
		{
		/*  Control Traffic received report in terms of number of bits.		*/
		op_stat_write (ctrl_traffic_rcvd_handle_inbits, (double) op_pk_total_size_get (wlan_rcvd_frame_ptr));
		op_stat_write (ctrl_traffic_rcvd_handle_inbits, 0.0);

		/*  Control Traffic received report in terms of number of packets.	*/
		op_stat_write (ctrl_traffic_rcvd_handle, 1.0);
		op_stat_write (ctrl_traffic_rcvd_handle, 0.0);

		/* Address information, sequence control fields,	*/
		/* and the data is extracted from the rcvd packet.	*/
		op_pk_nfd_access (wlan_rcvd_frame_ptr, "Wlan Header",	&pk_dhstruct_ptr);

		/* Data packet id of the received data frame is extracted. (Set -1 for beacon)	*/
		op_pk_nfd_access (wlan_rcvd_frame_ptr, "Data Packet ID", &data_pkt_id);

		/* The destination address in this case will be -1 as the */
		/* beacon frame is a broadcast frame.				      */
		dest_addr = pk_dhstruct_ptr->address1;			
		
		/* Store the address of the AP transmitting the beacon    */
		remote_sta_addr = pk_dhstruct_ptr->address2;	
		rcvd_sta_bssid = pk_dhstruct_ptr->address3;		

		/* Init frame response to send 			*/
		fresp_to_send = WlanC_None; 
		
		/* Extracting the Beacon Body packet 	*/
		op_pk_nfd_get (wlan_rcvd_frame_ptr, "Frame Body", &seg_pkptr);

		/* Extracting the Beacon Body structure from packet	*/
		op_pk_nfd_access (seg_pkptr, "Beacon Body",	&pk_bbstruct_ptr);

		/* Only send acknowledgement if the beacon frame is addressed to this station.	*/
		/* No Acks for broadcast frame.													*/
		if (dest_addr == my_address)
			{
			/* Send the acknowledgement to any received data frame.	*/
			fresp_to_send = WlanC_Ack;
			}
		else
			{
			/* If the frame is not destined for this station	*/
			/* then do not respond with any frame.				*/
			fresp_to_send = WlanC_None;
			}						
			
		if (expected_frame_type != WlanC_None) 
			{
			/* Since the station did not receive the expected frame	*/
			/* it has to retransmit the packet.						*/
			if (wlan_flags->pcf_active == OPC_BOOLINT_ENABLED) 
				{
				pcf_retry_count = pcf_retry_count + 1;
				}
			else
				{
				retry_count = retry_count + 1;
				}
			
			/* If Rts sent flag was enable then disable it as the station will recontend for the channel.	*/
			if (wlan_flags->rts_sent == OPC_BOOLINT_ENABLED)
				{
				wlan_flags->rts_sent = OPC_BOOLINT_DISABLED;
				}
			}
			
		/* If beacon is intended for this BSS, update BSS related variables */
		if (rcvd_sta_bssid == bss_id) 
			{
			/* Set beacon interal */
			beacon_int = pk_bbstruct_ptr->beacon_intv;				
	
			/* Set CFP period */		
			cfp_prd = pk_bbstruct_ptr->cf_par.cfp_period;		
			
			/* Set CFP length*/
			cfp_length =pk_bbstruct_ptr->cf_par.cfp_maxduration;	

			/* Indicate the presence of an active AP  */
			if (cfp_prd != 0)
				{
				/* set active PC flag */	
				active_pc = OPC_BOOLINT_ENABLED;
				
				temp = (((int) (pk_bbstruct_ptr->timestamp/beacon_int)) % cfp_prd);
				
				temp2 = (temp - pk_bbstruct_ptr->cf_par.cfp_count + cfp_prd);
				cfp_offset = temp2 % cfp_prd;
				}
			}

		if ((pk_bbstruct_ptr->cf_par.cfp_count == 0) && 
			((pk_bbstruct_ptr->timestamp + pk_bbstruct_ptr->cf_par.cfp_durremaining) > nav_duration))
			{
			nav_duration = pk_bbstruct_ptr->timestamp + pk_bbstruct_ptr->cf_par.cfp_durremaining;

			/* Set the flag that indicates updated NAV value.			*/
			wlan_flags->nav_updated = OPC_BOOLINT_ENABLED;
			
			}

		/* Update nav duration if the received nav duration is greater	*/
		/* than the current nav duration.							  	*/    	
		else if ((pk_dhstruct_ptr->duration < 32768) && 
			(nav_duration < (pk_dhstruct_ptr->duration + current_time)))
			{
			nav_duration = pk_dhstruct_ptr->duration + current_time;

			/* Set the flag that indicates updated NAV value.			*/
			wlan_flags->nav_updated = OPC_BOOLINT_ENABLED;
			
			}

		/* Destroy beacon body since nolonger needed */
		op_pk_destroy (seg_pkptr);
		
		/* Indicate the medium controlled by the active AP */
		cfp_ap_medium_control = OPC_BOOLINT_ENABLED;
		
		/* Printing out information to ODB.	*/
		if (wlan_trace_active == OPC_TRUE)
			{
			op_prg_odb_print_major ("Beacon frame is received", OPC_NIL);
		 	}
		}

	else if	(rcvd_frame_type == WlanC_Rts)
		{
		/** First check that wether the station is expecting any frame or not	**/
		/** If not then decapsulate the Rts frame and set a Cts frame response	**/
		/** if frame is destined for this station. Otherwise, just update the	**/
		/** network allocation vector for this station.							**/

		/*  Control Traffic received report in terms of number of bits.		*/
		op_stat_write (ctrl_traffic_rcvd_handle_inbits, (double) op_pk_total_size_get (wlan_rcvd_frame_ptr));
		op_stat_write (ctrl_traffic_rcvd_handle_inbits, 0.0);
		
		/*  Control Traffic received report in terms of number of packets.	*/
		op_stat_write (ctrl_traffic_rcvd_handle, 1.0);
		op_stat_write (ctrl_traffic_rcvd_handle, 0.0);

		op_pk_nfd_access (wlan_rcvd_frame_ptr, "Wlan Header",	&pk_chstruct_ptr);
			
		dest_addr = pk_chstruct_ptr->rx_addr;
		remote_sta_addr = pk_chstruct_ptr->tx_addr;
			
		if (expected_frame_type == WlanC_None)
			{
			/* We will respond to the Rts with a Cts only if a) the	*/
			/* Rts is destined for us, and b) our NAV duration is	*/
			/* not larger than current simulation time.				*/
			if ((my_address == dest_addr) && (current_time >= nav_duration))
				{
				/* Set the frame response field to Cts.				*/
				fresp_to_send = WlanC_Cts;

				/* Printing out information to ODB.					*/
				if (wlan_trace_active == OPC_TRUE)
					op_prg_odb_print_major ("Rts is received and Cts will be transmitted", OPC_NIL);
				}			
			else
				{
				/* Printing out information to ODB.					*/
				if (wlan_trace_active == OPC_TRUE)
					op_prg_odb_print_major ("Rts is received and discarded", OPC_NIL);
				}				
		 	}
		else
			{				
			/* Since the station did not receive the expected frame it has to retransmit the packet.	*/
			retry_count = retry_count + 1;				
			
			/* If Rts sent flag was enable then disable it as the station will recontend for the channel.	*/
			if (wlan_flags->rts_sent == OPC_BOOLINT_ENABLED)
				{
				wlan_flags->rts_sent = OPC_BOOLINT_DISABLED;
				}
				
			/* Reset the NAV duration so that the				*/
			/* retransmission is not unnecessarily delayed.		*/
			if (cfp_ap_medium_control == OPC_BOOLINT_DISABLED)
				nav_duration = current_time;
						
			/* Reset the expected frame type variable since we	*/
			/* will retransmit.									*/
			fresp_to_send = WlanC_None;
			}

		/* Update NAV duration if the received NAV duration is greater	*/
		/* than the current NAV duration.							  	*/    	
		if (nav_duration < (pk_chstruct_ptr->duration + current_time))
			{
			nav_duration = pk_chstruct_ptr->duration + current_time;

			/* Set the flag that indicates updated NAV value.			*/
			wlan_flags->nav_updated = OPC_BOOLINT_ENABLED;
			}			
		}		
	
	else if	(rcvd_frame_type == WlanC_Cts)
		{
		/** First check that whether the station is expecting any frame or not	**/
		/** If not then decapsulate the Rts frame and set a Cts frame response	**/
		/** if frame is destined for this station. Otherwise, just update the	**/
		/** network allocation vector for this station.							**/

		/*  Control Traffic received report in terms of number of bits.	*/
		op_stat_write (ctrl_traffic_rcvd_handle_inbits, (double) op_pk_total_size_get (wlan_rcvd_frame_ptr));
		op_stat_write (ctrl_traffic_rcvd_handle_inbits, 0.0);

		/*  Control Traffic received report in terms of number of packets.	*/
		op_stat_write (ctrl_traffic_rcvd_handle, 1.0);
		op_stat_write (ctrl_traffic_rcvd_handle, 0.0);	

		op_pk_nfd_access (wlan_rcvd_frame_ptr, "Wlan Header",	&pk_chstruct_ptr);
		dest_addr = pk_chstruct_ptr->rx_addr;

		/* If the frame is destined for this station and the station is expecting	*/
		/* Cts frame then set appropriate indicators.								*/
		if ((dest_addr == my_address) && (expected_frame_type == rcvd_frame_type)) 				
			{
			/* The receipt of Cts frame indicates that Rts is successfully	*/
			/* transmitted and the station can now respond with Data frame	*/
			fresp_to_send = WlanC_Data;

			/* Set the flag indicating that Rts is succesfully transmitted	*/
			wlan_flags->rts_sent = OPC_BOOLINT_ENABLED;

			op_stat_write (retrans_handle, (double) (retry_count * 1.0));

			/* Printing out information to ODB.	*/
			if (wlan_trace_active == OPC_TRUE)
				{	
				sprintf (msg_string, "Cts is received for Data packet %d", pkt_in_service);
				op_prg_odb_print_major (msg_string, OPC_NIL);
				}
			}
		else
			{
			/* Printing out information to ODB.								*/
			if (wlan_trace_active == OPC_TRUE)
				op_prg_odb_print_major ("Cts is received and discarded.", OPC_NIL);

			/* Check whether we were expecting another frame. If yes then	*/
			/* we need to retransmit the frame for which we were expecting	*/
			/* a reply.														*/
			if (expected_frame_type != WlanC_None)
				{				
				/* Since the station did not receive the expected frame it	*/
				/* has to retransmit the packet.							*/
				retry_count = retry_count + 1;				

				/* If Rts sent flag was enable then disable it as the		*/
				/* station will recontend for the channel.					*/
				if (wlan_flags->rts_sent == OPC_BOOLINT_ENABLED)
					wlan_flags->rts_sent = OPC_BOOLINT_DISABLED;
				
				/* Reset the NAV duration so that the retransmission is not	*/
				/* unnecessarily delayed.									*/
				if (cfp_ap_medium_control == OPC_BOOLINT_DISABLED)
					nav_duration = current_time;
				}
			}
						
		/* If network allocation vector is less than the received duration	*/
		/* value then update its value.  									*/
		if (nav_duration < (pk_chstruct_ptr->duration + current_time))
			{
			nav_duration = pk_chstruct_ptr->duration + current_time;				
			
			/* Set the flag that indicates updated NAV value.				*/
			wlan_flags->nav_updated = OPC_BOOLINT_ENABLED;
			}			
		}
	
	else if	((rcvd_frame_type == WlanC_Ack)	|| (rcvd_frame_type == WlanC_Cf_End) ||
			(rcvd_frame_type == WlanC_Cf_End_A))
		{				
		op_pk_nfd_access (wlan_rcvd_frame_ptr,"Wlan Header", &pk_chstruct_ptr); 		
		dest_addr = pk_chstruct_ptr->rx_addr;
		
		/* If PCF active and this is an AP */
		if ((wlan_flags->pcf_active == OPC_BOOLINT_ENABLED) && (ap_flag == OPC_BOOLINT_ENABLED))
			{
			/* Check if frame is intended for this station and increment failed poll count if necessary */
			if (dest_addr == my_address) 
				poll_fail_count = 0;
			else if (wlan_flags->active_poll == OPC_BOOLINT_ENABLED) 
				{
				poll_fail_count ++;
				}

			wlan_flags->active_poll = OPC_BOOLINT_DISABLED;
			
			/* clear side traffic flag if set since Ack is now received */
			wlan_flags->pcf_side_traf = OPC_BOOLINT_DISABLED;
			}
			
		/*  Control Traffic received report in terms of number of bits.		*/
		op_stat_write (ctrl_traffic_rcvd_handle_inbits, (double) op_pk_total_size_get (wlan_rcvd_frame_ptr));
		op_stat_write (ctrl_traffic_rcvd_handle_inbits, 0.0);

		/*  Control Traffic received report in terms of number of packets.	*/
		op_stat_write (ctrl_traffic_rcvd_handle, 1.0);
		op_stat_write (ctrl_traffic_rcvd_handle, 0.0);

		if (expected_frame_type == WlanC_Ack)
			{
			/* If an Ack is expected, and a DCF Ack is recieved for this destinaltion, or 	*/  
			/* CF_END+ACK is received, process the received ack.                            */
			if (((rcvd_frame_type == WlanC_Ack) && (dest_addr == my_address)) || 
				(rcvd_frame_type == WlanC_Cf_End_A))
				{
				/* Printing out information to ODB.	*/
				if (wlan_trace_active == OPC_TRUE)
					{
					if ((last_frametx_type == WlanC_Data) || (last_frametx_type == WlanC_Data_Ack))
						{
						sprintf (msg_string, "Ack received for data packet %d", pkt_in_service);
						op_prg_odb_print_major (msg_string, OPC_NIL);
						}
					else
						{
						sprintf (msg_string, "Ack received for data packet %d", pcf_pkt_in_service);
					  	op_prg_odb_print_major (msg_string, OPC_NIL);
						}
					}			

				if ((last_frametx_type == WlanC_Data) || (last_frametx_type == WlanC_Data_Ack))
					{
					op_stat_write (retrans_handle, (double) (retry_count * 1.0));

					/* Reset the retry counter as the expected frame is received	*/
					retry_count = 0;
				
					/* Decrement number of fragment count because one fragment is successfully transmitted.	*/
					num_fragments = num_fragments - 1;				
					}
				else
		        	{
					op_stat_write (retrans_handle, (double) (pcf_retry_count * 1.0));

					/* Reset the retry counter as the expected frame is received	*/
					pcf_retry_count = 0;

					/* Decrement number of fragment count because one fragment is successfully transmitted.	*/
					pcf_num_fragments = pcf_num_fragments - 1;				
                    }

				/* When there are no more fragments to transmit then disable the Rts sent flag	*/
				/* if it was enabled because the contention period due to Rts/Cts exchange is 	*/
				/* over and another Rts/Cts exchange is needed for next contention period.		*/
				if (num_fragments == 0)
					{
					wlan_flags->rts_sent = OPC_BOOLINT_DISABLED;
					
					/* Set the contention window flag. Since the ACK for the last 		*/
					/* fragment indicates a	sucessful transmission of the entire data, 	*/
					/* we need to back-off for a contention window period.				*/
					if (rcvd_frame_type == WlanC_Ack)
						wlan_flags->cw_required = OPC_TRUE;
					
					}

				/* Data packet is successfully delivered to remote station,			*/
				/* since no further retransmission is needed the copy of the data	*/
				/* packet will be destroyed.										*/
				if (((last_frametx_type == WlanC_Data) || (last_frametx_type == WlanC_Data_Ack)) &&
					(wlan_transmit_frame_copy_ptr != OPC_NIL)) 
					{
					op_pk_destroy (wlan_transmit_frame_copy_ptr);
					wlan_transmit_frame_copy_ptr = OPC_NIL;
					}
				else if (wlan_pcf_transmit_frame_copy_ptr != OPC_NIL)
					{
					op_pk_destroy (wlan_pcf_transmit_frame_copy_ptr);
					wlan_pcf_transmit_frame_copy_ptr = OPC_NIL;
					}
				}
			else
				{
				/* A frame has not been properly acknowledged and requires retransmission */
				retry_count = retry_count + 1;
				
				if (expected_frame_type == WlanC_Ack)
					{
					/* Printing out information to ODB.	*/
					if (wlan_trace_active == OPC_TRUE)
						{
						op_prg_odb_print_major ("Ack is received and discarded.", OPC_NIL);
						}
					}
				}
			}

		if (expected_frame_type == WlanC_Cts) 
			{
			/* Since the station did not receive the expected frame	*/
			/* it has to retransmit the packet.						*/
			retry_count = retry_count + 1;
			
			/* If Rts sent flag was enable then disable it as the station will recontend for the channel.	*/
			if (wlan_flags->rts_sent == OPC_BOOLINT_ENABLED)
				{
				wlan_flags->rts_sent = OPC_BOOLINT_DISABLED;
				}
			
			/* Reset the NAV duration so that the				*/
			/* retransmission is not unnecessarily delayed.		*/
			if (cfp_ap_medium_control == OPC_BOOLINT_DISABLED)
				nav_duration = current_time;
			}
		
		if ((rcvd_frame_type == WlanC_Cf_End) || (rcvd_frame_type == WlanC_Cf_End_A))
			{
			/* If a CFP end frame was received, indicate that availability of */
			/* the medium to the stations to contend to transmit frames		  */
			cfp_ap_medium_control = OPC_BOOLINT_DISABLED;
			
			/* If the STA should only respond to CF-End in its own BSS and this is   */
			/* from its BSS, orthe STA should resond to all CF-End regardless of BSS */
			if (bss_id == rcvd_sta_bssid)
				{
				/*  If end of cfp, reset NAV */
				nav_duration = current_time;

				/* Set the flag that indicates updated NAV value.			*/
				wlan_flags->nav_updated = OPC_BOOLINT_ENABLED;
				
				/* Printing out information to ODB.	*/
				if (wlan_trace_active == OPC_TRUE)
					{
					op_prg_odb_print_major ("CF_End frame is received.", OPC_NIL);
					}
				
				}
			}

		/* If network allocation vector is less than the received duration	*/
		/* value then update its value. 									*/
		if (nav_duration < (pk_chstruct_ptr->duration + current_time))
			{
			nav_duration = pk_chstruct_ptr->duration + current_time;

			/* Set the flag that indicates updated NAV value.				*/
			wlan_flags->nav_updated = OPC_BOOLINT_ENABLED;
			}
		
		}

	else 
		{
		/* Unknown frame type so declare error.								*/
		wlan_mac_error ("Unexpected frame type received.", OPC_NIL, OPC_NIL);
		}
   
	/* Report the amount of time the channel will be busy if the NAV is		*/
	/* updated with the received packet.									*/
	if (!(rcvd_frame_type == WlanC_Ack || rcvd_frame_type == WlanC_Cf_End || rcvd_frame_type == WlanC_Cf_End_A))
		{
		op_stat_write (channel_reserv_handle, (nav_duration - current_time));
		op_stat_write (channel_reserv_handle, 0.0);
		}
	
	/* Check whether further retries are possible or	*/
	/* the data frame needs to be discarded.			*/
	if ((wlan_flags->pcf_active == OPC_BOOLINT_ENABLED) && (ap_flag == OPC_BOOLINT_ENABLED))
		wlan_pcf_frame_discard ();
	else
		wlan_frame_discard ();

	/* Set the expected frame type to None because either the 	*/
	/* expected frame is recieved or the station will have to 	*/
	/* retransmit the frame										*/
	expected_frame_type = WlanC_None;
	
	/* Destroying the received frame once relevant information is taken out of it.	*/
	op_pk_destroy (wlan_rcvd_frame_ptr);														

	FOUT;
	}

Boolean
wlan_tuple_find (int sta_addr, int seq_id, int frag_num)
	{
	Boolean 							result = OPC_BOOLINT_DISABLED;
	int 								list_index;
	WlanT_Mac_Duplicate_Buffer_Entry*	tuple_ptr;

	/** This routine determines whether the received data frame already exists in the duplicate buffer.	**/
	/** If it is not then it will be added to the list and the list is updated such that its size will	**/
	/** will not be greater then the MAX TUPLE SIZE.													**/
	FIN (wlan_tuple_find (sta_addr, seq_id, frag_num));

	/* Finding the index of the station address in the list,	*/
	/* if the station belongs to this subnet.					*/
	list_index = oms_aa_address_find (oms_aa_wlan_handle, sta_addr);

	/* If remote station entry doesn't exist then create new node.	*/
	if (list_index >= 0)					
		{			
		if (duplicate_list_ptr [list_index] == OPC_NIL)
			{	
			/* Creating struct type for duplicate frame (or tuple) structure. 	*/
			tuple_ptr = (WlanT_Mac_Duplicate_Buffer_Entry *)
				                       op_prg_mem_alloc (sizeof (WlanT_Mac_Duplicate_Buffer_Entry));

			/* Generate error and abort simulation if no more memory left to allocate for duplicate buffer	*/
			if (tuple_ptr == OPC_NIL)
				{
				wlan_mac_error ("Cannot allocate memory for duplicate buffer entry", OPC_NIL, OPC_NIL);
				}	

			tuple_ptr->tx_station_address 	= remote_sta_addr;						
			tuple_ptr->sequence_id 			= seq_id;
			tuple_ptr->fragment_number 		= frag_num;

			/* Insert new entry in the list.	*/					
			duplicate_list_ptr [list_index] = tuple_ptr;						
			}			
		else
			{
			if (duplicate_list_ptr [list_index]->sequence_id == seq_id &&
				duplicate_list_ptr [list_index]->fragment_number == frag_num)
				{
				/* This will be set in the retry field of Acknowledgement.	*/
				duplicate_entry = 1;

				/* Break the routine as the packet is already received by the station.	*/
				FRET (OPC_TRUE);
				}
			else
				{
				/* Update the sequence id and fragment number fields of the	*/
				/* remote station in the duplicate buffer list. The list	*/
				/* maintains the sequence id and fragment number of the 	*/
				/* previously received frame from this remote station. 		*/
				duplicate_list_ptr [list_index]->sequence_id = seq_id;
				duplicate_list_ptr [list_index]->fragment_number = frag_num;							
				}
			}
        }
	else
		{
		/* Its not possible for a station to directly receive packet from a station that	*/
		/* does not exist in its BSS.														*/
		wlan_mac_error ("Receiving packet from a station that does not exist in this BSS", 
			"Possibly wrong destination address", "Please check the configuration");
		}

	/* This will be set in the retry field of Acknowledgement.	*/
	duplicate_entry = 0;

	/* Packet is not already received by the station.	*/ 
	FRET (OPC_FALSE);				
	}

	
static void
wlan_data_process (Packet* seg_pkptr,int dest_addr, int sta_addr, int final_dest_addr,
				int frag_num, int more_frag, int pkt_id)
	{
	char										msg_string [120];
	int	 										current_index;
	int 										list_index;
	int 										list_size;
	int											protocol_type;
	Packet*										copy_pkptr;
	Boolean										send_to_higher;
	WlanT_Mac_Defragmentation_Buffer_Entry*     defrag_ptr;
	
	/** This routine handles defragmentation process and also sends data to the	**/
	/** higher layer if all the fragments have been received by the station.	**/
	FIN (wlan_data_process (seg_pkptr, dest_addr, sta_addr, final_dest_addr, frag_num, more_frag, pkt_id));

	/* Defragmentation of the received data frame.					*/
	/* Inserting fragments into the reassembly buffer. There are	*/
	/* two possible cases:											*/
	/* 1. The remote station has just started sending the 			*/
	/* fragments and it doesn't exist in the list.					*/
	/* 2. The remote station does exist in the list and the 		*/
	/* and the new fragment is a series of fragments for the data 	*/
	/* packet.													  	*/

	/* Get the size of the defragmentation list.	*/
	list_size = op_prg_list_size (defragmentation_list_ptr);

	/* Initialize the current node index which will indicate whether	*/
	/* the entry for the station exists in the list.					*/
	current_index = -1;

	/* Searching through the list to find if the remote station address 	*/
	/* exists i.e. the source station has received fragments for this   	*/
	/* data packet before.													*/
	/* Also, removing entries from the defragmentation buffer which has		*/
	/* reached its maximum receieve lifetime.								*/
	for (list_index = 0; list_index < list_size; list_index++)
		{
		/* Accessing node of the list for search purposes.	*/						
		defrag_ptr = (WlanT_Mac_Defragmentation_Buffer_Entry*) 
						op_prg_list_access (defragmentation_list_ptr, list_index);

		/* Removing station entry if the receive lifetime has expired.	*/
		if ((current_time - defrag_ptr->time_rcvd) >= max_receive_lifetime)
			{
			/* Removing the partially completed fragment once its lifetime has reached.	*/
			defrag_ptr =(WlanT_Mac_Defragmentation_Buffer_Entry *)
					op_prg_list_remove (defragmentation_list_ptr, list_index);
			op_sar_buf_destroy (defrag_ptr->reassembly_buffer_ptr);					
			op_prg_mem_free (defrag_ptr);	

			/* Updating the total list size.	*/
			list_size = list_size - 1;
			}

		/* If the station entry already exists in the list then store its index for future use.	*/
		else if (remote_sta_addr == defrag_ptr->tx_station_address)
			{
			current_index = list_index;
			}
		}                             						

	/* If remote station entry doesn't exist then create new node	*/
	if (current_index == -1)											
		{
		/* If the entry of the station does not exist in the defrag list		*/
		/* and the fragment received is not the first fragment of the packet    */
		/* then it implies that the maximum receive lifetime of the packet		*/
		/* has expired. In this case the received packet will be destroyed and	*/
		/* the acknowledgement is sent to the receiver as specified by the		*/
		/* protocol.															*/
		if (frag_num > 0)
			{
			op_pk_destroy (seg_pkptr);
			FOUT;
			}

		/* Creating struct type for defragmentation structure	*/  						
		defrag_ptr = (WlanT_Mac_Defragmentation_Buffer_Entry *) op_prg_mem_alloc (sizeof (WlanT_Mac_Defragmentation_Buffer_Entry));

		/* Generate error and abort simulation if no more memory left to allocate for duplicate buffer	*/
		if (defrag_ptr == OPC_NIL)
			{
			wlan_mac_error ("Cannot allocate memory for defragmentation buffer entry", OPC_NIL, OPC_NIL);
			}	

		/* Source station address is store in the list for future reference.	*/
		defrag_ptr->tx_station_address = sta_addr;

		/* For new node creating a reassembly buffer	*/
		defrag_ptr->reassembly_buffer_ptr = op_sar_buf_create (OPC_SAR_BUF_TYPE_REASSEMBLY, OPC_SAR_BUF_OPT_DEFAULT);
		op_prg_list_insert (defragmentation_list_ptr, defrag_ptr, OPC_LISTPOS_TAIL);
		}

    /* Record the received time of this fragment.	*/
	defrag_ptr->time_rcvd = current_time;
					
	/* Insert fragment into the reassembly buffer 	*/
	op_sar_rsmbuf_seg_insert (defrag_ptr->reassembly_buffer_ptr, seg_pkptr);

	/* If this is the last fragment then send the data to higher layer.	*/
	if (more_frag == 0)
		{
		/* If no more fragments to rcv then send the data to higher		*/
		/* layer and increment rcvd fragment count.						*/
		seg_pkptr = op_sar_rsmbuf_pk_remove (defrag_ptr->reassembly_buffer_ptr);

		if (ap_flag == OPC_BOOLINT_ENABLED) 
			{
			/* If the address is not found in the address list then access point will sent the data to higher	*/
			/* layer for address resolution. Note that if destination address is same as AP's address then		*/
			/* the packet is sent to higher layer for address resolution. If the destination address is			*/
			/* broadcast address then the packet is both transmitted within the BSS and also forwarded to the	*/
			/* higher layer.																					*/
			if ((oms_aa_address_find (oms_aa_wlan_handle, final_dest_addr) >= 0 && final_dest_addr != my_address) ||
				final_dest_addr == MAC_BROADCAST_ADDR)
				{
				/* Printing out information to ODB.						*/
				if (wlan_trace_active == OPC_TRUE)
					{
					sprintf (msg_string, "All fragments of Data packet %d is received and enqueued for transmission within a subnet", pkt_id);
					op_prg_odb_print_major (msg_string, OPC_NIL);
					}

				/* If the destination address is broadcast address then	*/
				/* we need to send a copy also to the higher layer.		*/
				if (final_dest_addr == MAC_BROADCAST_ADDR)
					{
					copy_pkptr = op_pk_copy (seg_pkptr);
					send_to_higher = OPC_TRUE;
					}
				else
					{
					copy_pkptr = seg_pkptr;
					send_to_higher = OPC_FALSE;
					}
				  
				/* Make sure that we have all the fragments.			*/
				if (copy_pkptr == OPC_NIL)
					{
					if (wlan_trace_active == OPC_TRUE)
						{
						strcpy (msg_string, "The received frame was of 0 size and hence all received fragments are discarded.");
						op_prg_odb_print_major (msg_string, OPC_NIL);
						}
					}
				else
					{
					/* Enqueuing packet for transmission within a		*/
					/* subnet.											*/
					if (wlan_poll_list_member_find (final_dest_addr) == OPC_TRUE)
						{
						wlan_hlpk_enqueue (copy_pkptr, final_dest_addr, OPC_TRUE);
						}
					else
						{
						wlan_hlpk_enqueue (copy_pkptr, final_dest_addr, OPC_FALSE);
						wlan_flags->data_frame_to_send  = OPC_BOOLINT_ENABLED;
						}
					}
				}
			else
				send_to_higher = OPC_TRUE;
			
			/* Send the packet to the higher layer if not destined		*/
			/* within own BSS or if it has broadcast address as			*/
			/* destination address.										*/
			if (send_to_higher == OPC_TRUE)
				{				
				/* Update the local/global throughput and end-to-end	*/
				/* delay statistics based on the packet that will be	*/
				/* forwarded to the higher layer.						*/
				wlan_accepted_frame_stats_update (seg_pkptr);

				/* Set the contents of the LLC-destined ICI -- set the address	*/
				/* of the transmitting station.									*/
				if (op_ici_attr_set (llc_iciptr, "src_addr", remote_sta_addr) == OPC_COMPCODE_FAILURE)
					{
					wlan_mac_error ("Unable to set source address in LLC ICI.", OPC_NIL, OPC_NIL);
					}

				/* Set the destination address (this mainly serves to			*/
				/* distinguish packets received under broadcast conditions.)	*/
				if (op_ici_attr_set (llc_iciptr, "dest_addr", final_dest_addr) == OPC_COMPCODE_FAILURE)
					{
					wlan_mac_error("Unable to set destination address in LLC ICI.", OPC_NIL, OPC_NIL);
					}
		
				/* Set the protocol type field contained in the Wlan frame.	*/
				protocol_type = 0;
				if (op_ici_attr_set (llc_iciptr, "protocol_type", protocol_type) == OPC_COMPCODE_FAILURE)
					{
					wlan_mac_error("Unable to set protocol type in LLC ICI.", OPC_NIL, OPC_NIL);
					}				

				/* Printing out information to ODB.	*/
				if (wlan_trace_active == OPC_TRUE)
					{
					sprintf (msg_string, "All fragments of Data packet %d is received and sent to the higher layer", pkt_id);
					op_prg_odb_print_major (msg_string, OPC_NIL);
					}
				
				/* Setting an ici for the higher layer */
				op_ici_install (llc_iciptr);

				/* Sending data to higher layer through mac interface.	*/
				op_pk_send (seg_pkptr, outstrm_to_mac_if);
				}
			}
		else
			{
			/* If the station is a gateway and not an access point then do not send		*/
			/* data to higher layer for address resolution.  This is for not allowing   */
			/* data to go out of the Adhoc BSS. Except, in the case of broadcast		*/
			/* packets and packets addressed to this station. On the other hand, if we	*/
			/* are in a bridge/switch node and not AP enabled, then drop the packet.	*/
			if ((wlan_flags->gateway_flag == OPC_BOOLINT_ENABLED  && 
				dest_addr != my_address && dest_addr >= 0) || 
				wlan_flags->bridge_flag == OPC_BOOLINT_ENABLED)
				{				
				/* Printing out information to ODB.	*/
				if (wlan_trace_active == OPC_TRUE)
					{
					strcpy (msg_string, "Gateway is not an access point so all received fragments are discarded.");
					op_prg_odb_print_major (msg_string, OPC_NIL);
					}
				op_pk_destroy (seg_pkptr);
				}
			else
				{
				/* Update the local/global throughput and end-to-end	*/
				/* delay statistics based on the packet that will be	*/
				/* forwarded to the higher layer.						*/
				wlan_accepted_frame_stats_update (seg_pkptr);
				
				/* Printing out information to ODB.	*/
				if (wlan_trace_active == OPC_TRUE)
					{
					sprintf (msg_string, "All fragments of Data packet %d is received and sent to the higher layer", pkt_id);
					op_prg_odb_print_major (msg_string, OPC_NIL);
					}

				/* Sending data to higher layer through mac interface	*/
				op_pk_send (seg_pkptr, outstrm_to_mac_if);
				}
			}
			
		/* Freeing up memory space once the received data frame is sent to higher layer.	*/
		defrag_ptr =(WlanT_Mac_Defragmentation_Buffer_Entry *)
					op_prg_list_remove (defragmentation_list_ptr, current_index);
		op_sar_buf_destroy (defrag_ptr->reassembly_buffer_ptr);					
		op_prg_mem_free (defrag_ptr);	
		}
	else
		{
		/* Printing out information to ODB.	*/
		if (wlan_trace_active == OPC_TRUE)
			{			
			sprintf (msg_string, "Data packet %d is received and waiting for more fragments ", pkt_id);
			op_prg_odb_print_major (msg_string, OPC_NIL);
			}
		}

	FOUT;
	}

static void
wlan_accepted_frame_stats_update (Packet* seg_pkptr)
	{
	double	ete_delay, pk_size;
	
	/** This function is called just before a frame received from	**/
	/** physical layer being forwarded to the higher layer to		**/
	/** update end-to-end delay and throughput statistics.			**/
	FIN (wlan_accepted_frame_stats_update (seg_pkptr));
	
	/* Total number of bits sent to higher layer is equivalent to a	*/
	/* throughput.													*/
	pk_size = (double) op_pk_total_size_get (seg_pkptr);
	op_stat_write (throughput_handle, pk_size);
	op_stat_write (throughput_handle, 0.0);

	/* Also update the global WLAN throughput statistic.			*/
	op_stat_write (global_throughput_handle, pk_size);
	op_stat_write (global_throughput_handle, 0.0);
	
	/* Compute the end-to-end delay for the frame and record it.	*/
	ete_delay = current_time - op_pk_stamp_time_get (seg_pkptr);
	op_stat_write (ete_delay_handle, 		ete_delay);
	op_stat_write (global_ete_delay_handle, ete_delay);
	
	FOUT;
	}


static void 
wlan_schedule_deference ()
	{
	/** This routine schedules self interrupt for deference **/
	/** to avoid collision and also deference to observe	**/
	/** interframe gap between the frame transmission.	**/
	FIN (wlan_schedule_deference ());

	/* Check the status of the receiver. If it is busy, exit the	*/
	/* function, since we will schedule the end of the deference	*/
	/* when it becomes idle.										*/
	if (wlan_flags->receiver_busy == OPC_BOOLINT_ENABLED)
		{
		FOUT;
		} 

	/* Extracting current time at each interrupt.				*/
	current_time = op_sim_time ();
	
	/* Adjust the NAV if necessary.								*/
	if (nav_duration < rcv_idle_time)
		{
		nav_duration = rcv_idle_time;
		}
	
	if (fresp_to_send == WlanC_Ack && IN_CFP) 
		{
		/* ACK over rides receiver busy signal (see 9.2.8).		*/
	
		/* Set deference interrupt for SIFS time.				*/
		deference_evh = op_intrpt_schedule_self (current_time + sifs_time, WlanC_Deference_Off);
		
		/* Disable backoff since not used with SIFS.			*/
		wlan_flags->backoff_flag = OPC_BOOLINT_DISABLED;
		wlan_flags->perform_cw   = OPC_BOOLINT_DISABLED;
		
		/* Set ignore busy flag to prevent canceling of			*/
		/* deference by receiver busy.							*/
		wlan_flags->ignore_busy = OPC_BOOLINT_ENABLED;
		}
	
	else if	(fresp_to_send != WlanC_Ack && wlan_flags->tx_beacon == OPC_BOOLINT_ENABLED && 
		     wlan_flags->pcf_active == OPC_BOOLINT_DISABLED)
		{
		/* If it is time to send a Beacon frame and if this is going to	*/
		/* be the beginning of a new CFP, use a PIFS time for deference.*/
		/* Check if PIFS deference already satisfied. If satisfied		*/
		/* start the transmission immediately. Else schedule PIFS based	*/
		/* on last medium idle time. Note that Beacon currently does	*/
		/* not defer to NAV.											*/
		if	(current_time > (rcv_idle_time + pifs_time))
			deference_evh = op_intrpt_schedule_self (current_time, WlanC_Deference_Off);
		else
			deference_evh = op_intrpt_schedule_self (rcv_idle_time + pifs_time, WlanC_Deference_Off);
		
		/* Since we are going to transmit a Beacon frame disable the	*/
		/* backoff and comtention window flags.							*/
		wlan_flags->backoff_flag = OPC_BOOLINT_DISABLED;
		wlan_flags->perform_cw   = OPC_BOOLINT_DISABLED;
		}

	else if (wlan_flags->pcf_active == OPC_BOOLINT_ENABLED)
		{
		/* PCF is active, so follow PCF deference and backoff rules. 	*/

		/* Disable backoff because this is the CFP (backoff is actually optional)	*/
		/* Note that a mandatory periodic DIFS + backoff during the CFP is not 		*/
		/* implemented.  (See 9.3.3.2)												*/
		wlan_flags->backoff_flag = OPC_BOOLINT_DISABLED;		
		wlan_flags->perform_cw   = OPC_BOOLINT_DISABLED;

		if (wlan_flags->pcf_side_traf == OPC_BOOLINT_ENABLED)
			{
			/* If last frame was side traffic must defer for ACK as well. Side traffic		*/
			/* requires an Ack from a station other than the AP and the AP may not be 		*/
			/* able to hear the Ack so AP should defer till the Ack would be completed.		*/
			/* Note that allowing side traffic in an infastructure network was voted out	*/
			/* of the 802.11 standard and should not be permited. However, the standard was	*/
			/* never fully updated and the wording is currently contradictory on this		*/
			/* point.  The side traffic capability can be shut off in the MAC model			*/
			/* attributes.	To account for side traffic poll response will set the duration	*/
			/* field to account for a WLAN_ACK duration and SIFS.  This is in violation of	*/
			/* the protocol since the duration field should be set to 32768 (0).  However,	*/
			/* since this whole thing is forbidden anyway, some liberties may be taken.		*/
			/* The AP will update it's NAV, and automatically defer the right amount of 	*/
			/* time.																		*/
			deference_evh = op_intrpt_schedule_self (nav_duration + sifs_time , WlanC_Deference_Off);
			wlan_flags->pcf_side_traf = OPC_BOOLINT_DISABLED;
			}

		else if ((ap_flag == OPC_BOOLINT_ENABLED) && (poll_fail_count != 0)	&&
				(wlan_flags->active_poll == OPC_BOOLINT_ENABLED))
			{
			/* If this is an AP, and a transmission has failed, already waited for 		*/
			/* pifs_time when waiting for response - don't need additional deference.	*/
			deference_evh = op_intrpt_schedule_self (current_time, WlanC_Deference_Off);
			}
	    else
			{
			/* Normal PCF transmission so use a sifs_time.								*/
			deference_evh = op_intrpt_schedule_self (current_time + sifs_time, WlanC_Deference_Off);
			}

		/* Set ignore busy flag to prevent canceling of deference by receiver busy.		*/
		wlan_flags->ignore_busy = OPC_BOOLINT_ENABLED;
		}
	
	else 
		{
		/* DCF is active, so follow DCF deference and backoff rules. 	*/
	
		/* Station needs to wait SIFS duration before responding to any */	
		/* frame. Also, if Rts/Cts is enabled then the station needs	*/
		/* to wait for SIFS duration after acquiring the channel using	*/
		/* Rts/Cts exchange.						*/
		if ((fresp_to_send != WlanC_None) || (wlan_flags->rts_sent == OPC_BOOLINT_ENABLED))
			{ 
			deference_evh = op_intrpt_schedule_self (current_time + sifs_time, WlanC_Deference_Off);

			/* Disable backoff and CW flags because this frame is a response frame	*/
			/* to the previously received frame (this could be Ack or Cts).			*/
			wlan_flags->backoff_flag = OPC_BOOLINT_DISABLED;
			wlan_flags->perform_cw   = OPC_BOOLINT_DISABLED;
			}

		/* If more fragments to send then wait for SIFS duration and transmit. 		*/
		/* Station need to contend for the channel if one of the fragments is not	*/
		/* successfully transmitted.												*/
		else if ((retry_count == 0) && (op_sar_buf_size (fragmentation_buffer_ptr) > 0))
			{
			/* Scheduling a self interrupt after SIFS duration.						*/
			deference_evh = op_intrpt_schedule_self (current_time + sifs_time, WlanC_Deference_Off);
		
			/* Disable backoff because the frame need to be transmitted after SIFS	*/
			/* duration. This frame is part of the fragment burst.					*/
			wlan_flags->backoff_flag = OPC_BOOLINT_DISABLED;
			
			}
	
		/* If the last frame we received was corrupted, which indicates a			*/
		/* collision, then we need to defer for a longer period, i.e. for EIFS		*/
		/* period, which starts when the medium becomes idle.						*/
		else if (wlan_flags->wait_eifs_dur == OPC_BOOLINT_ENABLED)
			{
			/* EIFS is required. We need to use the larger of rcv_idle_time + EIFS	*/
			/* and NAV + DIFS since EIFS period starts when the receiver becomes	*/
			/* idle regardless of the status of virtual carrier sensing (section	*/
			/* 9.2.3.4).															*/
			if (rcv_idle_time + eifs_time >= nav_duration + difs_time)
				deference_evh = op_intrpt_schedule_self ((rcv_idle_time + eifs_time), WlanC_Deference_Off);
			else
				deference_evh = op_intrpt_schedule_self ((nav_duration + difs_time), WlanC_Deference_Off);

			/* After an EIFS period, a backoff is needed.							*/
			if (wlan_flags->cw_required == OPC_BOOLINT_ENABLED)
				wlan_flags->perform_cw = OPC_BOOLINT_ENABLED;
			else
				wlan_flags->backoff_flag = OPC_BOOLINT_ENABLED;
		
			/* Reset the EIFS flag.													*/
			wlan_flags->wait_eifs_dur = OPC_BOOLINT_DISABLED;
			}
	
		/* If we are in contention window period, which follows a successful		*/
		/* packet trasnmission, set the flag to trigger the backoff period.			*/
		else if ((wlan_flags->cw_required == OPC_BOOLINT_ENABLED) && 
			(wlan_flags->polled == OPC_BOOLINT_DISABLED))
			{
			wlan_flags->perform_cw = OPC_BOOLINT_ENABLED;
			
			/* We defer for NAV duration plus DIFS duration before resuming for CW	*/
			/* backoff period.														*/
			deference_evh = op_intrpt_schedule_self ((nav_duration + difs_time), WlanC_Deference_Off);		
			}

		else if ((((fresp_to_send != WlanC_None) || 
				(wlan_flags->rts_sent == OPC_BOOLINT_ENABLED) ||
				((op_sar_buf_size (fragmentation_buffer_ptr) > 0) && (retry_count == 0))) &&
				(wlan_flags->pcf_active == OPC_BOOLINT_ENABLED)) || 
				((wlan_flags->polled == OPC_BOOLINT_ENABLED) ||
				((op_sar_buf_size (fragmentation_buffer_ptr) > 0) && (retry_count == 0))))
			{
			/* Station only waits a SIFS duration before sending a response frame, 		*/
			/* contiuing a Rts/Cts sequence in progress, or if additional fragments of 	*/
			/* a MSDU are being sent.  Also no back off is required as long as no 		*/
			/* retrys have been (or are) required. 										*/
			deference_evh = op_intrpt_schedule_self (current_time + sifs_time, WlanC_Deference_Off);

			/* Disable the backoffs since not needed if SIFS time being used.			*/
			wlan_flags->backoff_flag = OPC_BOOLINT_DISABLED;
			wlan_flags->perform_cw   = OPC_BOOLINT_DISABLED;

			/* Set ignore busy flag to prevent canceling of deference by receiver busy.	*/
			wlan_flags->ignore_busy = OPC_BOOLINT_ENABLED;
			}
		
		else
			{
			/* If the station needs to transmit or retransmit frame, it will */
			/* defer for NAV duration plus  DIFS duration and then backoff   */
			deference_evh = op_intrpt_schedule_self ((nav_duration + difs_time), WlanC_Deference_Off);		
		
			/* Before sending data frame or Rts backoff is needed. */
			wlan_flags->backoff_flag = OPC_BOOLINT_ENABLED;
			}
		}
	
	/* Reset the updated NAV flag, since as of now we scheduled a new	*/
	/* "end of deference" interrupt after the last update.				*/
	wlan_flags->nav_updated = OPC_BOOLINT_DISABLED;
	
	FOUT;
	}

static void
wlan_frame_discard ()
	{
	int seg_bufsize;
	Packet* seg_pkptr;

	/** No further retries for the data frame for which the retry limit has reached.	**/
	/** As a result these frames are discarded.											**/
	FIN (wlan_frame_discard ());

	/* If retry limit has reached then drop the frame.	*/
	if (retry_count == retry_limit)
		{
		/* Update retransmission count statistic.	*/						
		op_stat_write (retrans_handle, (double) (retry_count * 1.0));

		/* Update the local and global dropped packet statistics.	*/
		op_stat_write (drop_packet_handle, 1.0);
	    op_stat_write (drop_packet_handle, 0.0);
		op_stat_write (drop_packet_handle_inbits, (double) packet_size_dcf);
	    op_stat_write (drop_packet_handle_inbits, 0.0);
		op_stat_write (global_dropped_data_handle, (double) packet_size_dcf);
		op_stat_write (global_dropped_data_handle, 0.0);

		/* Reset the retry count for the next packet.	*/
		retry_count = 0;

		/* Get the segmenation buffer size to check if there are more fragments left to be transmitted.	*/
		seg_bufsize =  (int) op_sar_buf_size (fragmentation_buffer_ptr); 

		if (seg_bufsize != 0)
			{
			/* Discard remaining fragments	*/
			seg_pkptr = op_sar_srcbuf_seg_remove (fragmentation_buffer_ptr, seg_bufsize);
			op_pk_destroy (seg_pkptr);
			}

		/* If expecting Ack frame then destroy the tx data frame as this frame will	*/
		/* no longer be transmitted (even if we are not expecting an Ack at this	*/
		/* moment, we still may have a copy of the frame if at one point in the		*/
		/* retransmission history of the original packet we received a Cts for our	*/
		/* Rts but then didn't receive an Ack for our data transmission; hence		*/
		/* consider this case as well).												*/
		if ((expected_frame_type == WlanC_Ack) || (wlan_transmit_frame_copy_ptr != OPC_NIL))
			{	
			/* Destroy the copy of the frame as the packet is discarded.	*/
			op_pk_destroy (wlan_transmit_frame_copy_ptr);
			wlan_transmit_frame_copy_ptr = OPC_NIL;
			}
			
		/* Reset the flag that indicates successful RTS transmission.		*/
		wlan_flags->rts_sent = OPC_BOOLINT_DISABLED;
		
		/* Reset the "frame to respond" variable unless we have a CTS or  	*/
		/* ACK to send.														*/
		if (fresp_to_send == WlanC_Data)
			{
			fresp_to_send = WlanC_None;
			}
		
		/* If there is not any other data packet sent from higher layer and	*/
		/* waiting in the buffer for transmission, reset the related flag.	*/
		if (op_prg_list_size (hld_list_ptr) == 0)
			{
			wlan_flags->data_frame_to_send = OPC_BOOLINT_DISABLED;
			}
		}
	
	FOUT;
	}

static void
wlan_pcf_frame_discard ()
	{
	int 		seg_bufsize;
	Packet* 	seg_pkptr;
	
	/** No further retries for the data frame for which the retry limit has reached.	**/
	/** As a result these frames are discarded.											**/
	FIN (wlan_pcf_frame_discard ());

	/* If retry limit has reached then drop the frame.	*/
	if (pcf_retry_count == retry_limit)
		{
		/* Update retransmission count statistic.	*/						
		op_stat_write (retrans_handle, (double) (pcf_retry_count * 1.0));

		/* Update the local and global dropped packet statistics.	*/
		op_stat_write (drop_packet_handle, 1.0);
	    op_stat_write (drop_packet_handle, 0.0);
		op_stat_write (drop_packet_handle_inbits, (double) packet_size_pcf);
	    op_stat_write (drop_packet_handle_inbits, 0.0);
		op_stat_write (global_dropped_data_handle, (double) packet_size_pcf);
		op_stat_write (global_dropped_data_handle, 0.0);

		/* Reset the retry count for the next packet.	*/
		pcf_retry_count = 0;

		if (pcf_frag_buffer_ptr != OPC_NIL)
			{
			
			/* Get the segmenation buffer size to check if there are more fragments left to be transmitted.	*/
			seg_bufsize =  (int) op_sar_buf_size (pcf_frag_buffer_ptr); 

			if (seg_bufsize != 0)
				{
				/* Discard remaining fragments	*/
				seg_pkptr = op_sar_srcbuf_seg_remove (pcf_frag_buffer_ptr, seg_bufsize);
				
				if (seg_pkptr != OPC_NIL)
					op_pk_destroy (seg_pkptr);
				}

			/* If expecting Ack frame or too many polls, then destroy the 	*/
			/* tx data frame as this frame will no longer be transmitted.	*/
			if ((expected_frame_type == WlanC_Ack) || (poll_fail_count > max_poll_fails) ||
				(wlan_pcf_transmit_frame_copy_ptr != OPC_NIL))
				{	
				/* Destroy the copy of the frame as the packet is discarded.	*/
				
				if (wlan_pcf_transmit_frame_copy_ptr != OPC_NIL)
					{
					op_pk_destroy (wlan_pcf_transmit_frame_copy_ptr);
					}
					
				wlan_pcf_transmit_frame_copy_ptr = OPC_NIL;
				}
			}

		/* Reset the "frame to respond" variable unless we have a CTS or  	*/
		/* ACK to send.														*/
		if (fresp_to_send == WlanC_Data)
			{
			fresp_to_send = WlanC_None;
			}
		}
	
	FOUT;
	}


static void
wlan_mac_rcv_channel_status_update (int channel_id)
	{
	int		mask = 1;
	double	rx_end_time;

	/** This function updates the status of the receiver's	**/
	/** channel by setting or resetting the corresponding 	**/
	/** bit in the rcv_channel_status state variable based	**/
	/** the channel from which the stat interrupt is 		**/
	/** received and the value of that channel's statwire.	**/
	FIN (wlan_mac_rcv_channel_status_update (int channel_id));
	
	/* Create a mask which will access the corresponding	*/
	/* bit of the channel that is changing its status.		*/
	mask = mask << channel_id;
		
	/* Set the bit to 1 if channel became busy and to 0 if	*/
	/* the channel became idle without changing the other	*/
	/* bits.												*/
	if (op_stat_local_read (channel_id) > rx_power_threshold)
		{
		/* A new channel became busy. If we did not detect	*/
		/* a collision before and if this is not the only	*/
		/* channel which is busy, then set the collision	*/
		/* flag to true.									*/
		if (wlan_flags->collision == OPC_BOOLINT_DISABLED && rcv_channel_status > 0)
			{
			wlan_flags->collision = OPC_BOOLINT_ENABLED;
			
			/* We need to discard the first packet we will	*/
			/* receive from the receiver after this moment	*/
			/* in any case, since it is collided. Hence,	*/
			/* set the corresponding flag.					*/
			wlan_flags->collided_packet = OPC_BOOLINT_ENABLED;
			}
		
		/* Set the corresponding bit to 1 for the channel	*/
		/* that became busy.								*/
		rcv_channel_status = rcv_channel_status | mask;
		}
	
	/* Else a packet reception is complete. Check whether	*/
	/* the receiver became available if it was busy. It may	*/
	/* not be busy if we were receving a noise packet with	*/
	/* a weak signal.										*/
	else if (rcv_channel_status != 0)
		{
		/* Compare the reception end time attribute value	*/
		/* with the current time to determine the			*/
		/* receiver's status.								*/
		op_ima_obj_attr_get (rx_objid, "reception end time", &rx_end_time);
		if (rx_end_time - PRECISION_RECOVERY <= op_sim_time())
			{		
			rcv_channel_status = 0;
			wlan_flags->collision = OPC_BOOLINT_DISABLED;
			}
		}
	
	FOUT;
	}

/****** Error handling procedure ******/
static void
wlan_mac_error (const char* msg1, const char* msg2, const char* msg3)
	{
	
	/** Terminates simulation with an error message.	**/
	FIN (wlan_mac_error (msg1, msg2, msg3));

	op_sim_end ("Error in Wireless LAN MAC process:", msg1, msg2, msg3);

	FOUT;
	}

Boolean
wlan_poll_list_member_find (int dest_addr) 
	{
	int		i;

	/** This routine determines whether a given address is on the polling list.	**/
	FIN (wlan_poll_list_member_find(dest_addr));

	/* if PCF not enabled or not an AP, then don't use polling list */
	if ((ap_flag == OPC_BOOLINT_DISABLED) || (pcf_flag == OPC_BOOLINT_DISABLED))
		{FRET (OPC_FALSE);}

	/* If this is an AP, PCF is active, and this is a broadcast packet */
	/* send under PCF */ 
	if (dest_addr == -1) {FRET (OPC_TRUE);}

	/* Otherwise, check if address on polling list */
	for (i = 0; i < poll_list_size; i++ )
		{
		if (dest_addr == polling_list[i]) {FRET (OPC_TRUE);}
		if (dest_addr < polling_list[i]) {FRET (OPC_FALSE);}
		}

	/* Destination address is not on polling list.	*/ 
	FRET (OPC_FALSE);				
	}

int
wlan_hld_list_elem_add_comp (WlanT_Hld_List_Elem* hld_ptr1,  WlanT_Hld_List_Elem* hld_ptr2)
	{
	/* This procedure is used in list processing to sort lists and find members of lists	*/
	/* containing higher layer data packets according to their MAC address destinations.	*/
	/* It returns 1 if hld_ptr1 is at a lower address than hld_ptr2 (closer to list head).	*/
	/* It returns -1 if hld_ptr1 is at a higher address than hld_ptr2 (closer to list tail).*/
	/* It returns 0 if the addresses associated with the two elements are equal.			*/

	FIN (wlan_hld_list_elem_add_comp(WlanT_Hld_List_Elem* hld_ptr1,  WlanT_Hld_List_Elem* hld_ptr2));

	if (hld_ptr1->destination_address > hld_ptr2->destination_address) {FRET (-1);}
	if (hld_ptr1->destination_address < hld_ptr2->destination_address) {FRET (1);}
	else {FRET (0);}

	}

static void
wlan_frame_type_conv(int frame_type,char *frame_type_name) 
	{
	/** This routine converts a frame type to a string containing the type name	**/
	FIN (wlan_frame_type_conv(int frame_type, char *frame_type_name));
	switch (frame_type)
		{
		case WlanC_Beac:
			{
			strcpy(frame_type_name,"WlanC_Beac");
			break;
			}
		case WlanC_Rts:
			{
			strcpy(frame_type_name,"WlanC_Rts");
			break;
			}
		case WlanC_Cts:
			{
			strcpy(frame_type_name,"WlanC_Cts");
			break;
			}
		case WlanC_Ack:
			{
			strcpy(frame_type_name,"WlanC_Ack");
			break;
			}
		case WlanC_Cf_End:
			{
			strcpy(frame_type_name,"WlanC_Cf_End");
			break;
			}
		case WlanC_Cf_End_A:
			{
			strcpy(frame_type_name,"WlanC_Cf_End_A");
			break;
			}
		case WlanC_Data:
			{
			strcpy(frame_type_name,"WlanC_Data");
			break;
			}
		case WlanC_Data_Ack:
			{
			strcpy(frame_type_name,"WlanC_Data_Ack");
			break;
			}
		case WlanC_Data_Poll:
			{
			strcpy(frame_type_name,"WlanC_Data_Poll");
			break;
			}
		case WlanC_Data_A_P:
			{
			strcpy(frame_type_name,"WlanC_Data_A_P");
			break;
			}
		case WlanC_Data_Null:
			{
			strcpy(frame_type_name,"WlanC_Data_Null");
			break;
			}
		case WlanC_Cf_Ack:
			{
			strcpy(frame_type_name,"WlanC_Cf_Ack");
			break;
			}
		case WlanC_Cf_Poll:
			{
			strcpy(frame_type_name,"WlanC_Cf_Poll");
			break;
			}
		case WlanC_Cf_A_P:
			{
			strcpy(frame_type_name,"WlanC_Cf_A_P");
			break;
			}
		case WlanC_None:
			{
			strcpy(frame_type_name,"WlanC_None");
			break;
			}
		default:
			{
			wlan_mac_error ("Unkown frame type: unable to convert to char", OPC_NIL, OPC_NIL);
			break;
			}
		}
	
	FOUT;			
	}



//-----------------------------------------------------------
//
//			   ROUTES MANAGEMENT
//
//-----------------------------------------------------------


//Prints the content of a route (route_length, source, ..., destination)
void mac_print_route(List* route_tmp){
	int		i;
	int		*int_ptr;

	printf("ROUTE : ");
	for(i=0 ; i < op_prg_list_size(route_tmp) ; i++){
		int_ptr = op_prg_list_access(route_tmp, i);
	
		printf(" %d" , *int_ptr);
		}
	printf("\n");
}


//returns the route length
short get_route_length(List* route_tmp){
	int		*int_ptr;
	
	//route length -> firt elem
	int_ptr = op_prg_list_access(route_tmp , 0);
	return(*int_ptr);
}


//Returns the average route length
double get_average_route_length(List *route_list){
	//final values
	int		value = 0;
	int		nb_routes = 0;
	//Elems
	int		*int_ptr;
	List	*route_tmp;
	//control
	int		i;
	
	
	for(i=0 ; i < op_prg_list_size(route_list) ; i++){
	
		//Gets the it^h route
		route_tmp = op_prg_list_access(route_list , i);
		
		//Its length
		int_ptr = op_prg_list_access(route_tmp , 0);
		
		//Update values
		value +=  *int_ptr;
		if (*int_ptr != 0)
			nb_routes++;
	}
	
	return(value / nb_routes);
}


//is addr in the route ?
Boolean is_in_route(List* route_tmp , int addr){
	int		i;
	int		*int_ptr;
	
	//NB : the first element is the route length
	for(i=1 ; i < op_prg_list_size(route_tmp) ; i++){
		int_ptr = op_prg_list_access (route_tmp , i);
		if (*int_ptr == addr)
			return(OPC_TRUE);
	}
	return(OPC_FALSE);
}

//returns the ratio of routes which pass through addr
double get_ratio_of_traffic(List* route_list , int addr){
	//final values
	int		value = 0;
	int		nb_routes = 0;
	//Elems
	int		*int_ptr;
	List	*route_tmp;
	//control
	int		i;
	
	
	for(i=0 ; i < op_prg_list_size(route_list) ; i++){
	
		//Gets the it^h route
		route_tmp = op_prg_list_access(route_list , i);
		
		//Its length
		if (is_in_route(route_tmp , addr))
			value ++;

		//Number of routes
		int_ptr = op_prg_list_access(route_tmp , 0);
		if (*int_ptr != 0)
			nb_routes++;
	}
	
	return( (double)value / (double)nb_routes);
}

/* End of Function Block */

/* Undefine optional tracing in FIN/FOUT/FRET */
/* The FSM has its own tracing code and the other */
/* functions should not have any tracing.		  */
#undef FIN_TRACING
#define FIN_TRACING

#undef FOUTRET_TRACING
#define FOUTRET_TRACING

#if defined (__cplusplus)
extern "C" {
#endif
	void wlan_mac_wifib (void);
	Compcode wlan_mac_wifib_init (void **);
	void wlan_mac_wifib_diag (void);
	void wlan_mac_wifib_terminate (void);
	void wlan_mac_wifib_svar (void *, const char *, char **);
#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
wlan_mac_wifib (void)
	{
	int _block_origin = 0;
	FIN (wlan_mac_wifib ());
	if (1)
		{
		/* variables used for registering and discovering process models */
		OmsT_Pr_Handle			process_record_handle;
		List*					proc_record_handle_list_ptr;
		int						record_handle_list_size;
		int						ap_count;
		int						count;
		double					sta_addr;
		double					statype ;
		Objid					mac_objid;
		Objid					mac_if_module_objid;
		char					proc_model_name [300];
		Objid					params_attr_objid;
		Objid					wlan_params_comp_attr_objid;
		Objid					strm_objid;
		int						strm;
		int						i_cnt,j_cnt, k_cnt;
		int						addr_index;
		int						num_out_assoc;
		int						node_count;
		int						node_objid;
		WlanT_Hld_List_Elem*	hld_ptr;
		Prohandle				own_prohandle;
		double					timer_duration;
		double					cw_slots;
		char					msg1 [120];
		char					msg2 [120];
		WlanT_Phy_Char_Code		sta_phy_char_flag;
		Boolean					bad_packet_rcvd = OPC_BOOLINT_DISABLED;
		int						pre_rx_status;
		double					pcf_active;
		int						address;
		int						pcf_enabled_stations;
		Boolean					pcf_enabled_on_AP;


		FSM_ENTER (wlan_mac_wifib)

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (INIT) enter executives **/
			FSM_STATE_ENTER_UNFORCED_NOLABEL (0, "INIT", "wlan_mac_wifib () [INIT enter execs]")
				{
				/* Initialization of the process model.				*/  
				/* All the attributes are loaded in this routine	*/
				wlan_mac_sv_init (); 
				 
				/* Schedule a self interrupt to wait for mac interface 	*/
				/* to move to next state after registering				*/
				op_intrpt_schedule_self (op_sim_time (), 0);
				
				//Get the routes in the network
				mac_all_routes = op_prg_list_create();
				
				op_ima_sim_attr_get(OPC_IMA_INTEGER , "BACKOFF_TYPE" , &backoff_type);
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (1,wlan_mac_wifib)


			/** state (INIT) exit executives **/
			FSM_STATE_EXIT_UNFORCED (0, "INIT", "wlan_mac_wifib () [INIT exit execs]")
				{
				/* Obtain the process's process handle	*/
				own_prohandle = op_pro_self ();
				
				/* Obtain the values assigned to the various attributes	*/
				op_ima_obj_attr_get (my_objid, "Wireless LAN Parameters", &wlan_params_comp_attr_objid);
				params_attr_objid = op_topo_child (wlan_params_comp_attr_objid, OPC_OBJTYPE_GENERIC, 0);
				
				/* Obtain the name of the process	*/
				op_ima_obj_attr_get (my_objid, "process model", proc_model_name);
				
				/* Determine the assigned MAC address which will be used for address resolution.	*/
				/* Note this is not the final MAC address as there may be static assignments in 	*/
				/* the network.																		*/
				op_ima_obj_attr_get (my_objid, "Address", &my_address);
				
				/* Perform auto-addressing for the MAC address. Apart	*/
				/* from dynamically addressing, if auto-assigned, the	*/
				/* address resolution function also detects duplicate	*/
				/* static assignments. The function also initializes 	*/
				/* every MAC address as a valid destination.			*/
				oms_aa_address_resolve (oms_aa_handle, my_objid, &my_address);
				
				/* Register Wlan MAC process in the model wide registry	*/
				own_process_record_handle = (OmsT_Pr_Handle) oms_pr_process_register (my_node_objid, my_objid, own_prohandle, proc_model_name);
				
				/* Initialize a temp variable for station type (AP/STA) */
				if (ap_flag == OPC_BOOLINT_ENABLED) 
					statype = 1.0;
				else 
					statype = 0.0;
				
				/* Initialize a temp variable for pcf_usage */
				if (pcf_flag == OPC_BOOLINT_ENABLED) 
					pcf_active = 1.0;
				else 
					pcf_active = 0.0;
				
				
				/* If this station is an access point then it has to be registered as an Access Point.	*/
				/* This is because the network will be treated as Infrastructure network once AP is		*/
				/* detected.																			*/
				if (ap_flag == OPC_BOOLINT_ENABLED)
					{
					oms_pr_attr_set (own_process_record_handle,
						"protocol",				OMSC_PR_STRING,			"mac",
						"mac_type",				OMSC_PR_STRING,			"wireless_lan",
						"subprotocol", 			OMSC_PR_NUMBER,			(double) WLAN_AP,
						"domain_id",			OMSC_PR_NUMBER,			(double) bss_id,
						"subnetid",				OMSC_PR_OBJID,		    my_subnet_objid,
						"ratx_objid",			OMSC_PR_OBJID,			tx_objid,
						"rarx_objid",			OMSC_PR_OBJID,			rx_objid,
						"address",			    OMSC_PR_NUMBER,			(double) my_address,
						"auto address handle",  OMSC_PR_ADDRESS,  	    oms_aa_handle,
						"PCF active",			OMSC_PR_NUMBER,			pcf_active,
						OPC_NIL);                                       
					}
				else
					{
				   	oms_pr_attr_set (own_process_record_handle,
						"protocol",				OMSC_PR_STRING,			"mac",
						"mac_type",				OMSC_PR_STRING,			"wireless_lan",
						"subprotocol", 			OMSC_PR_NUMBER,			(double) WLAN_STA,
						"domain_id",			OMSC_PR_NUMBER,		    (double) bss_id,
						"subnetid",				OMSC_PR_OBJID,			my_subnet_objid,
						"ratx_objid",			OMSC_PR_OBJID,			tx_objid,
						"rarx_objid",			OMSC_PR_OBJID,			rx_objid,
						"address",				OMSC_PR_NUMBER,			(double) my_address,
						"auto address handle",	OMSC_PR_ADDRESS,		oms_aa_handle,
						"PCF active",			OMSC_PR_NUMBER,			pcf_active,
						OPC_NIL);                                       
					}
				
				/* Obtain the MAC layer information for the local MAC	*/
				/* process from the model-wide registry.				*/
				/* This is to check if the node is a gateway or not.	*/
				proc_record_handle_list_ptr = op_prg_list_create ();
				
				oms_pr_process_discover (OPC_OBJID_INVALID, proc_record_handle_list_ptr, 
					"node objid",					OMSC_PR_OBJID,			 my_node_objid,
					"protocol", 					OMSC_PR_STRING, 		 "bridge",
				 	 OPC_NIL);
				
				/* If the MAC interface process registered itself,	*/
				/* then there must be a valid match					*/
				record_handle_list_size = op_prg_list_size (proc_record_handle_list_ptr);
				
				if (record_handle_list_size != 0)
					{
					wlan_flags->bridge_flag = OPC_BOOLINT_ENABLED;
					}
				
				/* If the station is not a bridge only then check for arp	*/
				if (wlan_flags->bridge_flag == OPC_BOOLINT_DISABLED)
					{
					/* Deallocate memory used for process discovery	*/
					while (op_prg_list_size (proc_record_handle_list_ptr))
						{
						op_prg_list_remove (proc_record_handle_list_ptr, OPC_LISTPOS_HEAD);
						}
					op_prg_mem_free (proc_record_handle_list_ptr);
				
					/* Obtain the MAC layer information for the local MAC	*/
					/* process from the model-wide registry.				*/
					proc_record_handle_list_ptr = op_prg_list_create ();
					
					oms_pr_process_discover (my_objid, proc_record_handle_list_ptr, 
						"node objid",			OMSC_PR_OBJID,			my_node_objid,
						"protocol", 			OMSC_PR_STRING,			"arp", 
						OPC_NIL);
				
					/* If the MAC interface process registered itself,	*/
					/* then there must be a valid match					*/
					record_handle_list_size = op_prg_list_size (proc_record_handle_list_ptr);
				
					}
				
				
				if (record_handle_list_size != 1)
					{
					/* An error should be created if there are more	*/
					/* than one WLAN-MAC process in the local node,	*/
					/* or if no match is found.						*/
					wlan_mac_error ("Either zero or several WLAN MAC interface processes found in the node.", OPC_NIL, OPC_NIL);
					}
				else
					{
					/*	Obtain a handle on the process record	*/
					process_record_handle = (OmsT_Pr_Handle) op_prg_list_access (proc_record_handle_list_ptr, OPC_LISTPOS_HEAD);
				
					/* Obtain the module objid for the Wlan MAC Interface module	*/
					oms_pr_attr_get (process_record_handle, "module objid", OMSC_PR_OBJID, &mac_if_module_objid);
				
					/* Obtain the stream numbers connected to and from the	*/
					/* Wlan MAC Interface layer process						*/
					oms_tan_neighbor_streams_find (my_objid, mac_if_module_objid, &instrm_from_mac_if, &outstrm_to_mac_if);
					}
					
				/* Deallocate memory used for process discovery	*/
				while (op_prg_list_size (proc_record_handle_list_ptr))
					{
					op_prg_list_remove (proc_record_handle_list_ptr, OPC_LISTPOS_HEAD);
					}
				
				op_prg_mem_free (proc_record_handle_list_ptr);
				
				if (wlan_trace_active)
					{
					/* Cache the state name from which this function was	*/
					/* called.												*/
					strcpy (current_state_name, "init");  
					}
				}


			/** state (INIT) transition processing **/
			FSM_TRANSIT_FORCE (8, state8_enter_exec, ;, "default", "", "INIT", "BSS_INIT")
				/*---------------------------------------------------------*/



			/** state (IDLE) enter executives **/
			FSM_STATE_ENTER_UNFORCED (1, state1_enter_exec, "IDLE", "wlan_mac_wifib () [IDLE enter execs]")
				{
				/** The purpose of this state is to wait until the packet has	**/
				/** arrived from the higher or lower layer.					 	**/
				/** In this state following intrpts can occur:				 	**/
				/** 1. Data arrival from application layer   				 	**/
				/** 2. Frame (DATA,ACK,RTS,CTS) rcvd from PHY layer			 	**/
				/** 3. Busy intrpt stating that frame is being rcvd			 	**/
				/** 4. Coll intrpt indicating that more than one frame is rcvd  **/
				/* When Data arrives from the application layer, insert it	*/
				/* in the queue.											*/
				/* If rcvr is not busy then	set Deference to DIFS 			*/
				/* and Change state to "DEFER" state						*/
				
				/* Rcvd RTS,CTS,DATA,or ACK (frame rcvd intrpt)				*/
				/* Set Backoff flag if the station needs to backoff			*/
				/* If the frame is destined for this station then send 		*/
				/* appropriate response and set deference to SIFS 			*/
				/* clear the rcvr busy flag and clamp any data transmission	*/
				/*															*/
				/* If it's a broadcast frame then set deference to NAV		*/
				/* and schedule self intrpt and change state to "DEFER".	*/
				/* Copy the frame (RTS/DATA) in retransmission variable		*/
				/* if rcvr start receiving frame (busy stat intrpt) then set*/
				/* a flag indicating rcvr is busy,if rcvr start receiving   */
				/* more than one frame (collision stat intrpt) then set the	*/
				/* rcvd frame as invalid frame set deference time to EIFS	*/
				
				if (wlan_trace_active)
					{
					/* Determine the current state name.	*/
					strcpy (current_state_name, "idle");
					}
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,wlan_mac_wifib)


			/** state (IDLE) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "IDLE", "wlan_mac_wifib () [IDLE exit execs]")
				{
				/* Interrupt processing routine.									*/
				wlan_interrupts_process ();
				
				/* Schedule deference interrupt when there is a frame to transmit	*/
				/* at the stream interrupt and the receiver is not busy				*/
				if (READY_TO_TRANSMIT)
					{
					/* If the medium was idling for a period equal or longer than	*/
					/* DIFS time then we don't need to defer.						*/
					if (MEDIUM_IS_IDLE && !IN_CFP)
						{
						/* We can start the transmission immediately.				*/
						wlan_flags->immediate_xmt = OPC_TRUE;
						backoff_slots = BACKOFF_SLOTS_UNSET;
						}
					
					/* We schedule a deference only for the DCF period. 			*/
					else if (!IN_CFP)
						{
						/* We need to defer. Schedule the end of it.				*/
						wlan_schedule_deference ();
						}
					}
				}


			/** state (IDLE) transition processing **/
			FSM_INIT_COND (READY_TO_TRANSMIT && !MEDIUM_IS_IDLE)
			FSM_TEST_COND (READY_TO_TRANSMIT && MEDIUM_IS_IDLE && cfp_ap_medium_control == OPC_BOOLINT_DISABLED)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("IDLE")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 2, state2_enter_exec, ;, "READY_TO_TRANSMIT && !MEDIUM_IS_IDLE", "", "IDLE", "DEFER")
				FSM_CASE_TRANSIT (1, 4, state4_enter_exec, ;, "READY_TO_TRANSMIT && MEDIUM_IS_IDLE && cfp_ap_medium_control == OPC_BOOLINT_DISABLED", "", "IDLE", "TRANSMIT")
				FSM_CASE_TRANSIT (2, 1, state1_enter_exec, ;, "default", "", "IDLE", "IDLE")
				}
				/*---------------------------------------------------------*/



			/** state (DEFER) enter executives **/
			FSM_STATE_ENTER_UNFORCED (2, state2_enter_exec, "DEFER", "wlan_mac_wifib () [DEFER enter execs]")
				{
				/* This state defer until the medium is available for transmission		*/
				/* interrupts that can occur in this state are:   						*/
				/* 1. Data arrival from application layer         						*/
				/* 2. Frame (DATA,ACK,RTS,CTS) rcvd from PHY layer						*/
				/* 3. Busy intrpt stating that frame is being rcvd						*/
				/* 4. Collision intrpt stating that more than one frame is rcvd    		*/
				/* 5. Deference timer has expired (self intrpt)                    		*/
				/*																		*/
				/* For Data arrival from application layer queue the packet. Set		*/
				/* Backoff flag if the station needs to backoff after deference because	*/
				/* the medium is busy. If the frame is destined for this station then	*/
				/* set frame to respond and set a deference timer to SIFS. Set			*/
				/* deference timer to SIFS and don't change states. If receiver starts	*/
				/* receiving more than one frame then flag the received frame as		*/
				/* invalid frame and set a deference to EIFS.							*/
				
				if (wlan_trace_active)
					{
					/* Determine the current state name.								*/
					strcpy (current_state_name, "defer");
					}
				
				/* If in CFP, schedule a deference interrupt if not already scheduled.	*/ 
				if ((cfp_ap_medium_control == OPC_BOOLINT_ENABLED || wlan_flags->pcf_active == OPC_BOOLINT_ENABLED) &&
					op_ev_valid (deference_evh) != OPC_TRUE)
					{
					wlan_schedule_deference ();
					}
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (5,wlan_mac_wifib)


			/** state (DEFER) exit executives **/
			FSM_STATE_EXIT_UNFORCED (2, "DEFER", "wlan_mac_wifib () [DEFER exit execs]")
				{
				/* Store the previous receiver status before processing the			*/
				/* interrupt, which may change the status information.				*/
				pre_rx_status = rcv_channel_status;
				
				/* Call the interrupt processing routine for each interrupt.		*/
				wlan_interrupts_process ();
				
				if (wlan_flags->ignore_busy == OPC_BOOLINT_DISABLED)
					{
					/* If the receiver is busy while the station is deferring		*/
					/* then clear the self interrupt. As there will be a new self	*/ 
					/* interrupt generated once the receiver becomes idle again.  	*/
					if (RECEIVER_BUSY_HIGH && (op_ev_valid (deference_evh) == OPC_TRUE))
						{
						op_ev_cancel (deference_evh);
						}
				
					/* Update the value of the temporary bad packet flag, which is	*/
					/* used in the FRAME_RCVD macro below.							*/
					bad_packet_rcvd = wlan_flags->rcvd_bad_packet;
				
					/* If the receiver became idle again schedule the end of the	*/
					/* deference.													*/
					if (RECEIVER_BUSY_LOW && pre_rx_status != 0)
						wlan_schedule_deference ();
				
					/* While we were deferring, if we receive a frame which			*/
					/* requires a response, or we had used EIFS due to a pervious 	*/
					/* error, then we need to re-schedule our end of				*/
					/* deference interrupt, since the deference time for response	*/
					/* frames is shorter. Similarly, we need to re-schedule it if	*/
					/* the received frame made us set our NAV to a higher value.	*/
					else if (FRAME_RCVD && 
							 (fresp_to_send != WlanC_None || wlan_flags->nav_updated == OPC_BOOLINT_ENABLED || 
							  ((wlan_flags->wait_eifs_dur == OPC_BOOLINT_ENABLED && IN_CFP) || wlan_flags->polled == OPC_BOOLINT_ENABLED)) && 
							 op_ev_valid (deference_evh) == OPC_TRUE)
						{
						/* Cancel the current event and schedule a new one.			*/
						op_ev_cancel (deference_evh);
						wlan_schedule_deference ();
						}
					
					/* Similarly if we received a self interrupt that indicates the	*/
					/* time to send a Beacon frame, and if we are deferring for the	*/
					/* transmission of a frame that is different then ACK frame,	*/
					/* then we need to reschedule the deference interrupt since the	*/
					/* Beacon frame will have priority over that frame, and a		*/
					/* different waiting time for deference.						*/
					else if (intrpt_type == OPC_INTRPT_SELF && intrpt_code == WlanC_Beacon_Tx_Time && fresp_to_send != WlanC_Ack &&
						     op_ev_valid (deference_evh) == OPC_TRUE)
						{
						/* Cancel the current event and schedule a new one.			*/
						op_ev_cancel (deference_evh);
						wlan_schedule_deference ();
						}
					}
				
				}


			/** state (DEFER) transition processing **/
			FSM_INIT_COND (DEFERENCE_OFF)
			FSM_TEST_COND (IDLE_AFTER_CFP)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("DEFER")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 3, state3_enter_exec, ;, "DEFERENCE_OFF", "", "DEFER", "BKOFF_NEEDED")
				FSM_CASE_TRANSIT (1, 1, state1_enter_exec, CANCEL_DEF_EVENT;;, "IDLE_AFTER_CFP", "CANCEL_DEF_EVENT;", "DEFER", "IDLE")
				FSM_CASE_TRANSIT (2, 2, state2_enter_exec, ;, "default", "", "DEFER", "DEFER")
				}
				/*---------------------------------------------------------*/



			/** state (BKOFF_NEEDED) enter executives **/
			FSM_STATE_ENTER_FORCED (3, state3_enter_exec, "BKOFF_NEEDED", "wlan_mac_wifib () [BKOFF_NEEDED enter execs]")
				{
				/** In this state we determine whether a back-off is necessary for the	**/
				/** frame we are trying to transmit. It is needed when station			**/
				/** preparing to transmit frame discovers that the medium is busy or	**/
				/** when the the station infers collision. Backoff is not needed when	**/
				/** the station is responding to the frame. Following a successful		**/
				/** packet transmission, again a back-off procedure is performed for a	**/
				/** contention window period as stated in 802.11 standard.				**/
				/**																		**/
				/** If backoff needed then check wether the station completed its 		**/
				/** backoff in the last attempt. If not then resume the backoff 		**/
				/** from the same point, otherwise generate a new random number 	 	**/
				/** for the number of backoff slots. 									**/
				
				
				// BACKOFF Modifications
				double 		max_ratio_traffic_for_me;
				double		backoff_factor = 0;
				
				//Computes the ratio of traffic I must forward
				max_ratio_traffic_for_me =  get_ratio_of_traffic(mac_all_routes, my_address);
				
				
				//And stores the max value for all nodes
				if (get_route_length(my_route) != 0)
					if ((max_ratio_traffic <= max_ratio_traffic_for_me) || (max_ratio_traffic_time < op_sim_time() - MAX_RATIO_TRAFFIC_TIMEOUT)){
						max_ratio_traffic		= max_ratio_traffic_for_me;
						max_ratio_traffic_time	= op_sim_time();
					}
				
				
				
				//And computes the according backoff factor
				switch(backoff_type){
					
					//The original backoff value of 802.11
					case ORIGINAL:
						backoff_factor = 1;
					break;
					
					//Backoff depends on the quantity of traffic we must forward
					case TRAFFIC_ADAPTIVE :
						backoff_factor = min ( MAX_BACKOFF_FACTOR , max_ratio_traffic / max_ratio_traffic_for_me );
					break;
					
					//Backoff is greated for non border nodes (not along the x and y-axis
					case BORDER_ADAPTIVE :
						if (!is_mac_border_node)
							backoff_factor = 2;
						else
							backoff_factor = 1;
						backoff_factor = min ( MAX_BACKOFF_FACTOR , max_ratio_traffic / max_ratio_traffic_for_me );
					break;
				
					//The backoff depends on the distance to the sink (bad strategy, which is logical)
					case DISTANCE_SINK_ADAPTIVE :
						backoff_factor =  min ( MAX_BACKOFF_FACTOR , get_route_length(my_route));
						//backoff_factor =  min (get_route_length(my_route) / get_average_route_length(mac_all_routes) , ;
					break;
				}
				
				
				
				//Debug
				if (my_address == 917291){
					printf("ratio %f (%d) / max %f\n", max_ratio_traffic_for_me , max_ratio_traffic < max_ratio_traffic_for_me,  max_ratio_traffic);
					printf("backoff %f (%d)\n", backoff_factor , my_address);
					//mac_print_route(my_route);
				}
				
				/* Checking whether backoff is needed or not.							*/ 
				if (wlan_flags->backoff_flag == OPC_BOOLINT_ENABLED || wlan_flags->perform_cw == OPC_BOOLINT_ENABLED)
					{
					if (backoff_slots == BACKOFF_SLOTS_UNSET)
						{                                                             
					/*	// Compute backoff interval using binary exponential process.	
						// After a successful transmission we always use cw_min.		
						if (retry_count == 0 || wlan_flags->perform_cw == OPC_BOOLINT_ENABLED)
							{			 
							// If retry count is set to 0 then set the maximum backoff	
							// slots to min window size.									
							max_backoff = cw_min;
							}
						else
							{
							// We are restransmitting. Increase the back-off window		
							// size.													
							max_backoff = max_backoff * 2 + 1; 				
							}
				
						// The number of possible slots grows exponentially until it	
						// exceeds a fixed limit.										
						if (max_backoff > cw_max) 
							{
							max_backoff = cw_max;
							}
					*/
						//backof modif according to the route length
						//printf("avant %d / apres %d\n", max_backoff , (int) ((double)max_backoff * backoff_factor));
				 
						/* Obtain a uniformly distributed random integer between 0 and	*/
						/* the minimum contention window size. Scale the number of		*/
						/* slots according to the number of retransmissions.			*/
						//backoff_slots = floor (op_dist_uniform ( (int) ((double)max_backoff * backoff_factor) + 1));
				
						backoff_slots = floor (op_dist_uniform ( max_backoff * backoff_factor + 1));
						}
				
					
					/* Set a timer for the end of the backoff interval.					*/
					intrpt_time = (current_time + backoff_slots * slot_time);
					
					/* Scheduling self interrupt for backoff.							*/
					if (wlan_flags->perform_cw == OPC_BOOLINT_ENABLED)
						backoff_elapsed_evh = op_intrpt_schedule_self (intrpt_time, WlanC_CW_Elapsed);
					else
						backoff_elapsed_evh = op_intrpt_schedule_self (intrpt_time, WlanC_Backoff_Elapsed);
					
					/* Reporting number of backoff slots as a statistic.				*/
					op_stat_write (backoff_slots_handle, backoff_slots);
					} 
				}


			/** state (BKOFF_NEEDED) exit executives **/
			FSM_STATE_EXIT_FORCED (3, "BKOFF_NEEDED", "wlan_mac_wifib () [BKOFF_NEEDED exit execs]")
				{
				}


			/** state (BKOFF_NEEDED) transition processing **/
			FSM_INIT_COND (TRANSMIT_FRAME)
			FSM_TEST_COND (PERFORM_BACKOFF)
			FSM_TEST_LOGIC ("BKOFF_NEEDED")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 4, state4_enter_exec, ;, "TRANSMIT_FRAME", "", "BKOFF_NEEDED", "TRANSMIT")
				FSM_CASE_TRANSIT (1, 5, state5_enter_exec, ;, "PERFORM_BACKOFF", "", "BKOFF_NEEDED", "BACKOFF")
				}
				/*---------------------------------------------------------*/



			/** state (TRANSMIT) enter executives **/
			FSM_STATE_ENTER_UNFORCED (4, state4_enter_exec, "TRANSMIT", "wlan_mac_wifib () [TRANSMIT enter execs]")
				{
				/** In this state following intrpts can occur:     				**/
				/** 1. Data arrival from application layer.			        	**/
				/** 2. Frame (DATA,ACK,RTS,CTS) rcvd from PHY layer.			**/
				/** 3. Busy intrpt stating that frame is being rcvd.			**/
				/** 4. Collision intrpt means more than one frame is rcvd.  	**/
				/** 5. Transmission completed intrpt from physical layer		**/
				/** Queue the packe for Data Arrival from the higher layer,		**/
				/** and do not change state.									**/
				/** After Transmission is completed change state to FRM_END		**/
				/** No response is generated for any lower layer packet arrival	**/
				
				/* Prepare transmission frame by setting appropriate	*/
				/* fields in the control/data frame.					*/ 
				/* Skip this routine if any frame is received from the	*/
				/* higher or lower layer(s)							  	*/
				if (wlan_flags->immediate_xmt == OPC_TRUE)
					{
					/* Initialize the contention window size for the 	*/
					/* packets that are sent without backoff for the 	*/
					/* first time, if in case they are retransmitted.	*/
					max_backoff = cw_min;
					
					/* Start the transmission.							*/
					wlan_frame_transmit ();
					
					/* Reset the immediate transmission flag.			*/
					wlan_flags->immediate_xmt = OPC_FALSE;
					}
				
				else if (wlan_flags->rcvd_bad_packet == OPC_BOOLINT_DISABLED &&
				    intrpt_type == OPC_INTRPT_SELF)  
					{
					/* If it is a PCF enabled MAC then make sure that	*/
					/* the interrupt was not PCF related.				*/
					if (pcf_flag == OPC_BOOLINT_DISABLED || intrpt_code == WlanC_Deference_Off || 
						intrpt_code == WlanC_Backoff_Elapsed || intrpt_code == WlanC_CW_Elapsed)
						{
						wlan_frame_transmit ();
						
						/* Reset the forced transmission flag which may	*/
						/* have been set.								*/
						wlan_flags->forced_xmt = OPC_BOOLINT_DISABLED;
						}
					}
				
				if (wlan_trace_active)
					{
					/* Determine the current state name.				*/
					strcpy (current_state_name, "transmit");
					}
				
				
				
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (9,wlan_mac_wifib)


			/** state (TRANSMIT) exit executives **/
			FSM_STATE_EXIT_UNFORCED (4, "TRANSMIT", "wlan_mac_wifib () [TRANSMIT exit execs]")
				{
				/* If the packet is received while the the station is      */ 
				/* transmitting then mark the received packet as bad.	   */
				if (op_intrpt_type () == OPC_INTRPT_STAT)
					{
					intrpt_code = (WlanT_Mac_Intrpt_Code)op_intrpt_stat ();
					if (intrpt_code < TRANSMITTER_BUSY_INSTAT && op_stat_local_read (intrpt_code) > rx_power_threshold && rcv_channel_status == 0)
						{	
						wlan_flags->rcvd_bad_packet = OPC_BOOLINT_ENABLED;
						}
					
					/* If we completed the transmission then reset the		*/
					/* transmitter flag.									*/
					else if (intrpt_code == TRANSMITTER_BUSY_INSTAT)
						{
						wlan_flags->transmitter_busy = OPC_BOOLINT_DISABLED;
						
						/* Also reset the receiver idle time, since with	*/
						/* the end of our transmission, we expect that the	*/
						/* medium became idle again (but make sure we are	*/
						/* also not receiving a packet).					*/
						if (rcv_channel_status == 0)
							{
							rcv_idle_time = op_sim_time ();
							}
						}
					}
				
				else if ((op_intrpt_type () == OPC_INTRPT_STRM) && (op_intrpt_strm () != instrm_from_mac_if))
					{
					/* While transmitting, we received a packet from		*/
					/* physical layer. Mark the packet as bad.				*/
					wlan_flags->rcvd_bad_packet = OPC_BOOLINT_ENABLED;
					}
				
				/* Call the interrupt processing routine for each interrupt.*/
				wlan_interrupts_process ();
				}


			/** state (TRANSMIT) transition processing **/
			FSM_INIT_COND (TRANSMISSION_COMPLETE)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("TRANSMIT")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 6, state6_enter_exec, ;, "TRANSMISSION_COMPLETE", "", "TRANSMIT", "FRM_END")
				FSM_CASE_TRANSIT (1, 4, state4_enter_exec, ;, "default", "", "TRANSMIT", "TRANSMIT")
				}
				/*---------------------------------------------------------*/



			/** state (BACKOFF) enter executives **/
			FSM_STATE_ENTER_UNFORCED (5, state5_enter_exec, "BACKOFF", "wlan_mac_wifib () [BACKOFF enter execs]")
				{
				/** Processing Random Backoff								**/
				/** In this state following intrpts can occur: 				**/
				/** 1. Data arrival from application layer   				**/
				/** 2. Frame (DATA,ACK,RTS,CTS) rcvd from PHY layer			**/
				/** 3. Busy intrpt stating that frame is being rcvd			**/
				/** 4. Coll intrpt stating that more than one frame is rcvd	**/  
				/** Queue the packet for Data Arrival from application 		**/
				/** layer and do not change the state.						**/ 
				/** If the frame is destined for this station then prepare  **/
				/** appropriate frame to respond and set deference to SIFS	**/
				/** Update NAV value (if needed) and reschedule deference	**/
				/** Change state to "DEFER"									**/
				/** If it's a broadcast frame then check wether NAV needs 	**/
				/** to be updated. Schedule self interrupt and change		**/
				/** state to Deference										**/
				/** If rcvr start receiving frame (busy stat intrpt) then 	**/
				/** set a flag indicating rcvr is busy. 					**/ 
				/** if rcvr start receiving more than one frame then flag 	**/ 
				/** the rcvd frame as invalid and set deference				**/
				/** timer to EIFS   										**/ 
				/* Change State to DEFER									**/
				
				if (wlan_trace_active)
					{
					/* Determine the current state name.	*/
					strcpy (current_state_name, "backoff");
					}
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (11,wlan_mac_wifib)


			/** state (BACKOFF) exit executives **/
			FSM_STATE_EXIT_UNFORCED (5, "BACKOFF", "wlan_mac_wifib () [BACKOFF exit execs]")
				{
				/* Call the interrupt processing routine for each interrupt.		*/
				wlan_interrupts_process ();
				
				/* Set the number of slots to zero, once the backoff is completed.	*/
				if (BACKOFF_COMPLETED)
					{
					backoff_slots = BACKOFF_SLOTS_UNSET;
					}
				else if (CW_COMPLETED)
					{
					backoff_slots = BACKOFF_SLOTS_UNSET;
					
					/* Reset the contention window flags to enable future			*/
					/* transmissions.												*/
					wlan_flags->cw_required = OPC_BOOLINT_DISABLED;
					wlan_flags->perform_cw  = OPC_BOOLINT_DISABLED;
					
					/* Reset the forced transmission flag, which may have been set,	*/
					/* if we don't have anything to transmit.						*/
					if (wlan_flags->data_frame_to_send == OPC_BOOLINT_DISABLED)
						wlan_flags->forced_xmt = OPC_BOOLINT_DISABLED;
					}
				
				/* Pause the backoff procedure if our receiver just became busy or	*/
				/* if we received a self interrupt indicating the time to send a	*/
				/* Beacon frame for a new contention free period.					*/
				if (RECEIVER_BUSY_HIGH || (rcv_channel_status == 0 && intrpt_type == OPC_INTRPT_SELF && intrpt_code == WlanC_Beacon_Tx_Time)) 
					{
					/* Computing remaining backoff slots for next iteration.		*/
					backoff_slots =  ceil ((intrpt_time - current_time - PRECISION_RECOVERY) / slot_time);
					
					/* Don't cancel the end-of-backoff interrupt if we have already	*/
					/* completed all the slots of the back-off.						*/
					if (op_ev_valid (backoff_elapsed_evh) == OPC_TRUE && (backoff_slots > 0.0 || !RECEIVER_BUSY_HIGH))
						{
						/* Clear the self interrupt as station needs to defer.		*/
						op_ev_cancel (backoff_elapsed_evh);
						
						/* Disable perform cw flag because the station will not		*/
						/* backoff using contention window.							*/
						wlan_flags->perform_cw  = OPC_BOOLINT_DISABLED;
						}
				
					/* If the remaining backoff slots were computed as "0" slot		*/
					/* then we are experiencing a special case: we started 			*/
					/* receiving a transmission at the exact same time when we will	*/
					/* complete our backoff and start our own transmission. In such	*/
					/* cases we ignore the reception and continue with our planned	*/
					/* transmission for the accuracy of the simulation model,		*/
					/* because these two events are happening at the exact same		*/
					/* time and	their execution order should not change the overall	*/
					/* behavior of the MAC.											*/
					if (backoff_slots == 0.0)
						wlan_flags->forced_xmt = OPC_BOOLINT_ENABLED;
					}
				}


			/** state (BACKOFF) transition processing **/
			FSM_INIT_COND (PERFORM_TRANSMIT)
			FSM_TEST_COND (BACK_TO_DEFER)
			FSM_TEST_COND (BACK_TO_IDLE)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("BACKOFF")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 4, state4_enter_exec, ;, "PERFORM_TRANSMIT", "", "BACKOFF", "TRANSMIT")
				FSM_CASE_TRANSIT (1, 2, state2_enter_exec, wlan_schedule_deference ();;, "BACK_TO_DEFER", "wlan_schedule_deference ();", "BACKOFF", "DEFER")
				FSM_CASE_TRANSIT (2, 1, state1_enter_exec, ;, "BACK_TO_IDLE", "", "BACKOFF", "IDLE")
				FSM_CASE_TRANSIT (3, 5, state5_enter_exec, ;, "default", "", "BACKOFF", "BACKOFF")
				}
				/*---------------------------------------------------------*/



			/** state (FRM_END) enter executives **/
			FSM_STATE_ENTER_FORCED (6, state6_enter_exec, "FRM_END", "wlan_mac_wifib () [FRM_END enter execs]")
				{
				/** The purpose of this state is to determine the next unforced	**/
				/** state after completing transmission.					    **/
				 
				/** 3 cases											     		**/
				/** 1.If just transmitted RTS or DATA frame then wait for 	 	**/
				/** response with expected_frame_type variable set and change   **/
				/** the states to Wait for Response otherwise just DEFER for 	**/
				/** next transmission											**/
				/** 2.If expected frame is rcvd then check to see what is the	**/
				/** next frame to transmit and set appropriate deference timer	**/
				/** 2a.If all the data fragments are transmitted then check		**/
				/** wether the queue is empty or not                            **/
				/** If not then based on threshold fragment the packet 	 		**/
				/** and based on threshold decide wether to send RTS or not   	**/
				/** If there is a data to be transmitted then wait for DIFS		**/
				/**	duration before contending for the channel					**/
				/** If nothing to transmit then go to IDLE state          		**/
				/** and wait for the packet arrival from higher or lower layer	**/
				/** 3.If expected frame is not rcvd then infer collision, 		**/
				/** set backoff flag, if retry limit is not reached       		**/
				/** retransmit the frame by contending for the channel  	 	**/
				
				/* If there is no frame expected then check to see if there		*/
				/* is any other frame to transmit.								*/
				
				if (expected_frame_type == WlanC_None) 
					{
					/* If the frame needs to be retransmitted or there is   	*/
					/* something in the fragmentation buffer to transmit or the	*/
					/* station needs to respond to a frame then schedule		*/
					/* deference.												*/
					if (FRAME_TO_TRANSMIT)
						{
						/* Schedule deference before frame transmission.		*/
						wlan_schedule_deference ();
						}
					}
				else
					{
					/* The station needs to wait for the expected frame type	*/
					/* So it will set the frame timeout interrupt which will be	*/
				    /* exectued if no frame is received in the set duration.   	*/
					timer_duration =  sifs_time + plcp_overhead_control + WLAN_ACK_DURATION;
					frame_timeout_evh = op_intrpt_schedule_self (current_time + timer_duration, WlanC_Frame_Timeout);
					}
				}


			/** state (FRM_END) exit executives **/
			FSM_STATE_EXIT_FORCED (6, "FRM_END", "wlan_mac_wifib () [FRM_END exit execs]")
				{
				
				}


			/** state (FRM_END) transition processing **/
			FSM_INIT_COND (FRM_END_TO_IDLE)
			FSM_TEST_COND (WAIT_FOR_FRAME)
			FSM_TEST_COND (FRM_END_TO_DEFER)
			FSM_TEST_LOGIC ("FRM_END")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 1, state1_enter_exec, ;, "FRM_END_TO_IDLE", "", "FRM_END", "IDLE")
				FSM_CASE_TRANSIT (1, 7, state7_enter_exec, ;, "WAIT_FOR_FRAME", "", "FRM_END", "WAIT_FOR_RESPONSE")
				FSM_CASE_TRANSIT (2, 2, state2_enter_exec, ;, "FRM_END_TO_DEFER", "", "FRM_END", "DEFER")
				}
				/*---------------------------------------------------------*/



			/** state (WAIT_FOR_RESPONSE) enter executives **/
			FSM_STATE_ENTER_UNFORCED (7, state7_enter_exec, "WAIT_FOR_RESPONSE", "wlan_mac_wifib () [WAIT_FOR_RESPONSE enter execs]")
				{
				/** The purpose of this state is to wait for the response after	**/
				/** transmission. The only frames which require acknowlegements	**/
				/**  are RTS and DATA frame. 									**/
				/** In this state following intrpts can occur:				   	**/
				/** 1. Data arrival from application layer   					**/
				/** 2. Frame (DATA,ACK,RTS,CTS) rcvd from PHY layer				**/
				/** 3. Frame timeout if expected frame is not rcvd 				**/
				/** 4. Busy intrpt stating that frame is being rcvd           	**/
				/** 5. Collision intrpt stating that more than one frame is rcvd**/		
				/** Queue the packet as Data Arrives from application layer		**/
				/** If Rcvd unexpected frame then collision is inferred and		**/
				/** retry count is incremented							    	**/
				/** if a collision stat interrupt from the rcvr then flag the   **/
				/** received frame as bad 										**/
				
				if (wlan_trace_active)
					{
					/* Determine the current state name.	*/
					strcpy (current_state_name, "wait_for_response");
					}
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (15,wlan_mac_wifib)


			/** state (WAIT_FOR_RESPONSE) exit executives **/
			FSM_STATE_EXIT_UNFORCED (7, "WAIT_FOR_RESPONSE", "wlan_mac_wifib () [WAIT_FOR_RESPONSE exit execs]")
				{
				/* First restore the value of the bad packet received flag		*/
				/* since we will use it also in the state transition decision.	*/
				bad_packet_rcvd = wlan_flags->rcvd_bad_packet;
				
				/* Determine the interrupt type and the stream index in the		*/
				/* case of stream interrupt, since this information will be		*/
				/* in the next if statement condition.							*/
				intrpt_type = op_intrpt_type ();
				if (intrpt_type == OPC_INTRPT_STRM)
					i_strm = op_intrpt_strm ();
				
				/* Clear the frame timeout interrupt once the receiver is busy	*/
				/* or the frame is received (in case of collisions, the			*/
				/* frames whose reception has started while we were				*/
				/* transmitting are excluded in the FRAME_RCVD macro).			*/
				if (((intrpt_type == OPC_INTRPT_STAT && op_intrpt_stat () < TRANSMITTER_BUSY_INSTAT && 
					  op_stat_local_read (op_intrpt_stat ()) > rx_power_threshold && rcv_channel_status == 0) ||
					 FRAME_RCVD) &&
					(op_ev_valid (frame_timeout_evh) == OPC_TRUE))
					{
					op_ev_cancel (frame_timeout_evh);
					}
				
				/* Call the interrupt processing routine for each interrupt		*/
				/* request.														*/
				wlan_interrupts_process ();
				
				/* If expected frame is not received in the set duration or the	*/
				/* there is a collision at the receiver then set the expected	*/
				/* frame type to be none because the station needs to			*/
				/* retransmit the frame.									 	*/
				if (FRAME_TIMEOUT)
					{
					/* Setting expected frame type to none frame.				*/
					expected_frame_type = WlanC_None;
					
					/* Set the flag that indicates collision.					*/
					wlan_flags->wait_eifs_dur = OPC_BOOLINT_ENABLED;
					
					/* If PCF is active, increment poll fail and the retry		*/
					/* counter.													*/
					if ((ap_flag == OPC_BOOLINT_ENABLED) &&
						(wlan_flags->pcf_active == OPC_BOOLINT_ENABLED))
						{
						/* If last frame a data frame, Increment retry counter.	*/
						if ((last_frametx_type == WlanC_Data_Poll)	||	(last_frametx_type == WlanC_Data_A_P))
							{
							pcf_retry_count = pcf_retry_count++;
							}
						
						if  (wlan_flags->active_poll == OPC_BOOLINT_ENABLED) 
							{
							poll_fail_count++;
							wlan_flags->active_poll = OPC_BOOLINT_DISABLED;
							}
						
						/* Check whether further retries are possible or the	*/
						/* data frame needs to be discarded.					*/
						wlan_pcf_frame_discard();
						}
					else
						{
					
						/* Retransmission counter will be incremented.			*/
						retry_count = retry_count + 1;
				
						/* Reset the NAV duration so that the retransmission is	*/
						/* not unnecessarily delayed.							*/
						nav_duration = current_time;
				
						/* Check whether further retries are possible or the	*/
						/* data frame needs to be discarded.					*/
						wlan_frame_discard ();
						}
					}
				
				}


			/** state (WAIT_FOR_RESPONSE) transition processing **/
			FSM_INIT_COND (FRAME_TIMEOUT || FRAME_RCVD)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("WAIT_FOR_RESPONSE")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 6, state6_enter_exec, ;, "FRAME_TIMEOUT || FRAME_RCVD", "", "WAIT_FOR_RESPONSE", "FRM_END")
				FSM_CASE_TRANSIT (1, 7, state7_enter_exec, ;, "default", "", "WAIT_FOR_RESPONSE", "WAIT_FOR_RESPONSE")
				}
				/*---------------------------------------------------------*/



			/** state (BSS_INIT) enter executives **/
			FSM_STATE_ENTER_UNFORCED (8, state8_enter_exec, "BSS_INIT", "wlan_mac_wifib () [BSS_INIT enter execs]")
				{
				/* Schedule a self interrupt to wait for mac interface 	*/
				/* to move to next state after registering				*/
				op_intrpt_schedule_self (op_sim_time (), 0);
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (17,wlan_mac_wifib)


			/** state (BSS_INIT) exit executives **/
			FSM_STATE_EXIT_UNFORCED (8, "BSS_INIT", "wlan_mac_wifib () [BSS_INIT exit execs]")
				{
				int		mac_layer_intf_id;
				List	**list_ptr_tmp;
				int		*int_ptr;
				
				/* Obtain the values assigned to the various attributes				*/
				op_ima_obj_attr_get (my_objid, "Wireless LAN Parameters", &wlan_params_comp_attr_objid);
				params_attr_objid = op_topo_child (wlan_params_comp_attr_objid, OPC_OBJTYPE_GENERIC, 0);
				
				/* Determining the final MAC address after address resolution.		*/
				op_ima_obj_attr_get (my_objid, "Address", &my_address);
				
				/* Update our own process registery record with the final address	*/
				/* information.														*/
				oms_pr_attr_set (own_process_record_handle, "Address", OMSC_PR_NUMBER, (double) my_address, OPC_NIL);                                       
				
				/* Once the station addresses are resolved, then create a pool for wlan addresses.	*/
				oms_aa_address_resolve (oms_aa_wlan_handle, my_objid, &my_address);
				
				/* Obtain the MAC layer information for the local MAC				*/
				/* process from the model-wide registry.							*/
				proc_record_handle_list_ptr = op_prg_list_create ();
				
				/* If the subnet is BSS based then do the process discovery using	*/
				/* BSS ID (domain_id), otherwise do it using subnet ID.				*/
				if (bss_id_type == WlanC_Bss_Divided_Subnet)
					{
				    oms_pr_process_discover (OPC_OBJID_INVALID, proc_record_handle_list_ptr, 
					  "domain_id",			OMSC_PR_NUMBER,  		 (double) bss_id,
					  "mac_type",			OMSC_PR_STRING,	 		 "wireless_lan",
					  "protocol",			OMSC_PR_STRING,			 "mac",
					   OPC_NIL);
				    }
				else
					{
					oms_pr_process_discover (OPC_OBJID_INVALID, proc_record_handle_list_ptr, 
					  "subnetid",			OMSC_PR_OBJID,			 my_subnet_objid,
					  "mac_type",			OMSC_PR_STRING,			"wireless_lan",
					  "protocol",			OMSC_PR_STRING,			"mac",
				 	   OPC_NIL);
					}
				
				/* If the MAC interface process registered itself,	*/
				/* then there must be a valid match					*/
				record_handle_list_size = op_prg_list_size (proc_record_handle_list_ptr);
				
				/* Allocating memory for the duplicate buffer based on number of stations in the subnet.	*/
				duplicate_list_ptr = (WlanT_Mac_Duplicate_Buffer_Entry**) 
									 op_prg_mem_alloc (record_handle_list_size * sizeof (WlanT_Mac_Duplicate_Buffer_Entry*));
				
				/* Initializing duplicate buffer entries.	*/
				for (i_cnt = 0; i_cnt <= (record_handle_list_size - 1); i_cnt++)
					{
					duplicate_list_ptr [i_cnt] = OPC_NIL;
					}
				
				/* Initialize the address list index to zero.	*/
				addr_index = 0; 
				
				/* Variable to counting number of access point in the network.	*/
				ap_count = 0;
				
				/* Maintain a list of stations in the BSS if it is an AP and a bridge	*/
				if (ap_flag == OPC_BOOLINT_ENABLED && wlan_flags->bridge_flag == OPC_BOOLINT_ENABLED)
					{
					bss_stn_list = (int *) op_prg_mem_alloc ((record_handle_list_size - 1) * sizeof (int));
					count = 0;
					
					/* Number of stations in the BSS	*/
					bss_stn_count = record_handle_list_size - 1;	  
					}
				
				/* Initialize the variable which holds the count of PCF users in network */
				poll_list_size =0;
				
				/* Keeps track of the number of stations which are PCF enabled 	*/
				/* If this value is greater than 0, only then a Beacon frame is */
				/* transmitted at the targated time.							*/
				pcf_enabled_stations = 0;
				
				/* Preserves the PCF status on an AP node */
				pcf_enabled_on_AP = OPC_FALSE;
				
				/* Traversing the process record handle list to determine if there is any access point in the subnet.	*/
				for (i_cnt = 0; i_cnt < record_handle_list_size; i_cnt++ )
					{
					/*	Obtain a handle on the process record	*/
					process_record_handle = (OmsT_Pr_Handle) op_prg_list_access (proc_record_handle_list_ptr, i_cnt);
				
					/* Get the Station type.	*/
					oms_pr_attr_get (process_record_handle, "subprotocol", OMSC_PR_NUMBER, &statype);
				
					/* If the station is an Access Point then its station id will be a BSS id for all the station in that subnet.	*/
					if (statype == (double) WLAN_AP)
						{
						/* If access point found then it means that it is a Infrastructured BSS.	*/
						bss_flag = OPC_BOOLINT_ENABLED;
					
						/* Obtain the station address of the access point.							*/
						oms_pr_attr_get (process_record_handle, "address",	OMSC_PR_NUMBER, &sta_addr);	 
						ap_mac_address  = (int) sta_addr;
				
						/* According to IEEE 802.11 there cannot be more than one Access point in	*/
						/* the same subnet.															*/
				   		ap_count = ap_count + 1;
						if (ap_count == 2)
							{
							sprintf(msg1,"More than one Access Point found within the same BSS (BSS ID = %d)" , bss_id);
							wlan_mac_error (msg1,"or in the same OPNET subnet.","Check the configuration.");
							}		 
						}
					    
					/* If the station is a bridge and an access point then	*/
					/* maintain a list of stations in the BSS				*/
					if (ap_flag == OPC_BOOLINT_ENABLED && wlan_flags->bridge_flag == OPC_BOOLINT_ENABLED)
						{
						/* Get the station id	*/
						oms_pr_attr_get (process_record_handle, "address",	OMSC_PR_NUMBER, &sta_addr);
				
						/* Maintain a list of stations in the BSS not including itself	*/
						if ((int) sta_addr != my_address)
							{
							bss_stn_list [count] = (int) sta_addr;
							count = count + 1;
							}
						}
					
					/* Checking the physical characteristic configuration for the subnet.	*/
					oms_pr_attr_get (process_record_handle, "module objid", OMSC_PR_OBJID, &mac_objid);
				
					/* Obtain the values assigned to the various attributes	*/
					op_ima_obj_attr_get (mac_objid, "Wireless LAN Parameters", &wlan_params_comp_attr_objid);
					params_attr_objid = op_topo_child (wlan_params_comp_attr_objid, OPC_OBJTYPE_GENERIC, 0);
				
					/* Load the appropriate physical layer characteristics.	*/	
					op_ima_obj_attr_get (params_attr_objid, "Physical Characteristics", &sta_phy_char_flag);
				
					if (sta_phy_char_flag != phy_char_flag)
						{
						wlan_mac_error ("Physical Characteristic configuration mismatch in the subnet.",
										"All stations in the subnet should have same physical characteristics", 
										"Check the configuration");			
						}	
					
					/*Check if the PCF mode has been enabled in this station	*/
					oms_pr_attr_get (process_record_handle, "PCF active", OMSC_PR_NUMBER, &pcf_active);
					
					/* If this station has its PCF functionality enabled,   */
					/* increment the PCF user count							*/
					if (pcf_active != 0.0) 
						{
						poll_list_size++;
				
						/* Preserve the PCF status if it is an AP. It is required later, when    */
						/* we check for PCF functionality enabled on other nodes in the network. */
						if (statype == (double) WLAN_AP)
							{
							pcf_enabled_on_AP = OPC_TRUE;
							}
						}
				    }
				
				
				/* Record the count of the PCF enabled stations.	*/
				/* The beacon interval will be transmitted, only	*/
				/* if there has been a PCF enabled station			*/
				pcf_enabled_stations = poll_list_size;
				
				/* If PCF functionality has been enabled on any of the nodes     */
				/* if yes, it is required to have an Access Point in the network */
				if (pcf_enabled_stations > 0) 
					{
					/* Indcates the type of network (DCF only or PCF and DCF enabled nodes 		 */
					/* If pcf_network = 1, network contains either only PCF nodes or combination */
					/* If pcf_network = 0, network contains only DCF enabled nodes 				 */
					pcf_network = 1;
				
					/* The network has PCF enabled nodes, but no AP. */
					if (ap_count == 0)
						{
						sprintf (msg1,"PCF functionality has been enabled on %d station(s)", pcf_enabled_stations);
						wlan_mac_error (msg1,"An Access Point is required in the network to support PCF functionality.", 
										"Check your network configuration");
						}
					/* PCF enabled nodes present in the network, but */
					/* AP does not support PCF. Raise an error.		 */
					else if (pcf_enabled_on_AP == OPC_FALSE)
						{
						sprintf (msg1,"PCF functionality has been enabled on %d station(s)", pcf_enabled_stations);
						wlan_mac_error (msg1,"The node configured as Access Point does not support PCF functionality.", 
										"Check your network configuration");
						}
					}
				
				
				/* Create polling list, only if the station is an Access Point	*/
				/* and has then PCF functionality enabled  					    */
				if ((ap_flag == OPC_BOOLINT_ENABLED) && (pcf_flag == OPC_BOOLINT_ENABLED))
					{
					/* Since the current station is a AP, exclude this	*/
					/* station from the polling list					*/		
					poll_list_size--;
				
					/* Need not allocate memory for the polling list if no nodes in */
					/* the network support PCF. Also it is not neccesary to create  */
					/* the polling list for the same case.							*/
					if (poll_list_size > 0)
						{
						/* Allocate memory for the polling list based on the number of PCF users.	*/
						polling_list = (int *) op_prg_mem_alloc (poll_list_size * sizeof(int));
				
						/* Initialize polling list entries.					*/
						j_cnt = 0;
					
						/* Loop through all the stations in the current subnet	*/
						/* which were shorlisted in the discovery process 		*/
						for (i_cnt = 0; i_cnt < record_handle_list_size; i_cnt++ )
							{
							/*	Obtain a handle to the ith station from the list of processes */
							process_record_handle = (OmsT_Pr_Handle) op_prg_list_access (proc_record_handle_list_ptr, i_cnt);
						
							/* Obtain the PCF functionality status	*/
							oms_pr_attr_get (process_record_handle, "PCF active", OMSC_PR_NUMBER, &pcf_active);
						
							/* Check if PCF functionality has been enabled on this station */
							if (pcf_active != 0.0) 
								{
								/* Obtain the address of the station */
								oms_pr_attr_get (process_record_handle, "address",	OMSC_PR_NUMBER, &sta_addr);	 
							
								/* Check if the address of station selected from the process	*/
								/* registry is not that of the current station					*/
								if (sta_addr != (double) my_address)
									{
									/* Store the selected station address in the polling list	*/
									polling_list[j_cnt] = (int) sta_addr;
								
									/*Increment the polling list index							*/
									j_cnt++;
								
									}
								}
							}
				
						/* Sorting has been implemented below to sort and store the	*/
						/* address of	stations in the ascending order 			*/
					
						/* Sorting needs to be done only if there are more than one	*/
						/* entry in the polling list								*/
						if (poll_list_size > 1)
							{
							/* Loop through all the elements in the polling list			*/
							for (i_cnt = 0; i_cnt < poll_list_size; i_cnt++ )
								{
								/* Store the index of the ith element 						*/
								k_cnt = i_cnt;
							
								/* Loop through all the elements from starting from i+1		*/
								for	(j_cnt = (i_cnt + 1); j_cnt < poll_list_size; j_cnt++ )
									{
									if (polling_list[j_cnt] < polling_list[k_cnt]) 
										k_cnt = j_cnt;
									}
								address= polling_list[i_cnt];
								polling_list[i_cnt] = polling_list[k_cnt];
								polling_list[k_cnt] = address;
								}
							}
						}
					}
				else 
					poll_list_size = 0;
				
				
				/* Printing out information to ODB.	*/
				if (wlan_trace_active == OPC_TRUE)
					{
					sprintf	(msg1, "%d stations have been polled by the AP", poll_list_size);	
					op_prg_odb_print_major (msg1, OPC_NIL);
					}
				
				
				/* Deallocate memory used for process discovery	*/
				while (op_prg_list_size (proc_record_handle_list_ptr))
					{
					op_prg_list_remove (proc_record_handle_list_ptr, OPC_LISTPOS_HEAD);
					}
				op_prg_mem_free (proc_record_handle_list_ptr);
				
				/* Obtain the MAC layer information for the local MAC	*/
				/* process from the model-wide registry.				*/
				/* This is to check if the node is a gateway or not.	*/
				proc_record_handle_list_ptr = op_prg_list_create ();
				
				oms_pr_process_discover (OPC_OBJID_INVALID, proc_record_handle_list_ptr, 
					"node objid",					OMSC_PR_OBJID,			 my_node_objid,
					"gateway node",					OMSC_PR_STRING,			"gateway",
				 	 OPC_NIL);
				
				/* If the MAC interface process registered itself,	*/
				/* then there must be a valid match					*/
				record_handle_list_size = op_prg_list_size (proc_record_handle_list_ptr);
				
				if (record_handle_list_size != 0)
					{
					wlan_flags->gateway_flag = OPC_BOOLINT_ENABLED;
					}
				
				/* Deallocate memory used for process discovery	*/
				while (op_prg_list_size (proc_record_handle_list_ptr))
					{
					op_prg_list_remove (proc_record_handle_list_ptr, OPC_LISTPOS_HEAD);
					}
				op_prg_mem_free (proc_record_handle_list_ptr);
				
				/* Transmit beacon only if there has been atleast one PCF enabled station */
				/* This check is made to prevent unecassary transmission of the bacon	  */
				/* frames, when stations are operating only in the DCF mode.			  */
				if ((pcf_enabled_stations > 0) && (ap_flag == OPC_BOOLINT_ENABLED))
					{
					/* Schedule a self interrupt to kick off the Beacon timer */
					/* All terminals need this to set NAV for PCF and other   */
					/* house keeping functions.                               */
					beacon_evh = op_intrpt_schedule_self (beacon_int , WlanC_Beacon_Tx_Time);	
					}
				
				
				
				
				
				//----------------------------------------------
				//					ROUTES
				//----------------------------------------------
				
				//Get my route to the sink
				mac_layer_intf_id = op_id_from_name( op_topo_parent(op_id_self()) , OPC_OBJTYPE_PROC , "wlan_mac_intf");
				if (mac_layer_intf_id == OPC_OBJID_INVALID)
					op_sim_end("I can not get the id", "of the mac_layer_interface process" , "" , "");
				
				list_ptr_tmp = op_ima_obj_svar_get(mac_layer_intf_id , "my_route_to_sink");
				my_route = *list_ptr_tmp;
				op_prg_list_insert(mac_all_routes , my_route , OPC_LISTPOS_TAIL);
				
				
				int_ptr = op_ima_obj_svar_get(mac_layer_intf_id , "is_border_node");
				is_mac_border_node = *int_ptr;
				
				//mac_print_route(my_route);
				
				
				}


			/** state (BSS_INIT) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "BSS_INIT", "IDLE")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (0,wlan_mac_wifib)
		}
	}

#if defined (__cplusplus)
	extern "C" { 
#endif
	extern VosT_Fun_Status Vos_Catmem_Register (const char * , int , VosT_Void_Null_Proc, VosT_Address *);
	extern VosT_Address Vos_Catmem_Alloc (VosT_Address, size_t);
	extern VosT_Fun_Status Vos_Catmem_Dealloc (VosT_Address);
#if defined (__cplusplus)
	}
#endif


Compcode
wlan_mac_wifib_init (void ** gen_state_pptr)
	{
	int _block_origin = 0;
	static VosT_Address	obtype = OPC_NIL;

	FIN (wlan_mac_wifib_init (gen_state_pptr))

	if (obtype == OPC_NIL)
		{
		/* Initialize memory management */
		if (Vos_Catmem_Register ("proc state vars (wlan_mac_wifib)",
			sizeof (wlan_mac_wifib_state), Vos_Vnop, &obtype) == VOSC_FAILURE)
			{
			FRET (OPC_COMPCODE_FAILURE)
			}
		}

	*gen_state_pptr = Vos_Catmem_Alloc (obtype, 1);
	if (*gen_state_pptr == OPC_NIL)
		{
		FRET (OPC_COMPCODE_FAILURE)
		}
	else
		{
		/* Initialize FSM handling */
		((wlan_mac_wifib_state *)(*gen_state_pptr))->current_block = 0;

		FRET (OPC_COMPCODE_SUCCESS)
		}
	}



void
wlan_mac_wifib_diag (void)
	{
	int _block_origin = __LINE__;

	FIN (wlan_mac_wifib_diag ())

	if (1)
		{
		/* variables used for registering and discovering process models */
		OmsT_Pr_Handle			process_record_handle;
		List*					proc_record_handle_list_ptr;
		int						record_handle_list_size;
		int						ap_count;
		int						count;
		double					sta_addr;
		double					statype ;
		Objid					mac_objid;
		Objid					mac_if_module_objid;
		char					proc_model_name [300];
		Objid					params_attr_objid;
		Objid					wlan_params_comp_attr_objid;
		Objid					strm_objid;
		int						strm;
		int						i_cnt,j_cnt, k_cnt;
		int						addr_index;
		int						num_out_assoc;
		int						node_count;
		int						node_objid;
		WlanT_Hld_List_Elem*	hld_ptr;
		Prohandle				own_prohandle;
		double					timer_duration;
		double					cw_slots;
		char					msg1 [120];
		char					msg2 [120];
		WlanT_Phy_Char_Code		sta_phy_char_flag;
		Boolean					bad_packet_rcvd = OPC_BOOLINT_DISABLED;
		int						pre_rx_status;
		double					pcf_active;
		int						address;
		int						pcf_enabled_stations;
		Boolean					pcf_enabled_on_AP;

		/* Diagnostic Block */


		BINIT
		/* Print information about this process.	*/
		if (wlan_trace_active == OPC_TRUE)
			{
			printf ("Current state name:%s\t", current_state_name);
			}
		
		printf ("Station MAC Address:%d\n", my_address);
		printf ("printing the higher layer queue contents (packet ids)\n");
		for (i_cnt = 0; i_cnt< op_prg_list_size (hld_list_ptr); i_cnt++)
			{
			/* Remove packet from higher layer queue. */
			hld_ptr = (WlanT_Hld_List_Elem*) op_prg_list_access (hld_list_ptr, i_cnt);
			printf ("%d\t", (int) op_pk_id (hld_ptr->pkptr));
			if ((i_cnt% 4) == 0)
				{
				printf ("\n");
				}
			}

		/* End of Diagnostic Block */

		}

	FOUT;
	}




void
wlan_mac_wifib_terminate (void)
	{
	int _block_origin = __LINE__;

	FIN (wlan_mac_wifib_terminate (void))

	if (1)
		{
		/* variables used for registering and discovering process models */
		OmsT_Pr_Handle			process_record_handle;
		List*					proc_record_handle_list_ptr;
		int						record_handle_list_size;
		int						ap_count;
		int						count;
		double					sta_addr;
		double					statype ;
		Objid					mac_objid;
		Objid					mac_if_module_objid;
		char					proc_model_name [300];
		Objid					params_attr_objid;
		Objid					wlan_params_comp_attr_objid;
		Objid					strm_objid;
		int						strm;
		int						i_cnt,j_cnt, k_cnt;
		int						addr_index;
		int						num_out_assoc;
		int						node_count;
		int						node_objid;
		WlanT_Hld_List_Elem*	hld_ptr;
		Prohandle				own_prohandle;
		double					timer_duration;
		double					cw_slots;
		char					msg1 [120];
		char					msg2 [120];
		WlanT_Phy_Char_Code		sta_phy_char_flag;
		Boolean					bad_packet_rcvd = OPC_BOOLINT_DISABLED;
		int						pre_rx_status;
		double					pcf_active;
		int						address;
		int						pcf_enabled_stations;
		Boolean					pcf_enabled_on_AP;

		/* Termination Block */


		BINIT
		{
		
		}

		/* End of Termination Block */

		}
	Vos_Catmem_Dealloc (pr_state_ptr);

	FOUT;
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in wlan_mac_wifib_svar function. */
#undef retry_count
#undef intrpt_type
#undef intrpt_code
#undef my_address
#undef my_objid
#undef my_node_objid
#undef my_subnet_objid
#undef tx_objid
#undef rx_objid
#undef own_process_record_handle
#undef hld_list_ptr
#undef operational_speed
#undef frag_threshold
#undef packet_seq_number
#undef packet_frag_number
#undef destination_addr
#undef fragmentation_buffer_ptr
#undef fresp_to_send
#undef nav_duration
#undef rts_threshold
#undef duplicate_entry
#undef expected_frame_type
#undef remote_sta_addr
#undef backoff_slots
#undef packet_load_handle
#undef intrpt_time
#undef wlan_transmit_frame_copy_ptr
#undef backoff_slots_handle
#undef instrm_from_mac_if
#undef outstrm_to_mac_if
#undef num_fragments
#undef remainder_size
#undef defragmentation_list_ptr
#undef wlan_flags
#undef oms_aa_handle
#undef current_time
#undef rcv_idle_time
#undef duplicate_list_ptr
#undef hld_pmh
#undef max_backoff
#undef current_state_name
#undef hl_packets_rcvd
#undef media_access_delay
#undef ete_delay_handle
#undef global_ete_delay_handle
#undef global_throughput_handle
#undef global_load_handle
#undef global_dropped_data_handle
#undef global_mac_delay_handle
#undef ctrl_traffic_rcvd_handle_inbits
#undef ctrl_traffic_sent_handle_inbits
#undef ctrl_traffic_rcvd_handle
#undef ctrl_traffic_sent_handle
#undef data_traffic_rcvd_handle_inbits
#undef data_traffic_sent_handle_inbits
#undef data_traffic_rcvd_handle
#undef data_traffic_sent_handle
#undef sifs_time
#undef slot_time
#undef cw_min
#undef cw_max
#undef difs_time
#undef plcp_overhead_control
#undef plcp_overhead_data
#undef channel_reserv_handle
#undef retrans_handle
#undef throughput_handle
#undef long_retry_limit
#undef short_retry_limit
#undef retry_limit
#undef last_frametx_type
#undef deference_evh
#undef backoff_elapsed_evh
#undef frame_timeout_evh
#undef eifs_time
#undef i_strm
#undef wlan_trace_active
#undef pkt_in_service
#undef bits_load_handle
#undef ap_flag
#undef bss_flag
#undef ap_mac_address
#undef hld_max_size
#undef max_receive_lifetime
#undef accept_large_packets
#undef phy_char_flag
#undef oms_aa_wlan_handle
#undef total_hlpk_size
#undef drop_packet_handle
#undef drop_packet_handle_inbits
#undef drop_pkt_log_handle
#undef drop_pkt_entry_log_flag
#undef packet_size
#undef receive_time
#undef llc_iciptr
#undef rcv_channel_status
#undef rx_power_threshold
#undef bss_stn_list
#undef bss_stn_count
#undef bss_id
#undef pcf_retry_count
#undef poll_fail_count
#undef max_poll_fails
#undef cfpd_list_ptr
#undef pcf_queue_offset
#undef beacon_int
#undef pcf_frag_buffer_ptr
#undef wlan_pcf_transmit_frame_copy_ptr
#undef pcf_num_fragments
#undef pcf_remainder_size
#undef polling_list
#undef poll_list_size
#undef poll_index
#undef pifs_time
#undef beacon_evh
#undef cfp_end_evh
#undef pcf_pkt_in_service
#undef pcf_flag
#undef active_pc
#undef cfp_prd
#undef cfp_offset
#undef cfp_length
#undef ap_relay
#undef total_cfpd_size
#undef packet_size_dcf
#undef packet_size_pcf
#undef receive_time_dcf
#undef receive_time_pcf
#undef cfp_ap_medium_control
#undef pcf_network
#undef my_route
#undef is_mac_border_node
#undef backoff_type



void
wlan_mac_wifib_svar (void * gen_ptr, const char * var_name, char ** var_p_ptr)
	{
	wlan_mac_wifib_state		*prs_ptr;

	FIN (wlan_mac_wifib_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (char *)OPC_NIL;
		FOUT;
		}
	prs_ptr = (wlan_mac_wifib_state *)gen_ptr;

	if (strcmp ("retry_count" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->retry_count);
		FOUT;
		}
	if (strcmp ("intrpt_type" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->intrpt_type);
		FOUT;
		}
	if (strcmp ("intrpt_code" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->intrpt_code);
		FOUT;
		}
	if (strcmp ("my_address" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_address);
		FOUT;
		}
	if (strcmp ("my_objid" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_objid);
		FOUT;
		}
	if (strcmp ("my_node_objid" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_node_objid);
		FOUT;
		}
	if (strcmp ("my_subnet_objid" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_subnet_objid);
		FOUT;
		}
	if (strcmp ("tx_objid" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->tx_objid);
		FOUT;
		}
	if (strcmp ("rx_objid" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->rx_objid);
		FOUT;
		}
	if (strcmp ("own_process_record_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->own_process_record_handle);
		FOUT;
		}
	if (strcmp ("hld_list_ptr" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->hld_list_ptr);
		FOUT;
		}
	if (strcmp ("operational_speed" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->operational_speed);
		FOUT;
		}
	if (strcmp ("frag_threshold" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->frag_threshold);
		FOUT;
		}
	if (strcmp ("packet_seq_number" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->packet_seq_number);
		FOUT;
		}
	if (strcmp ("packet_frag_number" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->packet_frag_number);
		FOUT;
		}
	if (strcmp ("destination_addr" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->destination_addr);
		FOUT;
		}
	if (strcmp ("fragmentation_buffer_ptr" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->fragmentation_buffer_ptr);
		FOUT;
		}
	if (strcmp ("fresp_to_send" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->fresp_to_send);
		FOUT;
		}
	if (strcmp ("nav_duration" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->nav_duration);
		FOUT;
		}
	if (strcmp ("rts_threshold" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->rts_threshold);
		FOUT;
		}
	if (strcmp ("duplicate_entry" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->duplicate_entry);
		FOUT;
		}
	if (strcmp ("expected_frame_type" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->expected_frame_type);
		FOUT;
		}
	if (strcmp ("remote_sta_addr" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->remote_sta_addr);
		FOUT;
		}
	if (strcmp ("backoff_slots" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->backoff_slots);
		FOUT;
		}
	if (strcmp ("packet_load_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->packet_load_handle);
		FOUT;
		}
	if (strcmp ("intrpt_time" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->intrpt_time);
		FOUT;
		}
	if (strcmp ("wlan_transmit_frame_copy_ptr" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->wlan_transmit_frame_copy_ptr);
		FOUT;
		}
	if (strcmp ("backoff_slots_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->backoff_slots_handle);
		FOUT;
		}
	if (strcmp ("instrm_from_mac_if" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->instrm_from_mac_if);
		FOUT;
		}
	if (strcmp ("outstrm_to_mac_if" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->outstrm_to_mac_if);
		FOUT;
		}
	if (strcmp ("num_fragments" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->num_fragments);
		FOUT;
		}
	if (strcmp ("remainder_size" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->remainder_size);
		FOUT;
		}
	if (strcmp ("defragmentation_list_ptr" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->defragmentation_list_ptr);
		FOUT;
		}
	if (strcmp ("wlan_flags" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->wlan_flags);
		FOUT;
		}
	if (strcmp ("oms_aa_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->oms_aa_handle);
		FOUT;
		}
	if (strcmp ("current_time" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->current_time);
		FOUT;
		}
	if (strcmp ("rcv_idle_time" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->rcv_idle_time);
		FOUT;
		}
	if (strcmp ("duplicate_list_ptr" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->duplicate_list_ptr);
		FOUT;
		}
	if (strcmp ("hld_pmh" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->hld_pmh);
		FOUT;
		}
	if (strcmp ("max_backoff" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->max_backoff);
		FOUT;
		}
	if (strcmp ("current_state_name" , var_name) == 0)
		{
		*var_p_ptr = (char *) (prs_ptr->current_state_name);
		FOUT;
		}
	if (strcmp ("hl_packets_rcvd" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->hl_packets_rcvd);
		FOUT;
		}
	if (strcmp ("media_access_delay" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->media_access_delay);
		FOUT;
		}
	if (strcmp ("ete_delay_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->ete_delay_handle);
		FOUT;
		}
	if (strcmp ("global_ete_delay_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->global_ete_delay_handle);
		FOUT;
		}
	if (strcmp ("global_throughput_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->global_throughput_handle);
		FOUT;
		}
	if (strcmp ("global_load_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->global_load_handle);
		FOUT;
		}
	if (strcmp ("global_dropped_data_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->global_dropped_data_handle);
		FOUT;
		}
	if (strcmp ("global_mac_delay_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->global_mac_delay_handle);
		FOUT;
		}
	if (strcmp ("ctrl_traffic_rcvd_handle_inbits" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->ctrl_traffic_rcvd_handle_inbits);
		FOUT;
		}
	if (strcmp ("ctrl_traffic_sent_handle_inbits" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->ctrl_traffic_sent_handle_inbits);
		FOUT;
		}
	if (strcmp ("ctrl_traffic_rcvd_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->ctrl_traffic_rcvd_handle);
		FOUT;
		}
	if (strcmp ("ctrl_traffic_sent_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->ctrl_traffic_sent_handle);
		FOUT;
		}
	if (strcmp ("data_traffic_rcvd_handle_inbits" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->data_traffic_rcvd_handle_inbits);
		FOUT;
		}
	if (strcmp ("data_traffic_sent_handle_inbits" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->data_traffic_sent_handle_inbits);
		FOUT;
		}
	if (strcmp ("data_traffic_rcvd_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->data_traffic_rcvd_handle);
		FOUT;
		}
	if (strcmp ("data_traffic_sent_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->data_traffic_sent_handle);
		FOUT;
		}
	if (strcmp ("sifs_time" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->sifs_time);
		FOUT;
		}
	if (strcmp ("slot_time" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->slot_time);
		FOUT;
		}
	if (strcmp ("cw_min" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->cw_min);
		FOUT;
		}
	if (strcmp ("cw_max" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->cw_max);
		FOUT;
		}
	if (strcmp ("difs_time" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->difs_time);
		FOUT;
		}
	if (strcmp ("plcp_overhead_control" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->plcp_overhead_control);
		FOUT;
		}
	if (strcmp ("plcp_overhead_data" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->plcp_overhead_data);
		FOUT;
		}
	if (strcmp ("channel_reserv_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->channel_reserv_handle);
		FOUT;
		}
	if (strcmp ("retrans_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->retrans_handle);
		FOUT;
		}
	if (strcmp ("throughput_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->throughput_handle);
		FOUT;
		}
	if (strcmp ("long_retry_limit" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->long_retry_limit);
		FOUT;
		}
	if (strcmp ("short_retry_limit" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->short_retry_limit);
		FOUT;
		}
	if (strcmp ("retry_limit" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->retry_limit);
		FOUT;
		}
	if (strcmp ("last_frametx_type" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->last_frametx_type);
		FOUT;
		}
	if (strcmp ("deference_evh" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->deference_evh);
		FOUT;
		}
	if (strcmp ("backoff_elapsed_evh" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->backoff_elapsed_evh);
		FOUT;
		}
	if (strcmp ("frame_timeout_evh" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->frame_timeout_evh);
		FOUT;
		}
	if (strcmp ("eifs_time" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->eifs_time);
		FOUT;
		}
	if (strcmp ("i_strm" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->i_strm);
		FOUT;
		}
	if (strcmp ("wlan_trace_active" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->wlan_trace_active);
		FOUT;
		}
	if (strcmp ("pkt_in_service" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pkt_in_service);
		FOUT;
		}
	if (strcmp ("bits_load_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->bits_load_handle);
		FOUT;
		}
	if (strcmp ("ap_flag" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->ap_flag);
		FOUT;
		}
	if (strcmp ("bss_flag" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->bss_flag);
		FOUT;
		}
	if (strcmp ("ap_mac_address" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->ap_mac_address);
		FOUT;
		}
	if (strcmp ("hld_max_size" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->hld_max_size);
		FOUT;
		}
	if (strcmp ("max_receive_lifetime" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->max_receive_lifetime);
		FOUT;
		}
	if (strcmp ("accept_large_packets" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->accept_large_packets);
		FOUT;
		}
	if (strcmp ("phy_char_flag" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->phy_char_flag);
		FOUT;
		}
	if (strcmp ("oms_aa_wlan_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->oms_aa_wlan_handle);
		FOUT;
		}
	if (strcmp ("total_hlpk_size" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->total_hlpk_size);
		FOUT;
		}
	if (strcmp ("drop_packet_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->drop_packet_handle);
		FOUT;
		}
	if (strcmp ("drop_packet_handle_inbits" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->drop_packet_handle_inbits);
		FOUT;
		}
	if (strcmp ("drop_pkt_log_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->drop_pkt_log_handle);
		FOUT;
		}
	if (strcmp ("drop_pkt_entry_log_flag" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->drop_pkt_entry_log_flag);
		FOUT;
		}
	if (strcmp ("packet_size" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->packet_size);
		FOUT;
		}
	if (strcmp ("receive_time" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->receive_time);
		FOUT;
		}
	if (strcmp ("llc_iciptr" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->llc_iciptr);
		FOUT;
		}
	if (strcmp ("rcv_channel_status" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->rcv_channel_status);
		FOUT;
		}
	if (strcmp ("rx_power_threshold" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->rx_power_threshold);
		FOUT;
		}
	if (strcmp ("bss_stn_list" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->bss_stn_list);
		FOUT;
		}
	if (strcmp ("bss_stn_count" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->bss_stn_count);
		FOUT;
		}
	if (strcmp ("bss_id" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->bss_id);
		FOUT;
		}
	if (strcmp ("pcf_retry_count" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pcf_retry_count);
		FOUT;
		}
	if (strcmp ("poll_fail_count" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->poll_fail_count);
		FOUT;
		}
	if (strcmp ("max_poll_fails" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->max_poll_fails);
		FOUT;
		}
	if (strcmp ("cfpd_list_ptr" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->cfpd_list_ptr);
		FOUT;
		}
	if (strcmp ("pcf_queue_offset" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pcf_queue_offset);
		FOUT;
		}
	if (strcmp ("beacon_int" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->beacon_int);
		FOUT;
		}
	if (strcmp ("pcf_frag_buffer_ptr" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pcf_frag_buffer_ptr);
		FOUT;
		}
	if (strcmp ("wlan_pcf_transmit_frame_copy_ptr" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->wlan_pcf_transmit_frame_copy_ptr);
		FOUT;
		}
	if (strcmp ("pcf_num_fragments" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pcf_num_fragments);
		FOUT;
		}
	if (strcmp ("pcf_remainder_size" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pcf_remainder_size);
		FOUT;
		}
	if (strcmp ("polling_list" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->polling_list);
		FOUT;
		}
	if (strcmp ("poll_list_size" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->poll_list_size);
		FOUT;
		}
	if (strcmp ("poll_index" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->poll_index);
		FOUT;
		}
	if (strcmp ("pifs_time" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pifs_time);
		FOUT;
		}
	if (strcmp ("beacon_evh" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->beacon_evh);
		FOUT;
		}
	if (strcmp ("cfp_end_evh" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->cfp_end_evh);
		FOUT;
		}
	if (strcmp ("pcf_pkt_in_service" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pcf_pkt_in_service);
		FOUT;
		}
	if (strcmp ("pcf_flag" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pcf_flag);
		FOUT;
		}
	if (strcmp ("active_pc" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->active_pc);
		FOUT;
		}
	if (strcmp ("cfp_prd" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->cfp_prd);
		FOUT;
		}
	if (strcmp ("cfp_offset" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->cfp_offset);
		FOUT;
		}
	if (strcmp ("cfp_length" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->cfp_length);
		FOUT;
		}
	if (strcmp ("ap_relay" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->ap_relay);
		FOUT;
		}
	if (strcmp ("total_cfpd_size" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->total_cfpd_size);
		FOUT;
		}
	if (strcmp ("packet_size_dcf" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->packet_size_dcf);
		FOUT;
		}
	if (strcmp ("packet_size_pcf" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->packet_size_pcf);
		FOUT;
		}
	if (strcmp ("receive_time_dcf" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->receive_time_dcf);
		FOUT;
		}
	if (strcmp ("receive_time_pcf" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->receive_time_pcf);
		FOUT;
		}
	if (strcmp ("cfp_ap_medium_control" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->cfp_ap_medium_control);
		FOUT;
		}
	if (strcmp ("pcf_network" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pcf_network);
		FOUT;
		}
	if (strcmp ("my_route" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_route);
		FOUT;
		}
	if (strcmp ("is_mac_border_node" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->is_mac_border_node);
		FOUT;
		}
	if (strcmp ("backoff_type" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->backoff_type);
		FOUT;
		}
	*var_p_ptr = (char *)OPC_NIL;

	FOUT;
	}

