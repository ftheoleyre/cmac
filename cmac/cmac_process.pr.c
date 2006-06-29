/* Process model C form file: cmac_process.pr.c */
/* Portions of this file copyright 1992-2002 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
static const char cmac_process_pr_c [] = "MIL_3_Tfile_Hdr_ 81A 30A modeler 7 44A39F0B 44A39F0B 1 ares-theo-1 ftheoley 0 0 none none 0 0 none 0 0 0 0 0 0                                                                                                                                                                                                                                                                                                                                                                                                                 ";
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
#include 	<math.h>
#include 	<string.h>
#include 	<stdarg.h>
#include 	<stdio.h>
#include 	<stdlib.h>





//-----------------------------------------------
//				INTER-FRAME TIME
//-----------------------------------------------

#define		SIFS							10E-6
#define		PIFS							30E-6
#define		DIFS							50E-6
#define		EIFS							(2*SIFS + 1500 / operational_speed)

#define		SLOT_BACKOFF					15E-6





//-----------------------------------------------
//				COMMON
//-----------------------------------------------

//reserved addresses
#define		BROADCAST						-1

//backoff
#define		MAX_BACKOFF						32
#define		MAX_EXPO_BACKOFF				1024

//Power
#define		MAX_BORDER_DIST					10

#define		PI								3.141592653589793238462 





//-----------------------------------------------
//				TIMEOUTS
//-----------------------------------------------

//Propagation delay = distance / light speed
#define		PROPAGATION_DELAY				(2E-6)

#define		MAX_NB_RETRY					7
#define		MEDIUM_NB_RETRY					3


//To avoid farmes duplication
#define		TIMEOUT_FRAME_ID				5.0


//A data frame is deleted if not sent after TIMEOUT seconds
#define		TIMEOUT_DATA_FRAME				0.5






//-----------------------------------------------
//				HELLOS
//-----------------------------------------------

#define		INTERVALL_HELLO					30.0

//The maximum number of stabilities to store
#define		MAX_STAB						15

//Two stabilities with STAB_STEP difference are considered equal
#define		STAB_STEP						3




//-----------------------------------------------
//				CTR / PRIVILEGE
//-----------------------------------------------


// PRIVILEGED MODE
#define		PRIV_MIN_RATIO					0.9



//Maximum number of branches for the AP
#define		MAX_NB_BRANCHES					4


//A CTR could be delayed
#define		MAX_CTR_DELAY_FROM_SINK			100E-6




//-----------------------------------------------
//				SYNC
//-----------------------------------------------

#define		SYNC_DIRECT_ANTENNA				OPC_TRUE






//-----------------------------------------------
//				BUSY TONE
//-----------------------------------------------

//The frequency for the busy tone is 
//SHIFT_FREQ_SEPARATION MHz less than for the transmission radio
#define		SHIFT_FREQ_SEPARATION			0.2					






//-----------------------------------------------
//				MULTI CHANNEL
//-----------------------------------------------

//#define		MULTI_CHANNEL					1
#define		RATIO_PRIV_BANDWIDTH 			1






//-----------------------------------------------
//			BORDER NODES ELECTION
//-----------------------------------------------

#define		BORDER_DYNAMIC					1
#define		BORDER_CENTRALIZED				2



//-----------------------------------------------
//				ROUTING
//-----------------------------------------------

#define		ROUTING_BORDER					2
#define		ROUTING_SHORT					1
#define		ROUTING_NO						0






//-----------------------------------------------
//				STREAM & STAT
//-----------------------------------------------

#define		STREAM_TO_UP					5
#define		STREAM_FROM_UP					5

#define		STREAM_TO_RADIO					0
#define		STREAM_FROM_RADIO				0

#define		STREAM_TO_BUSY_TONE				1
#define		STREAM_FROM_BUSY_TONE			1

#define		STREAM_TO_DIRECT_SYNC			2


#define		STAT_FROM_RX					0
#define		STAT_FROM_TX					1

#define		STAT_FROM_RX_BUSY_TONE			2
#define		STAT_FROM_TX_BUSY_TONE			3

#define		STAT_FROM_TX_DIREC_SYNC			4





//-----------------------------------------------
//				DEBUG
//-----------------------------------------------


#define		NO								0
#define		LOW								1
#define		MEDIUM							2
#define		MAX								3


#define		DEBUG_GLOBAL					0
#define		DEBUG_STATE						1

#define		DEBUG_BACKOFF					2

#define		DEBUG_SEND						3
#define		DEBUG_RECEIVE					4
#define		DEBUG_RADIO						5

#define		DEBUG_CONTROL					6

#define		DEBUG_TIMEOUT					7

#define		DEBUG_HELLO						8

#define		DEBUG_UP						9

#define		DEBUG_NODE						10






//-----------------------------------------------
//		   	INTERRUPTIONS
//-----------------------------------------------

#define		PK_FROM_UPPER					((op_intrpt_type() == OPC_INTRPT_STRM) && (op_intrpt_strm() == STREAM_FROM_UP))
#define		PK_FROM_LOWER					((op_intrpt_type() == OPC_INTRPT_STRM) && (op_intrpt_strm() != STREAM_FROM_UP))



//We received a frame
#define		IS_FRAME_RECEIVED				((op_intrpt_type() == OPC_INTRPT_STRM) && (op_intrpt_strm() == STREAM_FROM_RADIO))


//We answer to a received frame (we are in backoff and we receive a frame -> whatever the frame was, we must reply (with a CTS, ACK...)
//NB: we reply only if we are not in communication (bu such a case does not exist since a backoff is not required when a flow is already initiated)
#define		IS_REPLY_TO_SEND				((get_nav_main_freq() <= op_sim_time()) && (!is_reply_required))


//Frame to received (acak, cts...) -> timeout	
#define		IS_FRAME_TIMEOUT				((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == FRAME_TIMEOUT_CODE))


//Is the medium busy ? (transmission / reception / reservation)
#define		IS_MEDIUM_BUSY					((is_rx_busy) || (is_tx_busy) || (get_nav_main_freq() >= op_sim_time()) || ((is_busy_tone_rx && !busy_tone_rx_ignored && BUSY_TONE_ACTIVATED) && (!is_border_node)))


//We must defer in any of these conditions: 
//- a CTR msut be transmitted 
//- a packet was received and we must reply (ACK, CTS, CTR...) 
//- the medium is busy (We must get another backoff) 
#define		IS_BACK_TO_DEFER				((next_frame_to_send.type == CTR_PK_TYPE) || (IS_FRAME_RECEIVED && IS_REPLY_TO_SEND))


//We must transmit one packet: our data buffer is not empty OR we have already prepared a frame to send
#define		IS_DATA_OK						((!strict_privileged_mode) || (is_hello_to_send && !IS_BROADCAST_FORBIDDEN) || !is_border_node || is_node_privileged || is_sink)
#define		IS_PK_TO_SEND					((IS_DATA_OK && !is_frame_buffer_empty()) || (next_frame_to_send.type != NO_PK_TYPE))




//The mode privileged is finished !
// -> The node must become unprivileged
// -> Or we have no more data frames to send, and we remained privileged for a sufficiently long time
//
//	NB: if we receive PRIVILEGED_MAX_CODE after becoming a unpriviledge mode, nothing happens
//		and two privileged modes are sufficiently inter spaced so that no problem occurs 
//		the sink sends CTR with a period largely superior to PRIVILEGED_MAX_TIME (PRIVILEGED_MAX_TIME * NB_BRANCH * BETA)
//		Moreover, we have a second verification after having received a PRIVILEGED_MAX_CODE
//
#define		PRIVILEGED_END					(PRIVILEGED_MEDIUM_LIMIT || PRIVILEGED_HIGH_LIMIT)

#define		END_PRIV_SLOT					(time_start_privileged + slot_privileged_duration - slot_privileged_offset <= op_sim_time())
#define		END_PRIV_INTRPT					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == PRIVILEGED_MAX_CODE))

#define		PRIVILEGED_MEDIUM_LIMIT			((is_node_privileged) && (is_frame_buffer_empty()) && (time_start_privileged + slot_privileged_duration * PRIV_MIN_RATIO <= op_sim_time()))
#define		PRIVILEGED_HIGH_LIMIT			((is_node_privileged) && (END_PRIV_SLOT || END_PRIV_INTRPT))


#define		MAIN_FREQ_RETURN				((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == MAIN_FREQ_RETURN_CODE))


//No frame to send
#define		NO_FRAME						(next_frame_to_send.type == NO_PK_TYPE)


//this node has the priority to sends its packets
#define		IS_NODE_PRIO					((next_frame_to_send.type == CTS_PK_TYPE) || (next_frame_to_send.type == ACK_PK_TYPE) || (next_frame_to_send.type == DATA_UNICAST_PK_TYPE))


//Collision: we have the priority to send a reply, but the medium is busy (another node is not aware of the current communicaiton)
#define		PRIORITY_AND_MEDIUM_BUSY		((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == DEFER_CODE) && (IS_MEDIUM_BUSY) && (IS_NODE_PRIO))


//transmission canceled from the DEFER state (no frame to send, or a collision occured
#define		TRANSMISSION_CANCELED			(NO_FRAME || PRIORITY_AND_MEDIUM_BUSY)


//Go to the 'null' state until the simulation terminated
#define		END_SIMULATION					(next_start_time == OPC_DBL_INFINITY)



//I will backoff
#define		GO_TO_BACKOFF					((!TRANSMISSION_CANCELED) && (!IS_MEDIUM_BUSY) && (my_backoff > 0))


//I will directly send one packet
#define		GO_TO_SEND						((!TRANSMISSION_CANCELED) && (!IS_MEDIUM_BUSY) && (my_backoff == 0))


//I can send one BROADCAST frame -> other nodes will receive it
//Special case: the source is the sink (I can send a packet even if I am not privileged (I am never))
#define		IS_BROADCAST_FORBIDDEN			0
//((!is_node_privileged && is_border_node && !is_sink && (nb_channels == 1)) || (is_nav_for_other_freq()) || (!is_main_freq_active(STREAM_TO_RADIO)))
//TAG







//-----------------------------------------------
//		   	INTERRUPTION CODES
//-----------------------------------------------


//A frame has timeouted -> no ACK/CTS received
#define		FRAME_TIMEOUT_CODE				1


//The sink must generate a CTR
#define		SINK_CTR_CODE					2


//The node must wait the interframe time berfore the transmission
#define		DEFER_CODE						3


//A CTR packet has to be sent
#define		CTR_PK_CODE						4


//A Hello packet has to be sent
#define		HELLO_PK_CODE					5


//The backoff interruption
#define		BACKOFF_CODE					6


//Verification of timeouts in the neighborhood_table
#define		NEIGHBOR_TIMEOUT_CODE			7


//A nav is expired -> unblock potential transmissions
#define		NAV_END_CODE					8


//The node must become unprivileged
#define		PRIVILEGED_MIN_CODE				9
//The node should become unprivileged if its data buffer is empty
#define		PRIVILEGED_MAX_CODE				10


//The code to generate a sync frame
#define		SINK_SYNC_CODE					11


//The periodical deletion of frame_ids
#define		FRAME_ID_TIMEOUT_CODE			12


//I must return to my main frequency
#define		MAIN_FREQ_RETURN_CODE			13



//-----------------------------------------------
//		   	PACKET TYPES
//-----------------------------------------------

#define		NO_PK_TYPE						0


#define		RTS_PK_TYPE						1
#define		CTS_PK_TYPE						2
#define		CTR_PK_TYPE						3
#define		CTR_END_PK_TYPE					4
#define		DATA_UNICAST_PK_TYPE			5
#define		DATA_MULTICAST_PK_TYPE			6
#define		ACK_PK_TYPE						7
#define		HELLO_PK_TYPE					8
#define		SYNC_PK_TYPE					9
#define		CTR_ACK_PK_TYPE					10


//Preamble + 2*addresses + Type + FCS
#define		HEADERS_PK_SIZE					(57 + 48*2 + 4 + 32)
#define		NB_TIER_SIZE					4
#define		DURATION_SIZE					16
#define		DIST_SINK_SIZE					4
#define		DIST_BORDER_SIZE				4


//MTU in bits
#define		MTU_MAX							2400.0

	

//-----------------------------------------------
//		   DEBUG FILES
//-----------------------------------------------

//Common -> but no pb since no concurrent execution (discrete event simulator)
FILE	*debug_files[20];
int		cmac_timestamp = 0;

//Number of nodes
int		nb_mac_nodes = 0;




//-----------------------------------------------
//					FRAME
//-----------------------------------------------

int	global_frame_id = 0;


typedef struct{
	int			source;
	int			destination;
	short		type;
	int			frame_id;
	Boolean		nb_retry;
	double		time_added;
	double		time_sent;
	double		time_transmission_min;
	double		pk_size;
	int			duration;			//for RTS / CTS (pk_size of the data_frame)
	double		ifs;				//inter frame spacing for this packet type
	Boolean		released;			//The memory was released (packet destroyed)
	Packet		*payload;
	double		next_hello;
	double		power_ratio;
}frame_struct;


//-----------------------------------------------
//					HELLOS
//-----------------------------------------------


typedef struct{
	int		address;
	int		dist_sink;
	int		dist_border;
	double	sync_rx_power;
	int		branch;
	List	*border_nodes_list;	
	Boolean	stability[MAX_STAB];
	double	timeout;
} neigh_struct;





//-----------------------------------------------
//				FRAME ID LIST
//-----------------------------------------------


typedef struct{
	int		id;
	double	timeout;
}id_timeout_struct;





//-----------------------------------------------
//					NAV
//-----------------------------------------------


typedef struct{
	int		address;
	double	timeout;
	double	frequency;
}nav_struct;





//-----------------------------------------------
//			COMPARISON FOR NEXT HOP
//-----------------------------------------------


typedef struct{
	int		addr;
	short	prio;
	short	stab;
} compar_struct;






//-----------------------------------------------
//			ELECTION OF BORDER NODES
//-----------------------------------------------


//distributed method
typedef struct{
	int		addr;
	double	pow;
	short	stab;
	short	branch;
}election_struct;


int	MAX_BRANCH_LENGTH = 0;



//centralized method
typedef struct{
	int	address;
	int	parent;
}bn_struct;

//-----------------------------------------------
//			LIST OF POSITIONS
//-----------------------------------------------


typedef struct{
	double	x;
	double	y;
	int		address;
} pos_struct;





//-----------------------------------------------
//					PRIVILEGE
//-----------------------------------------------

double TIME_MAX_PRIVILEGED;




//-----------------------------------------------
//				GLOBAL LISTS
//-----------------------------------------------

//to compute border nodes in a centralized manner
List *global_border_nodes_list;

//To have the position of all nodes
List *positions_list;


//-----------------------------------------------
//				PROTOTYPES
//-----------------------------------------------

//debug
void	debug_print(const int level, const int type , const char* fmt, ...);
char* 	pk_type_to_str(short pk_type , char *msg);
double 	convert_double(double value);
int	 	convert_int(int value);


//Stats
void	end_sim();


//frame management
int 	get_new_frame_id();


//data frames
frame_struct *get_unicast_frame_buffer(int pos);
frame_struct *get_multicast_frame_buffer(int pos);


//Hellos
void 	print_neighborhood_table(int debug_type);
void  	generate_hello(double next_hello);
void 	update_neighborhood_table(int source , int dist_sink , int dist_border, double sync_rx_power, int branch , List *bn_list_tmp , double next_hello);
char* 	print_border_nodes(char *msg);


//Stability
int 	compute_stability	(int stab[]);
void 	update_stability	(int stab[], short value);
void 	init_stability		(int stab[], short value);


//To compute duration for NAV
double 	compute_rts_cts_data_ack_time(int data_pk_size);
double 	compute_cts_data_ack_time(int data_pk_size);
double 	compute_data_ack_time(int data_pk_size);


//Busy tone
void 	maintain_busy_tone(double time);


//antennas
void 	change_antenna_direction(int stream , int branch);
void 	change_tx_power(double power , int stream);

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
	int	                    		my_address;
	Boolean	                		is_reply_required;
	Boolean	                		is_reply_bad;
	Boolean	                		is_reply_received;
	frame_struct	           		next_frame_to_send;
	frame_struct	           		last_frame_sent;
	List*	                  		unicast_frame_buffer;
	List*	                  		multicast_frame_buffer;
	double	                 		operational_speed;
	Boolean	                		is_border_node;
	Boolean	                		is_node_privileged;
	Boolean	                		is_sink;
	int	                    		my_dist_sink;
	int	                    		my_dist_border;
	List*	                  		my_neighborhood_table;
	int	                    		nb_channels;
	Boolean	                		is_tx_busy;
	double	                 		my_current_tx_power;
	Boolean	                		is_rx_busy;
	double	                 		rx_power_threshold;
	double	                 		my_sync_rx_power;
	List*	                  		my_nav_list;
	int	                    		cw;
	int	                    		my_backoff;
	Evhandle	               		backoff_intrpt;
	Distribution *	         		backoff_dist;
	double	                 		time_start_privileged;
	Evhandle	               		frame_timeout_intrpt;
	Evhandle	               		timeout_intrpt;
	Evhandle	               		main_freq_return_intrpt;
	int	                    		DEBUG;
	Evhandle	               		defer_intrpt;
	int	                    		ctr_last_branch;
	double	                 		last_rx_power;
	double	                 		my_main_frequency;
	double	                 		my_main_bandwidth;
	Boolean	                		is_busy_tone_rx;
	double	                 		busy_tone_speed;
	Boolean	                		is_busy_tone_tx;
	Boolean	                		busy_tone_rx_ignored;
	Boolean	                		BUSY_TONE_ACTIVATED;
	FILE*	                  		my_debug_file;
	int	                    		my_stat_id;
	Boolean	                		is_hello_to_send;
	List*	                  		my_frame_id_seen;
	double	                 		next_start_time;
	double	                 		last_next_start_time_verif;
	Boolean	                		is_ctr_activated;
	Boolean	                		strict_privileged_mode;
	int	                    		is_sync_direct_antenna;
	int	                    		sync_last_branch;
	int	                    		my_branch;
	double	                 		my_privileged_frequency;
	double	                 		slot_privileged_duration;
	double	                 		slot_privileged_offset;
	List*	                  		bn_list;
	int	                    		BETA;
	double	                 		RTS_PK_SIZE;
	int	                    		MAC_ROUTING;
	double	                 		POWER_TX;
	int	                    		border_election;
	double	                 		RADIO_RANGE;
	} cmac_process_state;

#define pr_state_ptr            		((cmac_process_state*) SimI_Mod_State_Ptr)
#define my_address              		pr_state_ptr->my_address
#define is_reply_required       		pr_state_ptr->is_reply_required
#define is_reply_bad            		pr_state_ptr->is_reply_bad
#define is_reply_received       		pr_state_ptr->is_reply_received
#define next_frame_to_send      		pr_state_ptr->next_frame_to_send
#define last_frame_sent         		pr_state_ptr->last_frame_sent
#define unicast_frame_buffer    		pr_state_ptr->unicast_frame_buffer
#define multicast_frame_buffer  		pr_state_ptr->multicast_frame_buffer
#define operational_speed       		pr_state_ptr->operational_speed
#define is_border_node          		pr_state_ptr->is_border_node
#define is_node_privileged      		pr_state_ptr->is_node_privileged
#define is_sink                 		pr_state_ptr->is_sink
#define my_dist_sink            		pr_state_ptr->my_dist_sink
#define my_dist_border          		pr_state_ptr->my_dist_border
#define my_neighborhood_table   		pr_state_ptr->my_neighborhood_table
#define nb_channels             		pr_state_ptr->nb_channels
#define is_tx_busy              		pr_state_ptr->is_tx_busy
#define my_current_tx_power     		pr_state_ptr->my_current_tx_power
#define is_rx_busy              		pr_state_ptr->is_rx_busy
#define rx_power_threshold      		pr_state_ptr->rx_power_threshold
#define my_sync_rx_power        		pr_state_ptr->my_sync_rx_power
#define my_nav_list             		pr_state_ptr->my_nav_list
#define cw                      		pr_state_ptr->cw
#define my_backoff              		pr_state_ptr->my_backoff
#define backoff_intrpt          		pr_state_ptr->backoff_intrpt
#define backoff_dist            		pr_state_ptr->backoff_dist
#define time_start_privileged   		pr_state_ptr->time_start_privileged
#define frame_timeout_intrpt    		pr_state_ptr->frame_timeout_intrpt
#define timeout_intrpt          		pr_state_ptr->timeout_intrpt
#define main_freq_return_intrpt 		pr_state_ptr->main_freq_return_intrpt
#define DEBUG                   		pr_state_ptr->DEBUG
#define defer_intrpt            		pr_state_ptr->defer_intrpt
#define ctr_last_branch         		pr_state_ptr->ctr_last_branch
#define last_rx_power           		pr_state_ptr->last_rx_power
#define my_main_frequency       		pr_state_ptr->my_main_frequency
#define my_main_bandwidth       		pr_state_ptr->my_main_bandwidth
#define is_busy_tone_rx         		pr_state_ptr->is_busy_tone_rx
#define busy_tone_speed         		pr_state_ptr->busy_tone_speed
#define is_busy_tone_tx         		pr_state_ptr->is_busy_tone_tx
#define busy_tone_rx_ignored    		pr_state_ptr->busy_tone_rx_ignored
#define BUSY_TONE_ACTIVATED     		pr_state_ptr->BUSY_TONE_ACTIVATED
#define my_debug_file           		pr_state_ptr->my_debug_file
#define my_stat_id              		pr_state_ptr->my_stat_id
#define is_hello_to_send        		pr_state_ptr->is_hello_to_send
#define my_frame_id_seen        		pr_state_ptr->my_frame_id_seen
#define next_start_time         		pr_state_ptr->next_start_time
#define last_next_start_time_verif		pr_state_ptr->last_next_start_time_verif
#define is_ctr_activated        		pr_state_ptr->is_ctr_activated
#define strict_privileged_mode  		pr_state_ptr->strict_privileged_mode
#define is_sync_direct_antenna  		pr_state_ptr->is_sync_direct_antenna
#define sync_last_branch        		pr_state_ptr->sync_last_branch
#define my_branch               		pr_state_ptr->my_branch
#define my_privileged_frequency 		pr_state_ptr->my_privileged_frequency
#define slot_privileged_duration		pr_state_ptr->slot_privileged_duration
#define slot_privileged_offset  		pr_state_ptr->slot_privileged_offset
#define bn_list                 		pr_state_ptr->bn_list
#define BETA                    		pr_state_ptr->BETA
#define RTS_PK_SIZE             		pr_state_ptr->RTS_PK_SIZE
#define MAC_ROUTING             		pr_state_ptr->MAC_ROUTING
#define POWER_TX                		pr_state_ptr->POWER_TX
#define border_election         		pr_state_ptr->border_election
#define RADIO_RANGE             		pr_state_ptr->RADIO_RANGE

/* This macro definition will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE
#define FIN_PREAMBLE	cmac_process_state *op_sv_ptr = pr_state_ptr;


/* Function Block */

enum { _block_origin = __LINE__ };

//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------																										  ---------------------------
//---------------------------												COMMON													  ---------------------------
//---------------------------																										  ---------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------












//-----------------------------------------------------------
//
//					FRAME TIMEOUTS
//
//-----------------------------------------------------------

//Adds a timeout interruption
//NB : i have a blocking state -> I do not send any other packet until either I received the reply or the frame timeouts
void add_frame_timeout(double data_pk_size){
	
	//error, the timeout interruption was not canceled !
	if (op_ev_valid(timeout_intrpt))
		op_ev_cancel(timeout_intrpt);
	if (op_ev_valid(timeout_intrpt))
		op_sim_end("A timeout interruption must be canceled when the reply is received" , "Please correct this bug", "" , "");

	//adds a timeout interruption
	timeout_intrpt = op_intrpt_schedule_self(op_sim_time() + 2*SIFS + 2*PROPAGATION_DELAY + (double)data_pk_size/operational_speed, FRAME_TIMEOUT_CODE);
	
	//debug
	debug_print(LOW , DEBUG_TIMEOUT , "adds a timeout for %fus\n", 1E6 * (2*SIFS + 2*PROPAGATION_DELAY + (double)data_pk_size/operational_speed) );

}









//-----------------------------------------------------------
//
//					ANTENNAS & BUSY TONE
//
//-----------------------------------------------------------


//Changes the radio power for tranmissions
void change_tx_power(double power , int stream){
	//id
	int			tx_id , chan_id , sub_chan_id;
	int			num_chan;
	int			i;
	
	//gets the id of the tansmitter + channel attributes
	tx_id 		= op_topo_assoc(op_id_self() , OPC_TOPO_ASSOC_OUT , OPC_OBJTYPE_RATX , stream);
	op_ima_obj_attr_get (tx_id, "channel", &chan_id);

	//Sets the channel attributes
	//NB: I have normally one single channel, but .....
	num_chan = op_topo_child_count(chan_id, OPC_OBJTYPE_RATXCH);		
	for(i=0 ; i<num_chan ; i++){
		sub_chan_id = op_topo_child (chan_id, OPC_OBJTYPE_RATXCH, 0);
		op_ima_obj_attr_set (sub_chan_id, "power", power);
	}

	debug_print(LOW , DEBUG_RADIO , "New power transmission %f\n", power);
	debug_print(LOW , DEBUG_SEND , "New power transmission %f\n", power);
}


//returns true if we are focused on the main channel
Boolean is_main_freq_active(int stream){
	int			radio_id , chan_id , sub_chan_id;
	double		frequency;
	

	//Gets only transmitter value (the receiver value is identical, or there is a big trouble !)
	radio_id 		= op_topo_assoc(op_id_self() , OPC_TOPO_ASSOC_OUT , OPC_OBJTYPE_RATX , stream);
	op_ima_obj_attr_get (radio_id, "channel", &chan_id);
	sub_chan_id = op_topo_child (chan_id, OPC_OBJTYPE_RATXCH, 0);
	op_ima_obj_attr_get (sub_chan_id, "min frequency", 	&frequency);
	
	return (frequency == my_main_frequency);
}

//Changes the radio power for tranmissions
void change_tx_rx_freq(double frequency , double bandwidth , int stream){
	int		radio_id , chan_id , sub_chan_id;
	double	old_freq;
	
	//------------ TRANSMISSION -----------
	radio_id 		= op_topo_assoc(op_id_self() , OPC_TOPO_ASSOC_OUT , OPC_OBJTYPE_RATX , stream);
	op_ima_obj_attr_get (radio_id, "channel", &chan_id);
	sub_chan_id = op_topo_child (chan_id, OPC_OBJTYPE_RATXCH, 0);
	op_ima_obj_attr_get (sub_chan_id, "min frequency", 	&old_freq);
	op_ima_obj_attr_set (sub_chan_id, "bandwidth", 		bandwidth);
	op_ima_obj_attr_set (sub_chan_id, "min frequency", 	frequency);
	
	

	//------------ RECEPTION -----------
	radio_id 		= op_topo_assoc(op_id_self() , OPC_TOPO_ASSOC_IN , OPC_OBJTYPE_RARX , stream);
	op_ima_obj_attr_get (radio_id, "channel", &chan_id);
	sub_chan_id = op_topo_child (chan_id, OPC_OBJTYPE_RARXCH, 0);
	op_ima_obj_attr_set (sub_chan_id, "bandwidth", 		bandwidth);
	op_ima_obj_attr_set (sub_chan_id, "min frequency", 	frequency);

	if (frequency != old_freq){
		debug_print(LOW , DEBUG_RADIO , "new bandwidth %f and frequency %f (main %f/%f)\n", bandwidth , frequency , my_main_bandwidth , my_main_frequency);
		debug_print(LOW , DEBUG_NODE , "new bandwidth %f and frequency %f (main %f/%f)\n", bandwidth , frequency , my_main_bandwidth , my_main_frequency);
	}
}



//Changes the radio power for tranmissions
void change_antenna_direction(int stream , int branch){
	int			antenna_id;
	int			tx_id;
	double		theta;
	double		x , y;
	
	//Transmitter id (I am connected via a stream to it)
	tx_id 		= op_topo_assoc(op_id_self() , OPC_TOPO_ASSOC_OUT , OPC_OBJTYPE_RATX , stream);
	
	//One single antenna per transmitter
	antenna_id	= op_topo_assoc(tx_id , OPC_TOPO_ASSOC_OUT , OPC_OBJTYPE_ANT , 0);
	
	
	//Direction
	theta = 2 * PI * branch / MAX_NB_BRANCHES;
	x = cos(theta);
	y = sin(theta);

	//and point it
	op_ima_obj_attr_set (antenna_id, "target latitude", 	x);
	op_ima_obj_attr_set (antenna_id, "target longitude", 	y);

	debug_print(LOW , DEBUG_RADIO , "New direction for the antenna: %f\n", theta);
}




//sends a packet to the busy tone radio (to maintain a busy state)
void maintain_busy_tone(double time){
	Packet	*pkptr;

	//Create a packet with the required size (in bits)
	pkptr = op_pk_create(busy_tone_speed * time);
	debug_print(LOW , DEBUG_RADIO , "new busy tone for %fs (packet size %f bits)\n", time , op_pk_total_size_get(pkptr));

	//transmission
	op_pk_send(pkptr , STREAM_TO_BUSY_TONE);	
}












//-----------------------------------------------------------
//
//			 NAV (Network Allocation Vector)
//
//-----------------------------------------------------------


//Prints the list of current NAV
void print_nav_list(int debug_type){
	int			i;
	nav_struct	*ptr;

	
	debug_print(LOW , debug_type , "------------------------------------------------------------------------------------------------------------\n");
	debug_print(LOW , debug_type , "									NAV LIST (%f)\n", op_sim_time());
	debug_print(LOW , debug_type , "------------------------------------------------------------------------------------------------------------\n");
	debug_print(LOW , debug_type , "\n");
	
	debug_print(LOW , debug_type , "    ADDR	|	FREQ		|	TIMEOUT\n");
	
	for (i=0 ; i< op_prg_list_size(my_nav_list) ; i++){
		ptr = op_prg_list_access(my_nav_list , i);
		
		debug_print(LOW , debug_type , "%8d	|	%3f	|	%f\n", ptr->address , ptr->frequency , ptr->timeout);
	}
	
}

//Deletes obsolete nav
void delete_timeouted_nav(void * arg, int code){
	int				i;
	nav_struct		*ptr;
	double			older_entry = 0;
	
	
	//Walks in the list
	for(i= op_prg_list_size(my_nav_list)-1 ; i>= 0 ; i--){
		ptr = op_prg_list_access(my_nav_list , i);
		
		//Timeouted entry !
		if (ptr->timeout <= op_sim_time()){
			debug_print(LOW , DEBUG_BACKOFF , "NAV from %d deleted\n", ptr->address);
			
			ptr = op_prg_list_remove(my_nav_list , i);
			op_prg_mem_free(ptr);
		}
		
		//updates the time for the older entry
		else if ((older_entry > ptr->timeout) || (older_entry == 0))
			older_entry = ptr->timeout;
	}
	
	
	//Next verification
	if (older_entry != 0)
		op_intrpt_schedule_call(older_entry , 0 , delete_timeouted_nav , NULL);
}


//adds a medium reservation
void add_nav(int src, double duration , double frequency){
	nav_struct	*ptr;
	int			i;
	
	//Deletes timeouts
	if (op_prg_list_size(my_nav_list) == 0)
		op_intrpt_schedule_call(op_sim_time() + duration , 0 , delete_timeouted_nav , NULL);
	
	//Updates the entry if one already exists
	for(i=0 ; i < op_prg_list_size(my_nav_list) ; i++){
		ptr = op_prg_list_access(my_nav_list , i);
		
		if ((ptr->address == src) && (ptr->frequency == frequency)){
			ptr->timeout = op_sim_time() + duration;
	
			debug_print(LOW , DEBUG_BACKOFF , "NAV updated, src %d, duration %fus\n", src , duration * 1E6);			
			return;
		}
	}
	
	//Adds one new element
	ptr = op_prg_mem_alloc(sizeof(nav_struct));
	ptr->address 	= src;
	ptr->frequency	= frequency;
	ptr->timeout 	= op_sim_time() + duration;
	op_prg_list_insert(my_nav_list, ptr , OPC_LISTPOS_TAIL);
	
	debug_print(LOW , DEBUG_BACKOFF , "NAV added, src %d, duration %fus\n", src , duration * 1E6);
}



//updates the nav
void update_nav_time(double transmission_time, int source , double frequency , int pk_size){
		
	//No real reservation
	if (transmission_time <= 0){
		debug_print(LOW , DEBUG_BACKOFF , "NAV ! UPDATED -> src %d, duration %fus, pk_size %d\n", source , transmission_time * 1E6 , pk_size);
		return;
	}

	//Stores the NAV duration
	add_nav(source , transmission_time , frequency);
					
	//Schedules the end of the NAV	
	op_intrpt_schedule_self(op_sim_time() + transmission_time , NAV_END_CODE);
	
	
	debug_print(LOW , DEBUG_BACKOFF , "NAV -> src %d, duration %fus, pk_size %d\n", source , transmission_time * 1E6 , pk_size);
}


//Returns TRUE if a reservation was registered for another frequency
Boolean is_nav_for_other_freq(){
	int			i;
	nav_struct	*ptr;
	
	for(i=0 ; i < op_prg_list_size(my_nav_list) ; i++){
		ptr = op_prg_list_access(my_nav_list , i);
		
		if ((ptr->timeout > op_sim_time()) && (ptr->frequency != my_main_frequency))
			return(OPC_TRUE);
	}
	
	return(OPC_FALSE);
}

//Returns the longest NAV for the main frequency
double get_nav_main_freq(){
	int			i;
	nav_struct	*ptr;
	double		timeout = 0;
	
	for(i=0 ; i < op_prg_list_size(my_nav_list) ; i++){
		ptr = op_prg_list_access(my_nav_list , i);
		
		if ((ptr->timeout > timeout) && (ptr->frequency == my_main_frequency))
			timeout = ptr->timeout;
	}
	
	return(timeout);
}


//Is the medium free to reply to addr ? (i.e. no other node has already reserved this frequency ?
Boolean is_reply_possible(int addr , double frequency){
	int			i;
	nav_struct	*ptr;
	
	for(i=0 ; i < op_prg_list_size(my_nav_list) ; i++){
		ptr = op_prg_list_access(my_nav_list , i);
		
		if ((ptr->timeout > op_sim_time()) && (ptr->frequency == frequency) && (ptr->address != addr))
			return(OPC_FALSE);
	}
	
	return(OPC_TRUE);
}


//The destination is available for reception (no NAV for any frequency)
Boolean is_destination_available(int destination){
	int			i;
	nav_struct	*ptr;
	
	for(i=0 ; i < op_prg_list_size(my_nav_list) ; i++){
		ptr = op_prg_list_access(my_nav_list , i);
		
		if ((ptr->timeout > op_sim_time()) && (ptr->address == destination))
			return(OPC_FALSE);
	}
	return(OPC_TRUE);
}





//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------																										  ---------------------------
//---------------------------												QUEUE MANAGEMENT										  ---------------------------
//---------------------------																										  ---------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------








//-----------------------------------------------------------
//
//					DUPLICATA DETECTION
//
//-----------------------------------------------------------


//Deletes the frame_id which became obsolete
void del_timeouted_frame_id(Vartype* tot , int code){
	int						i;
	id_timeout_struct		*ptr;
		
	//Walks in the list
	for(i= op_prg_list_size(my_frame_id_seen)-1 ; i>= 0 ; i--){
		ptr = op_prg_list_access(my_frame_id_seen , i);
		
		//Timeouted entry !
		if (ptr->timeout <= op_sim_time()){
			ptr = op_prg_list_remove(my_frame_id_seen , i);
			op_prg_mem_free(ptr);
		}
	}
	
	
	//Next verification
	if (op_prg_list_size(my_frame_id_seen) != 0)
		op_intrpt_schedule_call(op_sim_time() + TIMEOUT_FRAME_ID , FRAME_ID_TIMEOUT_CODE , del_timeouted_frame_id , NULL);
	
}


//Is thie frame_id already in the list ?
Boolean is_frame_id_seen(int frame_id){
	id_timeout_struct		*ptr;
	int						i;
	
	return(OPC_FALSE);
	
	for(i=0 ; i < op_prg_list_size(my_frame_id_seen) ; i++){
		ptr = op_prg_list_access(my_frame_id_seen , i);
		
		if (ptr->id == frame_id)
			return(OPC_TRUE);
	}
	
	return(OPC_FALSE);
}



//adds the frame_id as already seen
void add_frame_id_seen(int frame_id){
	id_timeout_struct		*ptr;
	
	return;
	
	//the frame id is already in the list
	if (is_frame_id_seen(frame_id))
		return;
	
	//Empty list -> timeouted verificaiton
	if (op_prg_list_size(my_frame_id_seen) == 0)
		op_intrpt_schedule_call(op_sim_time() + 1.0 , FRAME_ID_TIMEOUT_CODE , del_timeouted_frame_id , NULL);
	
	//New entry
	ptr = op_prg_mem_alloc(sizeof(id_timeout_struct));
	ptr->id			= frame_id;
	ptr->timeout	= op_sim_time() + TIMEOUT_FRAME_ID;
	op_prg_list_insert(my_frame_id_seen , ptr , OPC_LISTPOS_TAIL);
}








//-----------------------------------------------------------
//
//					UNICAST FRAME BUFFER
//
//-----------------------------------------------------------

//increments the number of retransmissions for the corresponding frame_id
void increment_nb_rety_unicast_frame_buffer(int id){
	//Frame buffer
	frame_struct	*ptr;
	//info
	char			msg[500];
	int				i;
	
	for(i=op_prg_list_size(unicast_frame_buffer)-1 ; i>=0 ; i--){
		ptr = op_prg_list_access(unicast_frame_buffer , i);

		if (ptr->frame_id == id){
		
			ptr->nb_retry++;
			
			if (ptr->nb_retry == MEDIUM_NB_RETRY){
				ptr->time_transmission_min = op_sim_time() + TIME_MAX_PRIVILEGED * 2;
			}
			
			//Verify that we dont have too many retransmissions
			if (ptr->nb_retry >= MAX_NB_RETRY){
				debug_print(LOW, DEBUG_SEND , "the unicast frame to %d was not acknowledged and we have too many retransmissions -> DELETED (type %s, id %d)\n",  ptr->destination , pk_type_to_str(ptr->type , msg) , ptr->frame_id);
				debug_print(LOW, DEBUG_TIMEOUT , "the unicast frame to %d was not acknowledged and we have too many retransmissions -> DELETED (type %s, id %d)\n",  ptr->destination , pk_type_to_str(ptr->type , msg) , ptr->frame_id);
				
				
				ptr = op_prg_list_remove(unicast_frame_buffer , i);
				if (ptr->payload != NULL)
					op_pk_destroy(ptr->payload);
				op_prg_mem_free(ptr);
			}
			else
				debug_print(LOW, DEBUG_TIMEOUT , "the unicast frame to %d was not acknowledged -> RETRANSMISSION (nb retry %d type %s, id %d)\n",  ptr->destination , ptr->nb_retry , pk_type_to_str(ptr->type , msg) , ptr->frame_id);
		}
	}
}


//Deletes the data which were timeouted
void del_timeouted_unicast(void *arg, int code){
	frame_struct	*ptr;
	int				i;
	char			msg[500];
	double			older_entry = -1;
	
	for(i=op_prg_list_size(unicast_frame_buffer) - 1 ; i >=0 ; i--){
		ptr = op_prg_list_access(unicast_frame_buffer , i);
		
		//timeouted entry
		if (ptr->time_added <= op_sim_time() - TIMEOUT_DATA_FRAME){
			
			debug_print(LOW, DEBUG_TIMEOUT , "the unicast frame to %d was DELETED after its timeout (nb_retry %d, type %s, id %d)\n",  ptr->destination , ptr->nb_retry , pk_type_to_str(ptr->type , msg) , ptr->frame_id);
			debug_print(LOW, DEBUG_NODE , "the unicast frame to %d was DELETED after its timeout (nb_retry %d, type %s, id %d)\n",  ptr->destination , ptr->nb_retry , pk_type_to_str(ptr->type , msg) , ptr->frame_id);
		
			ptr = op_prg_list_remove(unicast_frame_buffer , i);
			if (ptr->payload != NULL)
				op_pk_destroy(ptr->payload);
			op_prg_mem_free(ptr);
		}
		
		//next verification
		else if ((older_entry > ptr->time_added + TIMEOUT_DATA_FRAME) || (older_entry == -1))
			older_entry = ptr->time_added + TIMEOUT_DATA_FRAME;
	}

	if (older_entry != -1)
		op_intrpt_schedule_call(op_sim_time() + older_entry , 0 , del_timeouted_unicast , NULL);
}


//adds a (data) frame to send
void add_in_unicast_frame_buffer(frame_struct frame_tmp , int position){
	//Frame buffer
	frame_struct	*ptr;
		
	//Error
	if ((frame_tmp.type != DATA_UNICAST_PK_TYPE) && (frame_tmp.type != DATA_MULTICAST_PK_TYPE) && (frame_tmp.type != HELLO_PK_TYPE))
		op_sim_end("It is not possible to add a control frame" , "in the buffer of data unicast frame to send", "" , "");
	
	
	//Timeouts
	if (op_prg_list_size(unicast_frame_buffer) == 0)
		op_intrpt_schedule_call(op_sim_time() + TIMEOUT_DATA_FRAME , 0 , del_timeouted_unicast , NULL);

	
	//Memory allocation + insertion in the buffer
	ptr = op_prg_mem_alloc(sizeof(frame_struct));
	*ptr  = frame_tmp;	
	op_prg_list_insert(unicast_frame_buffer , ptr , position);
	
	debug_print(MEDIUM , DEBUG_SEND , "a unicast frame to %d (id %d) was added in the buffer\n", frame_tmp.destination , frame_tmp.frame_id);

}


//returns the nb of data frames to send
int get_unicast_frame_buffer_size(){
	frame_struct	*ptr;
	int				i;
	int				nb = 0;

	for(i=op_prg_list_size(unicast_frame_buffer)-1 ; i>=0 ; i--){
		ptr = op_prg_list_access(unicast_frame_buffer , i);

		if (ptr->time_transmission_min < op_sim_time())
			nb++;
	}
	
	return(nb);
}


//Must data frame be transmitted ?
Boolean is_unicast_frame_buffer_empty(){
	return(get_unicast_frame_buffer_size() == 0);
}


//returns the first data frame to send
frame_struct *get_unicast_frame_buffer(int pos){
	frame_struct	*ptr;
	int				i;
	int				nb = 0;

	for(i=op_prg_list_size(unicast_frame_buffer)-1 ; i>=0 ; i--){
		ptr = op_prg_list_access(unicast_frame_buffer , i);

		if (ptr->time_transmission_min < op_sim_time()){
			if (pos == nb)
				return(ptr);
			nb++;
		}
	}
	
	return(NULL);
}


//Prints the content of the data frame buffer
void print_unicast_frame_buffer(int debug_type){
	//Frame buffer
	frame_struct	*ptr;
	//info
	char			msg[500];
	int				i;

	debug_print(LOW , debug_type , "-------------------------------------------------------------\n");
	debug_print(LOW , debug_type , "			Unicast Frame Buffer of %d (size %d)\n" , my_address , op_prg_list_size(unicast_frame_buffer));
	debug_print(LOW , debug_type , "-------------------------------------------------------------\n");
	debug_print(LOW , debug_type , "	DEST	|	TYPE		| 	ID		|	NB_RETRY	|	MIN_TRANSMISSION\n");


	
	for(i=op_prg_list_size(unicast_frame_buffer)-1 ; i>=0 ; i--){
		ptr = op_prg_list_access(unicast_frame_buffer , i);

		debug_print(LOW , debug_type , "%5d	|	%s	|	%d		|	%d		|	%f\n", ptr->destination ,  pk_type_to_str(ptr->type , msg) , ptr->frame_id , ptr->nb_retry , ptr->time_transmission_min);
	}
	
}




//-----------------------------------------------------------
//
//					MULTICAST FRAME BUFFER
//
//-----------------------------------------------------------


//Deletes the multicast frames which were timeouted
void del_timeouted_multicast(void *arg, int code){
	frame_struct	*ptr;
	int				i;
	char			msg[500];
	double			older_entry = -1;
	
	for(i=op_prg_list_size(multicast_frame_buffer) - 1 ; i >=0 ; i--){
		ptr = op_prg_list_access(multicast_frame_buffer , i);
		
		//timeouted entry
		if (ptr->time_added <= op_sim_time() - TIMEOUT_DATA_FRAME){
			
			debug_print(LOW, DEBUG_TIMEOUT , "the multicast frame to %d was DELETED after its timeout (nb_retry %d, type %s, id %d)\n",  ptr->destination , ptr->nb_retry , pk_type_to_str(ptr->type , msg) , ptr->frame_id);
			debug_print(LOW, DEBUG_NODE , "the multicast frame to %d was DELETED after its timeout (nb_retry %d, type %s, id %d)\n",  ptr->destination , ptr->nb_retry , pk_type_to_str(ptr->type , msg) , ptr->frame_id);
		
			ptr = op_prg_list_remove(multicast_frame_buffer , i);
			if (ptr->payload != NULL)
				op_pk_destroy(ptr->payload);
			op_prg_mem_free(ptr);
		}
		
		//next verification
		else if ((older_entry > ptr->time_added + TIMEOUT_DATA_FRAME) || (older_entry == -1))
			older_entry = ptr->time_added + TIMEOUT_DATA_FRAME;
	}

	if (older_entry != -1)
		op_intrpt_schedule_call(op_sim_time() + older_entry , 0 , del_timeouted_multicast , NULL);
}


//adds a (data) frame to send
void add_in_multicast_frame_buffer(frame_struct frame_tmp , int position){
	//Frame buffer
	frame_struct	*ptr;
		
	//Error
	if ((frame_tmp.type != DATA_MULTICAST_PK_TYPE) && (frame_tmp.type != HELLO_PK_TYPE))
		op_sim_end("It is not possible to add a control frame" , "in the buffer of data frame to send", "" , "");
	
	
	//Timeouts
	if (op_prg_list_size(multicast_frame_buffer) == 0)
		op_intrpt_schedule_call(op_sim_time() + TIMEOUT_DATA_FRAME , 0 , del_timeouted_multicast , NULL);

	
	//Memory allocation + insertion in the buffer
	ptr = op_prg_mem_alloc(sizeof(frame_struct));
	*ptr  = frame_tmp;	
	op_prg_list_insert(multicast_frame_buffer , ptr , position);
	
	debug_print(MEDIUM , DEBUG_SEND , "a multicast frame to %d (id %d) was added in the buffer\n", frame_tmp.destination , frame_tmp.frame_id);

}


//returns the nb of data frames to send
int get_multicast_frame_buffer_size(){
	frame_struct	*ptr;
	int				i;
	int				nb = 0;

	for(i=op_prg_list_size(multicast_frame_buffer)-1 ; i>=0 ; i--){
		ptr = op_prg_list_access(multicast_frame_buffer , i);

		debug_print(LOW , DEBUG_STATE , "->%f > %f\n", ptr->time_transmission_min , op_sim_time());
		if (ptr->time_transmission_min < op_sim_time())
			nb++;
	}
	
	return(nb);
}


//Must data frame be transmitted ?
Boolean is_multicast_frame_buffer_empty(){
	return(get_multicast_frame_buffer_size() == 0);
}


//returns the first data frame to send
frame_struct *get_multicast_frame_buffer(int pos){
	frame_struct	*ptr;
	int				i;
	int				nb = 0;

	for(i=op_prg_list_size(multicast_frame_buffer)-1 ; i>=0 ; i--){
		ptr = op_prg_list_access(multicast_frame_buffer , i);

		if (ptr->time_transmission_min < op_sim_time()){
			if (pos == nb)
				return(ptr);
			nb++;
		}
	}
	
	return(NULL);
}



//Prints the content of the data frame buffer
void print_multicast_frame_buffer(int debug_type){
	//Frame buffer
	frame_struct	*ptr;
	//info
	char			msg[500];
	int				i;

	debug_print(LOW , debug_type , "-------------------------------------------------------------\n");
	debug_print(LOW , debug_type , "			Multicast Frame Buffer of %d (size %d)\n" , my_address , op_prg_list_size(multicast_frame_buffer));
	debug_print(LOW , debug_type , "-------------------------------------------------------------\n");
	debug_print(LOW , debug_type , "	DEST	|	TYPE	| 	ID		|	NB_RETRY	|	MIN_TRANSMISSION\n");


	
	for(i=op_prg_list_size(multicast_frame_buffer)-1 ; i>=0 ; i--){
		ptr = op_prg_list_access(multicast_frame_buffer , i);

		debug_print(LOW , debug_type , "%5d	|	%s	|	%d		|	%d		|	%f\n", ptr->destination ,  pk_type_to_str(ptr->type , msg) , ptr->frame_id , ptr->nb_retry , ptr->time_transmission_min);
	}
	
}



//-----------------------------------------------------------
//
//					MULTICAST & UNICAST FRAME BUFFER
//
//-----------------------------------------------------------


//Rteurns the nb of possible frames
int get_frame_buffer_size(){
	frame_struct	*ptr;
	frame_struct	*ptr_uni;
	frame_struct	*ptr_multi;
	int				i , j;
	int				nb = 0;
	
	//initialization
	i = 0;
	//Multicast -> only when all my neighbors can receive it
	if (IS_BROADCAST_FORBIDDEN)
		j = op_prg_list_size(multicast_frame_buffer);
	else
		j = 0;

	while((i < op_prg_list_size(unicast_frame_buffer)) || (j < op_prg_list_size(multicast_frame_buffer))){
	
		//no more unicast frame
		if (i == op_prg_list_size(unicast_frame_buffer)){
			ptr = op_prg_list_access(multicast_frame_buffer , j);
			j++;
		}
	
	
		//no more multicast frame
		else if (j == op_prg_list_size(multicast_frame_buffer)){
			ptr = op_prg_list_access(unicast_frame_buffer , i);
			i++;
		}
	
	
		else{
			ptr_uni 	= op_prg_list_access(unicast_frame_buffer , i);
			ptr_multi	= op_prg_list_access(multicast_frame_buffer , j);
			
			//next element for the next time
			if (ptr_uni->time_added < ptr_multi->time_added){
				ptr = ptr_uni;
				i++;
			}
			else{
				ptr = ptr_multi;
				j++;
			}
		}
		
		//result
		if ((ptr->time_transmission_min < op_sim_time()) && (is_destination_available(ptr->destination)))
			nb++;
	}
	return(nb);
}

//Must any multicast / anycast data farme be sent ?
Boolean is_frame_buffer_empty(){
	return((get_frame_buffer_size() == 0));
}


//returns the first data frame to send
frame_struct *get_frame_buffer(int pos){
	frame_struct	*ptr;
	frame_struct	*ptr_uni;
	frame_struct	*ptr_multi;
	int				i , j;
	int				nb = 0;
	
	//initialization
	i = 0;
	//Multicast -> only when all my neighbors can receive it
	if (IS_BROADCAST_FORBIDDEN)
		j = op_prg_list_size(multicast_frame_buffer);
	else
		j = 0;

	while((i < op_prg_list_size(unicast_frame_buffer)) || (j < op_prg_list_size(multicast_frame_buffer))){
	
		//no more unicast frame
		if (i == op_prg_list_size(unicast_frame_buffer)){
			ptr = op_prg_list_access(multicast_frame_buffer , j);
			j++;
		}
	
	
		//no more multicast frame
		else if (j == op_prg_list_size(multicast_frame_buffer)){
			ptr = op_prg_list_access(unicast_frame_buffer , i);
			i++;
		}
	
	
		else{
			ptr_uni 	= op_prg_list_access(unicast_frame_buffer , i);
			ptr_multi	= op_prg_list_access(multicast_frame_buffer , j);
			
			//next element for the next time
			if (ptr_uni->time_added < ptr_multi->time_added){
				ptr = ptr_uni;
				i++;
			}
			else{
				ptr = ptr_multi;
				j++;
			}
		}
		
		//result
		if ((ptr->time_transmission_min < op_sim_time()) && (is_destination_available(ptr->destination)) && (pos == nb))
			return(ptr);
		else if ((ptr->time_transmission_min < op_sim_time()) && (is_destination_available(ptr->destination)))
			nb++;
	}
	return(NULL);
}



//deletes the multicast or unicast frames with the frame_id 'id'
void del_frame_buffer_with_id(int id , Boolean acknowledged){
	frame_struct	*ptr;
	char			msg[500];
	int				i;

	for(i=op_prg_list_size(unicast_frame_buffer)-1 ; i>=0 ; i--){
		ptr = op_prg_list_access(unicast_frame_buffer , i);

		if (ptr->frame_id == id){
			debug_print(LOW, DEBUG_TIMEOUT , "the unicast frame to %d was received (ack required %d, nb_retry %d, type %s, id %d)\n",  ptr->destination , acknowledged , ptr->nb_retry , pk_type_to_str(ptr->type , msg) , ptr->frame_id);
		
			ptr = op_prg_list_remove(unicast_frame_buffer , i);
			if (ptr->payload != NULL)
				op_pk_destroy(ptr->payload);
			op_prg_mem_free(ptr);
		}
	}
	for(i=op_prg_list_size(multicast_frame_buffer)-1 ; i>=0 ; i--){
		ptr = op_prg_list_access(multicast_frame_buffer , i);

		if (ptr->frame_id == id){
			debug_print(LOW, DEBUG_TIMEOUT , "the multicast frame to %d was received (ack required %d, nb_retry %d, type %s, id %d)\n",  ptr->destination , acknowledged , ptr->nb_retry , pk_type_to_str(ptr->type , msg) , ptr->frame_id);
		
			ptr = op_prg_list_remove(multicast_frame_buffer , i);
			if (ptr->payload != NULL)
				op_pk_destroy(ptr->payload);
			op_prg_mem_free(ptr);
		}
	}
	
}



//returns TRUE if the frame_id is present in the buffer
Boolean is_in_frame_buffer(int frame_id){
	frame_struct	*ptr;
	int				i;
	
	for(i=op_prg_list_size(multicast_frame_buffer)-1 ; i>=0 ; i--){
		ptr = op_prg_list_access(multicast_frame_buffer , i);

		if (ptr->frame_id == frame_id)
			return(OPC_TRUE);
	}
	for(i=op_prg_list_size(unicast_frame_buffer)-1 ; i>=0 ; i--){
		ptr = op_prg_list_access(unicast_frame_buffer , i);

		if (ptr->frame_id == frame_id)
			return(OPC_TRUE);
	}
	
	return(OPC_FALSE);
}


//Debug
void print_frame_buffer(int debug_type){
	print_multicast_frame_buffer(debug_type);
	print_unicast_frame_buffer(debug_type);

}






//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------																										  ---------------------------
//---------------------------												ROUTING													  ---------------------------
//---------------------------																										  ---------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------




//-----------------------------------------------------------
//
//						NEXT HOP
//
//-----------------------------------------------------------


//Comares stab / prio (stab is more important
Boolean compare_stab_prio(compar_struct val , short stab, short prio){

	debug_print(MAX , DEBUG_NODE , "stab prio %d %d %d %d\n", val.stab , val.prio , stab , prio);

	if (val.stab > stab + STAB_STEP)
		return(OPC_FALSE);
	else if (val.stab < stab - STAB_STEP)
		return(OPC_TRUE);
	else if (val.prio < prio)
		return(OPC_TRUE);
	else
		return(OPC_FALSE);

}

//returns the next_hop to the sink
int get_next_hop(){
	int				i;
	neigh_struct	*neigh_ptr;
	compar_struct 	next;
	
	//Initialization
	next.prio = 0;
	next.addr = BROADCAST;
	next.stab = 0;
	switch (MAC_ROUTING){
		case ROUTING_BORDER:
		
			//Walks in the list
			for(i= op_prg_list_size(my_neighborhood_table)-1 ; i>= 0 ; i--){
				neigh_ptr = op_prg_list_access(my_neighborhood_table , i);
		
				//Border node
				if ((neigh_ptr->dist_border == 0) && (neigh_ptr->dist_sink < my_dist_sink) && (compare_stab_prio(next , compute_stability(neigh_ptr->stability) , 4))){
					next.addr	= neigh_ptr->address;
					next.prio	= 5;
					next.stab	= compute_stability(neigh_ptr->stability);
				}
				   
				//Other node -> to the borders	   
				if ((neigh_ptr->dist_border < my_dist_border) && (neigh_ptr->dist_sink < my_dist_sink) && (compare_stab_prio(next , compute_stability(neigh_ptr->stability) , 4))){
					next.addr	= neigh_ptr->address;
					next.prio	= 4;
					next.stab	= compute_stability(neigh_ptr->stability);
				}
		
				//Other node -> to the borders (there must be an improvement toward the sink) 
				if ((neigh_ptr->dist_sink <= my_dist_sink) && (neigh_ptr->dist_border < my_dist_border) && (!is_border_node) && (compare_stab_prio(next , compute_stability(neigh_ptr->stability) , 3))){
					next.addr	= neigh_ptr->address;
					next.prio	= 3;
					next.stab	= compute_stability(neigh_ptr->stability);
					}
		
				//Other node -> to the sink if no border node		   
				if ((neigh_ptr->dist_sink < my_dist_sink) && (compare_stab_prio(next , compute_stability(neigh_ptr->stability) , 2))){
					next.addr	= neigh_ptr->address;
					next.prio	= 2;
					next.stab	= compute_stability(neigh_ptr->stability);
				}
				debug_print(LOW , DEBUG_NODE , "	->%d -> next hop %d (%d %d)\n",  neigh_ptr->address , next.addr , next.prio , next.stab);
			}
		break;
		case ROUTING_SHORT:
			//Walks in the list
			for(i= op_prg_list_size(my_neighborhood_table)-1 ; i>= 0 ; i--){
				neigh_ptr = op_prg_list_access(my_neighborhood_table , i);
		
				//Shortest route to the sink
				if ((neigh_ptr->dist_sink < my_dist_sink) && (compare_stab_prio(next , compute_stability(neigh_ptr->stability) , 2))){
					next.addr	= neigh_ptr->address;
					next.prio	= 2;
					next.stab	= compute_stability(neigh_ptr->stability);
				}
				
				debug_print(MAX , DEBUG_NODE , "	->%d -> next hop %d (%d %d)\n",  neigh_ptr->address , next.addr , next.prio , next.stab);
			}
		break;
		case ROUTING_NO:
			op_sim_end("I am not allowed to route packets" , "But I try to find one next hop" , "Please correct the corresponding bug" , "");
		break;
		default:
			op_sim_end("Unknown routing type" , "" , "" , "");
		break;
	}

	return(next.addr);

}





//-----------------------------------------------------------
//
//					IS BORDER status
//
//-----------------------------------------------------------

//Coputes the euclidian distance
double get_dist(double x1 , double y1 , double x2 , double y2){
	double x , y;
	
	x = pow (x1 - x2 , 2);
	y = pow (y1 - y2 , 2);

	return(sqrt(x + y));
}

//elect border nodes in a centralized manner
void compute_border_nodes_status(){
	int			i , j , k;
	//Topology identification
	int			process_id;
	int			node_id;
	//values
	double		x , y;
	int			addr;
	int			*pvalue;
	//tmp vars
	bn_struct	*bn_elem;
	//list of positions
	List		*pos_list;
	pos_struct	*elem;
	//Last node in the branch
	pos_struct	last_node , sink_node , best_node;
	double		direction_x , direction_y;
	double		current_dist , last_dist , best_dist;
	double		range;
	
	
	if (!is_sink)
		op_sim_end("Only the sink is allowed to compute" , "the list of border nodes, event in this centralized algorithm" , "" , "");
	
	//initialization
	pos_list = op_prg_list_create();
	
	//gets all the positions
	for (i=0 ; i < op_topo_object_count(OPC_OBJTYPE_NDMOB) ; i++){
		node_id = op_topo_object(OPC_OBJTYPE_NDMOB , i);
		op_ima_obj_attr_get(node_id , "x position" , &x);
		op_ima_obj_attr_get(node_id , "y position" , &y);
		
		//tries to get the attribute 'address' from all the processes
		addr = 0;
		for(j=0 ; (j < op_topo_child_count(node_id , OPC_OBJTYPE_PROC))  && (addr == 0); j++){
			process_id = op_topo_child(node_id , OPC_OBJTYPE_PROC , j);
			pvalue = op_ima_obj_svar_get(process_id , "my_address");
			if (pvalue != NULL)
				addr = *pvalue; 
		}
		
		elem = op_prg_mem_alloc(sizeof(pos_struct));
		elem->x 		= x;
		elem->y 		= y;
		elem->address	= addr;
		op_prg_list_insert(pos_list , elem , OPC_LISTPOS_TAIL);
		
		//I am the sink
		if (elem->address == my_address)
			sink_node = *elem;
	}
	
	debug_print(LOW , DEBUG_CONTROL , "List of the border nodes (elected in a centralized manner):\n");
	printf("bn list from %d\n", my_address);
	
	
	//Expands each branch
	for(i=0 ; i < MAX_NB_BRANCHES ; i++){
		printf("NEW BRANCH\n");
	
		//O^th node : the sink
		last_node = sink_node;
		
		//The direction of the branch (one x and y step for this direction)
		direction_x = cos(i * 2 * PI / MAX_NB_BRANCHES) * RADIO_RANGE;
		direction_y = sin(i * 2 * PI / MAX_NB_BRANCHES) * RADIO_RANGE;
		
		//Adds one node in the branch (if possible)
		for(j=1; j < MAX_BRANCH_LENGTH+1 ; j++){			
			//The nodes to add
			best_node 			= last_node;
			best_dist			= OPC_DBL_INFINITY;
			
			//Searches if we have a better candidate
			for(k=0 ; k < op_prg_list_size(pos_list) ; k++){
				elem = op_prg_list_access(pos_list , k);
				
				range 			= get_dist(elem->x 		, elem->y 		, last_node.x 					, last_node.y);
				current_dist 	= get_dist(elem->x 		, elem->y 		, sink_node.x + direction_x * j , sink_node.y + direction_y * j);
				last_dist 		= get_dist(last_node.x 	, last_node.y 	, sink_node.x + direction_x * j , sink_node.y + direction_y * j);
				
				
				//This node and the last node are connected
				if (range <= RADIO_RANGE)
									
					//distance to the k° point of the branch (the goal in an optimal objective)
					if (current_dist < best_dist)
						
						//There exists an improvement !
						//In other words, elem is nearer from the goal than the last node of the branch 
						//It could not be the case if we have a 'hole' in the network
						if (last_dist > current_dist){
							best_dist = current_dist;
							best_node = *elem;
						}
			}
			
			//If the candidate is not null -> add it
			if (best_node.address != last_node.address){
				debug_print(LOW , DEBUG_CONTROL , "%d is a bn\n", best_node.address);
				printf("%d bn (last %d)\n", best_node.address , last_node.address);
				
				bn_elem				= op_prg_mem_alloc(sizeof(bn_struct));
				bn_elem->address 	= best_node.address;
				bn_elem->parent		= last_node.address;
				op_prg_list_insert(global_border_nodes_list , bn_elem , OPC_LISTPOS_TAIL);
				
				last_node 			= best_node;
			}
		}
	}
	
	
	
	//Memory free
	while(op_prg_list_size(pos_list) != 0){
		elem = op_prg_list_remove(pos_list , OPC_LISTPOS_TAIL);
		op_prg_mem_free(elem);
	}
	op_prg_list_free(pos_list);
		
}


//Is this node a border node ?
Boolean	get_is_border_node(){
	int				i , j;
	neigh_struct	*neigh_ptr;
	int				*addr_ptr;
	int				max_stab_in_neigh=0;
	int				stab_my_parent = 0;
	int				stab_tmp;
	bn_struct		*elem;
	char		   	msg[500];
	
	//The sink is always a border node !
	if (is_sink)
		return(OPC_TRUE);
	

	
	switch (border_election){
	
//Border nodes are elected dynamically (from their parent which choose one child)
		case BORDER_DYNAMIC: 

			//Walks in the list
			for(i=op_prg_list_size(my_neighborhood_table)-1 ; i>= 0 ; i--){
				neigh_ptr = op_prg_list_access(my_neighborhood_table , i);
				stab_tmp = compute_stability(neigh_ptr->stability);
		
				//I am in its list of border nodes
				for(j=0 ; j < op_prg_list_size(neigh_ptr->border_nodes_list) ; j++){
					addr_ptr = op_prg_list_access(neigh_ptr->border_nodes_list , j);
			
					//It is one parent -> store its stability
					if ((*addr_ptr == my_address) && (stab_tmp > stab_my_parent))
						stab_my_parent = stab_tmp;
				}
				//The max stability
				if (stab_tmp > max_stab_in_neigh)
					max_stab_in_neigh = stab_tmp;
			}
	
		//No one
		if (stab_my_parent == 0)
			return(OPC_FALSE);
	
		//This parent has a sufficient stability
		if (stab_my_parent >= max_stab_in_neigh - STAB_STEP)
			return(OPC_TRUE);
		else
			return(OPC_FALSE);
	break;

		
//The border nodes are elected in a centralized manner
	case BORDER_CENTRALIZED	:
	
		for(i=0 ; i < op_prg_list_size(global_border_nodes_list) ; i++){
			elem = op_prg_list_access(global_border_nodes_list , i);
			if (elem->address == my_address)
				return(OPC_TRUE);
		}
		
		return(OPC_FALSE);
	break;		
	
		
		
//Error : no border nodes election
	default :
		sprintf(msg , "%d unknown" , border_election);
		op_sim_end("Unknown border node election" , msg , "" , "");
	break;
	}
}


//empty a list (memory release of all elements)
void empty_list(List *ll){
	void	*ptr;
	
	while(op_prg_list_size(ll) != 0){
		ptr = op_prg_list_remove(ll ,0);
		op_prg_mem_free(ptr);
	}
}


//Changes the status of this node
Boolean update_is_border_node(){
	int		old_value;

	old_value = is_border_node;
	is_border_node = get_is_border_node();
	
	//the value changed !
	if (is_border_node != old_value){
		
		//updates the border distance
		if (is_border_node)
			my_dist_border = 0;
	
		//debug
		debug_print(LOW , DEBUG_NODE , "changed the border_node status: %d -> %d\n", old_value , is_border_node);
		//printf("%d border_node status: %d -> %d\n", my_address , old_value , is_border_node);
		print_neighborhood_table(DEBUG_NODE);
	}
	
	
	//The value changed (a gratuitous hello is surely required)
	return(old_value != is_border_node);
}


//-----------------------------------------------------------
//
//				DISTANCES BORDER / SINK
//
//-----------------------------------------------------------


//Distance to the sink to the border nodes
void update_dist(){
	int				i;
	neigh_struct	*neigh_ptr;
	int				current_dist_border , current_dist_sink;
	int				current_stab;
	
	//init
	current_dist_border = OPC_INT_INFINITY;
	current_dist_sink	= OPC_INT_INFINITY;
	current_stab		= 0;
	
	//particular case
	if (is_sink){
		my_dist_sink 	= 0;
		my_dist_border	= 0;
		return;
	}
	
	//Walks the neighborhood table and update distance (takes only into account the most stable nodes)
	for(i=0 ; i < op_prg_list_size(my_neighborhood_table) ; i++){
		neigh_ptr = op_prg_list_access(my_neighborhood_table , i);
		
		//most stable -> most important
		if (current_stab + STAB_STEP < compute_stability(neigh_ptr->stability)){
			current_dist_sink	= neigh_ptr->dist_sink 		+ 1;
			current_dist_border = neigh_ptr->dist_border 	+ 1;
			current_stab 		= compute_stability(neigh_ptr->stability);
		}
		
		
		//Else, if stability is too low -> nothing 
		else if (current_stab > compute_stability(neigh_ptr->stability) + STAB_STEP){
		}
		
		
		//Else, update distances (consider that the stabilities are similar)		
		else{
			//Distance to the border
			if (current_dist_border > neigh_ptr->dist_border + 1){
				current_dist_border = neigh_ptr->dist_border + 1;
				current_stab = compute_stability(neigh_ptr->stability);
			}
			
			//Distance to the sink
			if (current_dist_sink > neigh_ptr->dist_sink + 1){
				current_dist_sink = neigh_ptr->dist_sink + 1;
				current_stab = compute_stability(neigh_ptr->stability);
			}
		}
		
		debug_print(MEDIUM , DEBUG_HELLO , "	-> %d:   %d/%d   %d/%d  %d/%d\n", neigh_ptr->address , compute_stability(neigh_ptr->stability) , current_stab , neigh_ptr->dist_border , current_dist_border , neigh_ptr->dist_sink , current_dist_sink);
	}
	
	//particular case
	if (is_border_node)
		current_dist_border = 0;
	
	my_dist_sink 	= current_dist_sink;
	my_dist_border	= current_dist_border;
	
	debug_print(MAX , DEBUG_HELLO , "Distances update\n");
	if (DEBUG >= MEDIUM)
		print_neighborhood_table(DEBUG_HELLO);
}











//-----------------------------------------------------------
//
//				BORDER FIELDS in HELLOS
//
//-----------------------------------------------------------


//Creates a list of border nodes
List *create_bn_list_from_packet(Packet *pk){
	//result
	List	*ll;
	//List size and elements
	int		nb_borders;
	int		*ptr;
	//Control
	char	field_name[50];
	int		i;
	//info
	int		source;
	
	//init
	ll = op_prg_list_create();
		
	//nb border nodes
	op_pk_nfd_get(pk ,"NB_BORDERS" , 	&nb_borders);
	op_pk_nfd_get(pk ,"SOURCE" , 		&source);

	//insert each border node in the list
	for(i=0 ; i<nb_borders; i++){
		ptr = op_prg_mem_alloc(sizeof(int));		
		sprintf(field_name ,"BORDER_%d", i);
		op_pk_nfd_get(pk , field_name , ptr);
		op_prg_list_insert(ll , ptr , OPC_LISTPOS_HEAD);
	}
	
	return(ll);
}

//Compares to couples addr/power 
int compare_election_struct(void *value_a, void * value_b){
	election_struct	a , b;
	
	a = *(election_struct*)value_a;
	b = *(election_struct*)value_b;
	
	
	//null power -> bad
	if (b.pow == 0)
		return(1);
	else if (a.pow == 0)
		return(-1);
	
	//0° criterium: branch nb (for the sink, else no particular interest)
	if ((is_sink) && (is_sync_direct_antenna)){
		if (a.branch < b.branch)
			return(1);
		else if (a.branch > b.branch)
			return(-1);
	}
	
	//First criterium: the stability
	else if (a.stab > b.stab + STAB_STEP)
		return(1);
	else if (a.stab < b.stab - STAB_STEP)
		return(-1);
	
	//Second criterium: power
	if (a.pow < b.pow)
		return(1);
	else if (a.pow > b.pow)
		return(-1);
	
	//3rd criterium: id
	else {
		if (a.addr > b.addr)
			return(1);
		else
			return(-1);
	}
}

//returns the min value
int	min_int(int a , int b){
	if (a < b)
		return(a);
	else
		return(b);
}

//compare two int_ptr
int	compare_int(void *a1 , void *a2){
	int	a,b;
	
	a = *(int*)a1;
	b = *(int*)a2;
	
	if (a < b)
		return(1);
	else if (a > b )
		return(-1);
	else return(0);

}
	
//returns true if the lists are identical
Boolean compare_bn_list(List *l1, List * l2){
	int		i;
	int		*a , *b;


	if (op_prg_list_size(l1) != op_prg_list_size(l2))
		return(OPC_FALSE);

	//sorts the lists
	op_prg_list_sort(l1, compare_int);
	op_prg_list_sort(l2, compare_int);
	
	//Compare it
	for (i=0 ; i < op_prg_list_size(l1) ; i++){
		a = op_prg_list_access(l1 , i);
		b = op_prg_list_access(l2 , i);
		
		if (*a != *b)
			return(OPC_FALSE);
	}
	return(OPC_TRUE);
}

//returns the list of current border nodes
void compute_current_bn_list(List **ll){
	int				i;
	neigh_struct	*neigh_ptr;
	//To store all the power, sorted, in a list
	List			*max_sync_rx_power;
	election_struct	*ptr;
	//Control
	int				*int_ptr;
	int				branch_tmp = -1;
	//conditions
	Boolean			direct_and_new;
	Boolean			no_direct_and_new;
	//centralized border nodes list
	bn_struct		*elem;
	
	
	
	//empty the old list
	while(op_prg_list_size(*ll) > 0){
		int_ptr = op_prg_list_remove(*ll , 0);
		op_prg_mem_free(int_ptr);
	}
	
	
	
	switch(border_election){
	
	
		case BORDER_CENTRALIZED :
			for(i= 0 ; i < op_prg_list_size(global_border_nodes_list) ; i++){
				elem = op_prg_list_access(global_border_nodes_list , i);
				
				//adds the node if I am its parent (It's my child !)
				if (elem->parent == my_address){
					int_ptr = op_prg_mem_alloc(sizeof(int));
					*int_ptr = elem->address;
					op_prg_list_insert(*ll , int_ptr , OPC_LISTPOS_TAIL);
				}			
			}
		
		break;
		
		
		
		
		case BORDER_DYNAMIC :		
	

			//--------------------------------------------------
			//				NO BORDER NODE
			//--------------------------------------------------
			if ((!is_sink) && ((my_sync_rx_power == 0) || (!is_border_node) || (my_dist_sink > MAX_BRANCH_LENGTH)))
				debug_print(MAX , DEBUG_NODE , "No border node: %d %d %d\n", my_sync_rx_power == 0 , !is_border_node , my_dist_sink > MAX_BRANCH_LENGTH);

			else{
				//initialization
				max_sync_rx_power = op_prg_list_create();
			
				
				
				//--------------------------------------------------
				//	Stores all the power of my neighbors (sorted)
				//--------------------------------------------------
				for(i= op_prg_list_size(my_neighborhood_table)-1 ; i>= 0 ; i--){
					
					neigh_ptr = op_prg_list_access(my_neighborhood_table , i);
				
					//Adds the power if the node is farther from the sink
					if (neigh_ptr->dist_sink > my_dist_sink){
						ptr = op_prg_mem_alloc(sizeof(election_struct));
						ptr->addr 	= neigh_ptr->address;
						ptr->pow 	= neigh_ptr->sync_rx_power;
						ptr->stab	= compute_stability(neigh_ptr->stability);
						ptr->branch	= neigh_ptr->branch;
						op_prg_list_insert_sorted(max_sync_rx_power , ptr , compare_election_struct);
					}
				}
			
				//--------------------------------------------------
				//			No node in the list
				//--------------------------------------------------
				if (op_prg_list_size(max_sync_rx_power) == 0){
				}
			
				//--------------------------------------------------
				//		For the sink -> places the N min power
				//--------------------------------------------------
				else if (is_sink){
					debug_print(LOW , DEBUG_NODE , "Border nodes: \n");

					//Firt branch to deal with
					branch_tmp = -1;
					
					//Walks in the sorted list of my neighbors
					for(i=0 ; i < op_prg_list_size(max_sync_rx_power) ; i++){
						ptr = op_prg_list_access(max_sync_rx_power , i);
						
						debug_print(MAX , DEBUG_NODE , "	ptr (%d : %d %f) current (%d)\n", ptr->addr , ptr->branch , ptr->pow , branch_tmp);
						
						//Eliminates nodes nearer from the sink than I am
						direct_and_new		= (is_sync_direct_antenna) && (ptr->branch > branch_tmp);
						no_direct_and_new	= (!is_sync_direct_antenna) && (i < MAX_NB_BRANCHES);
						
						if ((ptr->pow != 0) && (direct_and_new || no_direct_and_new)){
							branch_tmp = ptr->branch;
						
							//creates a list of current border nodes
							int_ptr = op_prg_mem_alloc(sizeof(int));
							*int_ptr = ptr->addr;
							op_prg_list_insert(*ll , int_ptr , OPC_LISTPOS_TAIL);
							
							debug_print(MAX , DEBUG_NODE , "	->%d\n", ptr->addr);					
						}
					}
				}	
				//------------------------------------------------------
				//	For a normal border node, chooses only the lowest
				//------------------------------------------------------
				else{

					ptr = op_prg_list_access(max_sync_rx_power , 0);
					
					if ((ptr->pow < my_sync_rx_power) && (ptr->pow != 0)){
						debug_print(MAX , DEBUG_NODE , "node %d chosen as border node\n", ptr->addr);
							
						//creates a list of current border nodes
						int_ptr = op_prg_mem_alloc(sizeof(int));
						*int_ptr = ptr->addr;
						op_prg_list_insert(*ll , int_ptr , OPC_LISTPOS_TAIL);
					}
					else
						debug_print(MAX , DEBUG_NODE , "no border node : %d\n", ptr->pow < my_sync_rx_power , ptr->pow != 0);
					
				
				}
					

				//--------------------------------------------------
				//			release memory
				//--------------------------------------------------
				while(op_prg_list_size(max_sync_rx_power) > 0){
					ptr = op_prg_list_remove(max_sync_rx_power , 0);
					op_prg_mem_free(ptr);
				}
				op_prg_mem_free(max_sync_rx_power);
			}
		break;
	}	
}

	
//fills the packet with the list of my border nodes
void fill_border_nodes_fields(Packet *pk){
	int				*int_ptr;
	//Control
	int				i;
	char			field_name[50];


	//Current border node list
	compute_current_bn_list(&bn_list);
	
	//Nb of border nodes
	op_pk_nfd_set(pk , "NB_BORDERS" , op_prg_list_size(bn_list));
	
	
	//Fills border fields
	for(i=0 ; i < MAX_NB_BRANCHES ; i++){
		sprintf(field_name , "BORDER_%d", i);
		
		//Border fields
		if (i < op_prg_list_size(bn_list)){
			int_ptr = op_prg_list_access(bn_list , i);
			op_pk_nfd_set(pk, field_name , *int_ptr);
		}
		//No more border fields
		else if (op_pk_nfd_is_set(pk, field_name))
			op_pk_nfd_strip(pk , field_name);
	
	}
	
	print_neighborhood_table(DEBUG_NODE);
}




//-----------------------------------------------------------
//
//				BORDER NODES (neighbors...)
//
//-----------------------------------------------------------



//Returns the border node (not for the sink, it have several border nodes !)
int get_child_border_node(){
	int			*int_ptr;
	
	if ((!is_border_node) || (op_prg_list_size(bn_list) == 0))
		return(BROADCAST);
	
	int_ptr = op_prg_list_access(bn_list , 0);
	return(*int_ptr);
}


//Returns the child of the current branch
int get_child_from_sink(int current_branch){
	int			*int_ptr;

	if ((!is_border_node) || (op_prg_list_size(bn_list) <= current_branch))
		return(BROADCAST);
	
	int_ptr = op_prg_list_access(bn_list , current_branch);
	return(*int_ptr);
}


//Prints the list of the current border nodes
char* print_border_nodes(char *msg){
	int		*int_ptr;
	int		i;

	sprintf(msg , "");
	for(i=0; i < op_prg_list_size(bn_list) ; i++){
		int_ptr = op_prg_list_access(bn_list , i);
		sprintf(msg , "%s %d" , msg , *int_ptr);
	}
	return(msg);
}
	
//returns the nb of current branches (number of border nodes father than I am from the sink)
int get_nb_branches(){
	return(op_prg_list_size(bn_list));
}


//returns the id of my father (and border node)
int get_father_border_node(){
	int				i;
	neigh_struct	*neigh_ptr;
	
	//Walks in the list
	for(i=0; i<op_prg_list_size(my_neighborhood_table) ; i++){
		neigh_ptr = op_prg_list_access(my_neighborhood_table , i);
		
		if ((neigh_ptr->dist_border == 0) && (neigh_ptr->dist_sink + 1 == my_dist_sink))
			return(neigh_ptr->address);
	}
	
	//default behavior
	return(-1);
}









//-----------------------------------------------------------
//
//					HELLOS
//
//-----------------------------------------------------------

//Computes the stability metric
int compute_stability(int stab[]){
	int		i;
	int		stability = 0;
	
	for(i=0 ; i < MAX_STAB ; i++)
		if (stab[i] > 0)
			stability ++;
	
	return(stability);
}

//updates one value of the stability metric
void update_stability(int stab[], short value){
	int		i;
	
	for(i=0 ; stab[i] != -1 ; i++)
		if (i >= MAX_STAB)
			op_sim_end("stability metric has a bug" , "no -1 is present in any case" , "" , "");

	//puts the value + next case = -1
	stab[i] = value;
	i++;
	if (i == MAX_STAB)
		i = 0;
	stab[i] = -1;

}

//Initializes the stability metric (with 'value')
void init_stability(int stab[], short value){
	int		i;
	
	//null array
	for(i=0 ; i < MAX_STAB ; i++)
		stab[i] = 0;
	
	
	stab[0] = value;
	stab[1] = -1;
}


//Generates an hello (next frame to send !)
void  generate_hello(double next_hello){ 
	//Packet to send
	frame_struct	frame;
	//info
	int				frame_id;
	
	//frame id
	frame_id = get_new_frame_id();
	
	frame.source				= my_address;
	frame.destination			= BROADCAST;
	frame.type					= HELLO_PK_TYPE;
	frame.frame_id				= frame_id;
	frame.nb_retry				= 0;
	frame.time_added			= op_sim_time();
	frame.time_sent				= 0;
	frame.time_transmission_min	= 0;
	frame.released				= OPC_FALSE;
	frame.payload				= NULL;
	frame.duration				= 0;
	frame.power_ratio			= 1;
	frame.pk_size				= HEADERS_PK_SIZE + DIST_SINK_SIZE + DIST_BORDER_SIZE;
	frame.next_hello			= next_hello;
	
	//Adds the hello in head of the data packet buffer
	add_in_multicast_frame_buffer(frame , OPC_LISTPOS_HEAD);
	is_hello_to_send 			= OPC_TRUE;
	
	debug_print(LOW , DEBUG_HELLO , "HELLO generated\n");
	debug_print(MAX , DEBUG_NODE , "HELLO generated\n");
}



//Deletes the neighbors which did not send an HELLO for a long time (timeout)
void delete_timeouted_neighbors(Vartype*ptr , int code){
	double			older_entry = 0;
	int				i;
	neigh_struct	*neigh_ptr;
	Boolean			is_modif_required = OPC_FALSE;
	
	//Walks in the list
	for(i= op_prg_list_size(my_neighborhood_table)-1 ; i>= 0 ; i--){
		neigh_ptr = op_prg_list_access(my_neighborhood_table , i);
		
		//The entry is timeouted -> stability --
		if (neigh_ptr->timeout <= op_sim_time()){
			update_stability(neigh_ptr->stability , OPC_FALSE);
			neigh_ptr->timeout = INTERVALL_HELLO + op_sim_time();

			//Stability = 0 -> delete the entry
			if (compute_stability(neigh_ptr->stability) == 0){		
				neigh_ptr = op_prg_list_remove(my_neighborhood_table , i);
			
				debug_print(LOW , DEBUG_HELLO , "neighbor %d timeouted after a null stability\n", neigh_ptr->address);
				debug_print(LOW , DEBUG_NODE , "neighbor %d timeouted after a null stability\n",  neigh_ptr->address);

				empty_list(neigh_ptr->border_nodes_list);
				op_prg_mem_free(neigh_ptr->border_nodes_list);
				op_prg_mem_free(neigh_ptr);
				neigh_ptr = NULL;
			}
			
			is_modif_required = OPC_TRUE;
		}
		
		//when must be done the next verification ?
		if ((neigh_ptr != NULL) && ((older_entry > neigh_ptr->timeout) || (older_entry == 0)))
			older_entry = neigh_ptr->timeout;
	}
	
	
	//Modification in the neighborhood table !
	if (is_modif_required){
		//Border node
		if (update_is_border_node())
			generate_hello(0);

		//Distances
		update_dist();
	}
	
	//Next verification
	if (op_prg_list_size(my_neighborhood_table) != 0)
		op_intrpt_schedule_call(older_entry , NEIGHBOR_TIMEOUT_CODE , delete_timeouted_neighbors , NULL);
	
	
}



//updates the neighborhood table with one new entry !
void update_neighborhood_table(int source , int dist_sink , int dist_border, double sync_rx_power, int branch , List *bn_list_tmp , double next_hello){
	int				i;
	neigh_struct	*neigh_ptr;
	Boolean			neighbor_updated 	= OPC_FALSE;
	Boolean			hello_required 		= OPC_FALSE;
	
	
	//---------------------------
	//	DIST SINK / BORDER
	//---------------------------
	if (my_dist_border > dist_border + 1){
		my_dist_border	= dist_border + 1 ;
		my_dist_sink 	= dist_sink + 1 ;
		
		debug_print(LOW , DEBUG_HELLO , "Distances updated : border %d, sink %d via %d\n", my_dist_sink, my_dist_border , source);
	//	hello_required = OPC_TRUE;
	}
	if ((dist_border == my_dist_border) && (my_dist_sink > dist_sink + 1)){
		my_dist_sink = dist_sink + 1;
		
		debug_print(LOW , DEBUG_HELLO , "Sink distance updated : sink %d via %d\n", my_dist_sink, my_dist_border , source);
	//	hello_required = OPC_TRUE;
	}
		
	//---------------------------
	//		LIST UPDATE
	//---------------------------
	for(i=0; i<op_prg_list_size(my_neighborhood_table) ; i++){
		neigh_ptr = op_prg_list_access(my_neighborhood_table , i);
	
		if (neigh_ptr->address == source){
			op_prg_mem_free(neigh_ptr->border_nodes_list);
			neigh_ptr->dist_sink			= dist_sink;
			neigh_ptr->dist_border			= dist_border;
			neigh_ptr->sync_rx_power		= sync_rx_power;
			neigh_ptr->branch				= branch;
			neigh_ptr->border_nodes_list	= bn_list_tmp;
			update_stability(neigh_ptr->stability , OPC_TRUE);
			neighbor_updated = OPC_TRUE;
			if (next_hello != 0)
				neigh_ptr->timeout				= op_sim_time() + next_hello + 0.1;			
				
			debug_print(MEDIUM , DEBUG_HELLO, "neighbor %d updated (sink %d, border %d)\n", source, dist_sink , dist_border);
			debug_print(MEDIUM , DEBUG_NODE, "neighbor %d updated (sink %d, border %d)\n", source, dist_sink , dist_border);
			
		}	
	}
	

	//---------------------------
	//		LIST ADD
	//---------------------------
	if (!neighbor_updated){
		//We add one entry -> we must schedule the timeout verification
		if (op_prg_list_size(my_neighborhood_table) == 0)
			op_intrpt_schedule_call(op_sim_time() + INTERVALL_HELLO , NEIGHBOR_TIMEOUT_CODE , delete_timeouted_neighbors , NULL);

		neigh_ptr = op_prg_mem_alloc(sizeof(neigh_struct));
		neigh_ptr->address				= source;
		neigh_ptr->dist_sink			= dist_sink;
		neigh_ptr->dist_border			= dist_border;
		neigh_ptr->sync_rx_power		= sync_rx_power;
		neigh_ptr->branch				= branch;
		neigh_ptr->timeout				= op_sim_time() +INTERVALL_HELLO;
		neigh_ptr->border_nodes_list	= bn_list_tmp;
		neigh_ptr->timeout				= op_sim_time() + next_hello + 0.1;			
		init_stability(neigh_ptr->stability , OPC_TRUE);
		op_prg_list_insert(my_neighborhood_table , neigh_ptr , OPC_LISTPOS_TAIL);	  
		
	
		debug_print(MEDIUM , DEBUG_HELLO, "neighbor %d added (sink %d, border %d)\n", source, dist_sink , dist_border);
		debug_print(MEDIUM , DEBUG_NODE, "neighbor %d added (sink %d, border %d)\n", source, dist_sink , dist_border);
	}
	
	//---------------------------
	//		IS BORDER NODE ?
	//---------------------------
	//NB: I must verify for each reception 
	//the source may have change the list of its border nodes
	
	if (update_is_border_node())
		hello_required = OPC_TRUE;
		
	//---------------------------
	//		IS BORDER NODE ?
	//---------------------------
	update_dist();
	
	
	
	//I changed by border node status or my distance to sink / border
	if (hello_required)
		generate_hello(0);
	
}



//Prints in my debug_file the neighborhood table
void print_neighborhood_table(int debug_type){
	int				i , j;
	neigh_struct	*ptr;
	int				*addr_ptr;
	char			msg[500];

	
	debug_print(LOW , debug_type , "------------------------------------------------------------------------------------------------------------\n");
	debug_print(LOW , debug_type , "								NEIGHBORHOOD TABLE (%f)\n", op_sim_time());
	debug_print(LOW , debug_type , "------------------------------------------------------------------------------------------------------------\n");
	debug_print(LOW , debug_type , "\n");
	
	debug_print(LOW , debug_type , "    ADDR	|	DIST_SINK	|	DIST_BORDER	|  BRANCH	|	TIMEOUT		| STABILITY	|BN_LIST	|	SYNC_POWER\n");
	
	for (i=0 ; i< op_prg_list_size(my_neighborhood_table) ; i++){
		ptr = op_prg_list_access(my_neighborhood_table , i);
		
		//list of border nodes
		sprintf(msg , "");
		for(j=0; j < op_prg_list_size(ptr->border_nodes_list) ; j++){
			addr_ptr = op_prg_list_access(ptr->border_nodes_list , j);
			sprintf(msg , "%s %d", msg , *addr_ptr);
		}
			
		//the whole info
		debug_print(LOW , debug_type , "%8d	|	%3d		|	%3d		|	%d	|	%f		|	%d	|	%s	|	%8.0f\n", ptr->address , convert_int(ptr->dist_sink) , convert_int(ptr->dist_border) , ptr->branch , ptr->timeout , compute_stability(ptr->stability) , msg ,  convert_double(ptr->sync_rx_power));
	}
	debug_print(LOW , debug_type , "\n");
	debug_print(LOW , debug_type , "ME: 	DIST_SINK 			= %d 		DIST_BORDER 	= %d\n", 	my_dist_sink , 	my_dist_border );
	debug_print(LOW , debug_type , "	 	IS_BORDER_NODE 		= %d 		SYNC_RX_POWER 	= %f\n", 	is_border_node, convert_double(my_sync_rx_power));
	debug_print(LOW , debug_type , "		BORDER NODES(last sent)	= %s\n" , print_border_nodes(msg));
	debug_print(LOW , debug_type , "\n");
}


























//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------																										  ---------------------------
//---------------------------												CONTROL FRAME											  ---------------------------
//---------------------------																										  ---------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------



//-----------------------------------------------------------
//
//			  	NEXT_FRAME to Send
//
//-----------------------------------------------------------

//from info about the frame, generate the corresponding packet (juste before transmission)
Packet* create_packet(frame_struct frame){
	char	msg[500];
	Packet	*pk;

	switch(frame.type){
		case DATA_MULTICAST_PK_TYPE:
		case DATA_UNICAST_PK_TYPE:
			pk = op_pk_create_fmt("cmac_frame");
			op_pk_nfd_set(pk , "PAYLOAD" , 		op_pk_copy(frame.payload));
			op_pk_nfd_set(pk , "DURATION" , 	frame.duration);
			op_pk_nfd_set(pk , "POWER_RATIO" , 	frame.power_ratio);
		break;
		case HELLO_PK_TYPE:
			pk = op_pk_create_fmt("cmac_hello");
			op_pk_nfd_set(pk , "DIST_SINK" , 	my_dist_sink);
			op_pk_nfd_set(pk , "DIST_BORDER" ,	my_dist_border);
			op_pk_nfd_set(pk , "SYNC_POWER" ,	my_sync_rx_power);
			op_pk_nfd_set(pk , "BRANCH" ,		my_branch);
			op_pk_nfd_set(pk , "NEXT" ,			frame.next_hello);
			fill_border_nodes_fields(pk);
		break;
		case RTS_PK_TYPE:
			pk = op_pk_create_fmt("cmac_rts");
			op_pk_nfd_set(pk , "DURATION" , 	frame.duration);
			op_pk_nfd_set(pk , "POWER_RATIO" , 	frame.power_ratio);
		break;
		case CTS_PK_TYPE:
			pk = op_pk_create_fmt("cmac_cts");
			op_pk_nfd_set(pk , "DURATION" , 	frame.duration);
			op_pk_nfd_set(pk , "POWER_RATIO" , 	frame.power_ratio);
		break;
		case CTR_PK_TYPE:
			pk = op_pk_create_fmt("cmac_ctr");
			op_pk_nfd_set(pk , "FREQ" , 		my_privileged_frequency);
			op_pk_nfd_set(pk , "T_SLOT" , 		slot_privileged_duration);
			op_pk_nfd_set(pk , "OFFSET" , 	   	op_sim_time() - frame.time_added);
debug_print(LOW, DEBUG_NODE , "offset sent %f\n", op_sim_time() - frame.time_added);
//TAG
//			op_pk_nfd_set(pk , "OFFSET" , 		0);
		
		break;
		case CTR_ACK_PK_TYPE:
			pk = op_pk_create_fmt("cmac_ctr_ack");
			op_pk_nfd_set(pk , "FREQ" , 		my_privileged_frequency);
			op_pk_nfd_set(pk , "T_SLOT" , 		slot_privileged_duration);
		break;
		case CTR_END_PK_TYPE:
			pk = op_pk_create_fmt("cmac_ctr_end");
		break;
		case ACK_PK_TYPE:
			pk = op_pk_create_fmt("cmac_ack");
			op_pk_nfd_set(pk , "DURATION" , 	frame.duration);
		break;
		case SYNC_PK_TYPE:
			pk = op_pk_create_fmt("cmac_sync");
			op_pk_nfd_set(pk , "BRANCH" , 		sync_last_branch);
		break;
		default:
			sprintf(msg, "Packet type %d unknown");
			op_sim_end(msg , "Consequently, I cannot generate the corresponding packet" , "" , "");
		break;
	}
	
	
	//Common fields
	op_pk_nfd_set(pk , "SOURCE" , 			frame.source);
	op_pk_nfd_set(pk , "DESTINATION" , 		frame.destination);
	op_pk_nfd_set(pk , "TYPE" ,				frame.type);
	op_pk_nfd_set(pk , "Data Packet ID" ,	frame.frame_id);
	op_pk_nfd_set(pk , "Accept" ,			OPC_TRUE);
	
	//returns the frame !
	return(pk);
}


//Deletes the next scheduled frame
void set_next_frame_null(){
	char	msg1[50];

	debug_print(MEDIUM , DEBUG_SEND , "old frame scheduled (destination %d, type %s) canceled / deleted\n", next_frame_to_send.destination, pk_type_to_str(next_frame_to_send.type , msg1));
	
	next_frame_to_send.type = NO_PK_TYPE;
}


//Replaces the next_frame_to_send
void change_next_frame(frame_struct new_frame){
	char	msg1[50];
	char	msg2[50];

	debug_print(MEDIUM , DEBUG_SEND , "old frame scheduled (destination %d, type %s) replaced by a new frame (destination %d, type %s)\n", next_frame_to_send.destination, pk_type_to_str(next_frame_to_send.type , msg1) , new_frame.destination , pk_type_to_str(new_frame.type , msg2));
	
	next_frame_to_send = new_frame;
}




//-----------------------------------------------------------
//
//				 FRAME ID MANAGEMENT
//
//-----------------------------------------------------------

//Returns a new unique frame id
int get_new_frame_id(){

	global_frame_id = (global_frame_id + 1) % ((int)(pow(2, sizeof(int) * 8) / 2) - 1);
	return(global_frame_id);
}






//-----------------------------------------------------------
//
//					REQUEST TO SEND
//
//-----------------------------------------------------------

//Generates a RTS
void generate_rts(int destination , int duration , int data_frame_id , double power_ratio){
	//Packet to send
	frame_struct		frame;

	//Next frame to send
	frame.source 				= my_address;
	frame.destination 			= destination;
	frame.type 					= RTS_PK_TYPE;
	frame.frame_id 				= data_frame_id;
	frame.nb_retry				= 0;
	frame.duration				= duration;
	frame.power_ratio			= power_ratio;
	frame.pk_size				= HEADERS_PK_SIZE + DURATION_SIZE;
	frame.time_added			= op_sim_time();
	frame.time_sent				= 0;
	frame.time_transmission_min	= 0;
	frame.released				= OPC_FALSE;
	frame.payload				= NULL;
	change_next_frame(frame);

	
	
	
	debug_print(MEDIUM , DEBUG_CONTROL , "RTS for %d (frame_id %d)\n", destination , data_frame_id);
}




//-----------------------------------------------------------
//
//					CLEAR TO SEND
//
//-----------------------------------------------------------

//Generates a CTS
void generate_cts(int destination , int duration, int frame_id , double power_ratio){
	//Packet to send
	frame_struct		frame;
	
	//Next frame to send
	frame.source 				= my_address;
	frame.destination 			= destination;
	frame.type 					= CTS_PK_TYPE;
	frame.frame_id 				= frame_id;
	frame.nb_retry				= 0;
	frame.duration				= duration;
	frame.power_ratio			= power_ratio;
	frame.pk_size				= HEADERS_PK_SIZE + DURATION_SIZE;
	frame.time_added			= op_sim_time();
	frame.time_sent				= 0;
	frame.time_transmission_min	= 0;
	frame.released				= OPC_FALSE;
	frame.payload				= NULL;
	change_next_frame(frame);

	debug_print(MEDIUM , DEBUG_CONTROL , "CTS for %d, frame id\n",  destination ,  frame_id);
}




//-----------------------------------------------------------
//
//					ACKNOWLEDGEMENT
//
//-----------------------------------------------------------

//Generates an ACK
void generate_ack(int destination, int frame_id , int duration , double power_ratio){
	//Packet to send
	frame_struct		frame;
	
	//Next frame to send
	frame.source 				= my_address;
	frame.destination 			= destination;
	frame.type 					= ACK_PK_TYPE;
	frame.frame_id 				= frame_id;
	frame.nb_retry				= 0;
	frame.power_ratio			= power_ratio;
	frame.pk_size				= HEADERS_PK_SIZE;
	frame.time_added			= op_sim_time();
	frame.time_sent				= 0;
	frame.time_transmission_min	= 0;
	frame.duration 				= duration;
	frame.released				= OPC_FALSE;
	frame.payload				= NULL;
	
	//prepares transmission
	change_next_frame(frame);
	
	debug_print(MEDIUM , DEBUG_CONTROL , "ACK generated by %d at %f\n", my_address , op_sim_time());
}



//-----------------------------------------------------------
//
//					SYNCHRONIZATION
//
//-----------------------------------------------------------


//Generates a SYNC and schedules its transmission (next_frame_to_send)
void generate_sync(){
	//Packet to send
	frame_struct		frame;
	//info
	int					frame_id;
	
	//frame id
	frame_id = get_new_frame_id();
	//Next frame to send
	frame.source 				= my_address;
	frame.destination 			= BROADCAST;
	frame.type 					= SYNC_PK_TYPE;
	frame.frame_id 				= frame_id;
	frame.nb_retry				= 0;
	frame.duration				= 0;
	frame.power_ratio			= 1;
	frame.pk_size				= HEADERS_PK_SIZE + NB_TIER_SIZE;
	frame.time_added			= op_sim_time();
	frame.time_sent				= 0;
	frame.time_transmission_min	= 0;
	frame.released				= OPC_FALSE;
	frame.payload				= NULL;
	change_next_frame(frame);
	
	debug_print(MEDIUM , DEBUG_CONTROL , "SYNC generated\n");
}



//-----------------------------------------------------------
//
//					CLEAR TO RECEIVE
//
//-----------------------------------------------------------


//Generates a CTR-END and schedules its transmission (next_frame_to_send)
void generate_ctr_end(){
	//Packet to send
	frame_struct		frame;
	//info
	int				frame_id;
	
	printf("CTR-END\n");
	
	//frame id
	frame_id = get_new_frame_id();
	//Next frame to send
	frame.source 				= my_address;
	frame.destination 			= BROADCAST;
	frame.type 					= CTR_END_PK_TYPE;
	frame.frame_id 				= frame_id;
	frame.nb_retry				= 0;
	frame.duration				= 0;
	frame.power_ratio			= 1;
	frame.pk_size				= HEADERS_PK_SIZE;
	frame.time_added			= op_sim_time();
	frame.time_sent				= 0;
	frame.time_transmission_min	= 0;
	frame.released				= OPC_FALSE;
	frame.payload				= NULL;
	change_next_frame(frame);
	
	debug_print(MEDIUM , DEBUG_CONTROL , "CTR-END generated\n");
}



//Generates a CTR and schedules its transmission (next_frame_to_send)
void generate_ctr(){
	//Packet to send
	frame_struct	frame;
	//info
	int				destination;
	int				frame_id;
	
	//Main frequency
	change_tx_rx_freq(my_main_frequency , my_main_bandwidth , STREAM_TO_RADIO);
	
	//Destination
	if (is_sink){
		ctr_last_branch++;
		
		//If several available channels, get one for this branch (round robin when the branch changes)
		if (nb_channels > 1){
			my_privileged_frequency = my_main_frequency;
			my_privileged_frequency += my_main_bandwidth / 1000 + SHIFT_FREQ_SEPARATION;
			my_privileged_frequency += (my_main_bandwidth * RATIO_PRIV_BANDWIDTH  / 1000 + SHIFT_FREQ_SEPARATION) * (ctr_last_branch % (nb_channels - 1));
		}
		else
			my_privileged_frequency = my_main_frequency;
		
		
		if (ctr_last_branch >= get_nb_branches())
			ctr_last_branch = 0;
		destination = get_child_from_sink(ctr_last_branch);	
	}
	else
		destination = get_child_border_node();
	
	//No more border node
	if (destination == BROADCAST){
		is_node_privileged = OPC_FALSE;
		return;
	}
	
	//Normal CTR
	else {
		//Unavailability of the next hop
		//update_nav_time(2 * (slot_privileged_duration + MAX_CTR_DELAY_FROM_SINK) , destination , my_privileged_frequency , 0);
	
		//frame id
		frame_id = get_new_frame_id();
	
		//Next frame to send
		frame.source 				= my_address;
		frame.destination 			= destination;
		frame.type 					= CTR_PK_TYPE;
		frame.frame_id 				= frame_id;
		frame.nb_retry				= 0;
		frame.duration				= 0;
		frame.power_ratio			= 1;
		frame.pk_size				= HEADERS_PK_SIZE;
		frame.time_added			= op_sim_time();
		frame.time_sent				= 0;
		frame.time_transmission_min	= 0;
		frame.released				= OPC_FALSE;
		frame.payload				= NULL;
		change_next_frame(frame);
	
		debug_print(LOW , DEBUG_CONTROL , "CTR generated for the child %d\n", destination);
	}
}



//Generates a CTR-ACK
void generate_ctr_ack(int destination, int frame_id , int duration_unavailable){
	//Packet to send
	frame_struct		frame;
	
	//Next frame to send
	frame.source 				= my_address;
	frame.destination 			= destination;
	frame.type 					= CTR_ACK_PK_TYPE;
	frame.frame_id 				= frame_id;
	frame.nb_retry				= 0;
	frame.power_ratio			= 1;
	frame.pk_size				= HEADERS_PK_SIZE;
	frame.time_added			= op_sim_time();
	frame.time_sent				= 0;
	frame.time_transmission_min	= 0;
	frame.duration 				= duration_unavailable;
	frame.released				= OPC_FALSE;
	frame.payload				= NULL;
	
	//prepares transmission
	change_next_frame(frame);
	
	
	debug_print(MEDIUM , DEBUG_CONTROL , "CTR-ACK generated by %d at %f\n", my_address , op_sim_time());
}




//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------																										  ---------------------------
//---------------------------												RECEPTION												  ---------------------------
//---------------------------																										  ---------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------












//-----------------------------------------------------------
//
//				 RECEIVE FROM UPPER
//
//-----------------------------------------------------------

//Prepares a data frame (headers, queue...)
void prepare_data_frame(Packet	*payload , int frame_id , int next_hop , double power_ratio){
	//Packet to send
	frame_struct	frame;
	
	if (DEBUG >= MAX)
		print_neighborhood_table(DEBUG_NODE);
	debug_print(MAX , DEBUG_NODE , "NEXT HOP %d -> %d\n", my_address , next_hop); 
	
	//Fields
	frame.source				= my_address;
	frame.destination			= next_hop;
	frame.frame_id				= frame_id;
	frame.nb_retry				= 0;
	frame.time_added			= op_sim_time();
	frame.time_sent				= 0;
	frame.time_transmission_min	= 0;
	frame.released				= OPC_FALSE;
	frame.payload				= payload;
	frame.duration				= 0;
	frame.power_ratio			= power_ratio;
	frame.pk_size				= HEADERS_PK_SIZE + op_pk_total_size_get(payload);
	
	if (next_hop == BROADCAST){
		frame.type	= DATA_MULTICAST_PK_TYPE;
		add_in_multicast_frame_buffer(frame , OPC_LISTPOS_TAIL);
	}
	else{
		frame.type	= DATA_UNICAST_PK_TYPE;
		add_in_unicast_frame_buffer(frame , OPC_LISTPOS_TAIL);
	}
	
}



//receives a packet from the upper layer
void receive_packet_from_up(){
	Packet	*payload;
	int		frame_id;
	//Info from the upper layer
	int		next_hop;
	Ici		*ici_ptr;
	double	power_ratio;
	
	
	//frame id
	frame_id = get_new_frame_id();
	
	//Packet from upper
	payload = op_pk_get(op_intrpt_strm());
	
	
	//Destination
	ici_ptr = op_intrpt_ici();
	
	//Usual case: the next hop is defined explicity by the upper layer
	if (ici_ptr != OPC_NIL){
		op_ici_attr_get (ici_ptr, "dest_addr", 		&next_hop);
		//op_ici_attr_get (ici_ptr, "power_ratio", 	&power_ratio);
	}	
	
	//NO routing in MAC but no specified next hop
	if ((ici_ptr == OPC_NIL) && (MAC_ROUTING == ROUTING_NO))
		op_sim_end("The mac layer is not allowed to route packets" , "and it received a packet from the upper layer" , "without any next hop specified in the ICI" , "");
	
	//Routing in MAC but a specified next hop
	else if ((ici_ptr != OPC_NIL) && (next_hop != -1) && (next_hop != 0) && (MAC_ROUTING != ROUTING_NO))
		op_sim_end("The mac layer is configured to route directly packets to the sink" , "but a next hop is specified explicitly from the upper layer" , "" , "");
		
	//routing in MAC -> computes the next hop		
	else if (MAC_ROUTING != ROUTING_NO){
		next_hop	= get_next_hop();		
		power_ratio	= 1;
	}

	//ready to send !
	if (next_hop != -1)
		prepare_data_frame(payload , frame_id , next_hop , power_ratio);

	debug_print(LOW , DEBUG_UP , "data frame sent by %d to %d (id %d)\n", my_address , next_hop , frame_id);
}





//-----------------------------------------------------------
//
//					TRANSMISSION TIMES
//
//-----------------------------------------------------------




//returns the time required for a whole exchange RTS / CTS / DATA / ACK (according to the data frame size)
double compute_rts_cts_data_ack_time(int data_pk_size){
	//Packet size
	Packet			*pkptr;
	int				rts_pk_size , cts_pk_size , ack_pk_size;	
	
	//to get the size of RTS/CTS/ACK without a constant (if we change the format -> it will be dynamically updated here !)
	//RTS
	pkptr = op_pk_create_fmt("cmac_rts");
	rts_pk_size = op_pk_total_size_get(pkptr);
	op_pk_destroy(pkptr);
	
	//CTS
	pkptr = op_pk_create_fmt("cmac_cts");
	cts_pk_size = op_pk_total_size_get(pkptr);
	op_pk_destroy(pkptr);
	
	//ACk pk size
	pkptr = op_pk_create_fmt("cmac_ack");
	ack_pk_size = op_pk_total_size_get(pkptr);
	op_pk_destroy(pkptr);
	
	//Exchange: SIFS / RTS / SIFS / CTS / SIFS / DATA / SIFS / ACK	
	return(4 * SIFS + 4 * PROPAGATION_DELAY + ((double)rts_pk_size + (double)cts_pk_size + (double)data_pk_size + (double)ack_pk_size ) / operational_speed);
}



//returns the time required for a whole exchange CTS / DATA / ACK (according to the data frame size)
double compute_cts_data_ack_time(int data_pk_size){
	//Packet size
	Packet			*pkptr;
	int				cts_pk_size , ack_pk_size;
	
	//to get the size of RTS/CTS/ACK without a constant (if we change the format -> it will be dynamically updated here !)
	//CTS
	pkptr = op_pk_create_fmt("cmac_cts");
	cts_pk_size = op_pk_total_size_get(pkptr);
	op_pk_destroy(pkptr);
	
	//ACk pk size
	pkptr = op_pk_create_fmt("cmac_ack");
	ack_pk_size = op_pk_total_size_get(pkptr);
	op_pk_destroy(pkptr);
	
	//Exchange: SIFS / CTS / SIFS / DATA / SIFS / ACK	
	return(3 * SIFS + 3 * PROPAGATION_DELAY + ((double)cts_pk_size + (double)data_pk_size + (double)ack_pk_size ) / operational_speed);
}

//returns the time required for a whole exchange DATA / ACK (according to the data frame size)
double compute_data_ack_time(int data_pk_size){
	//Packet size
	Packet			*pkptr;
	int				ack_pk_size;

	//ACk pk size
	pkptr = op_pk_create_fmt("cmac_ack");
	ack_pk_size = op_pk_total_size_get(pkptr);
	op_pk_destroy(pkptr);
	
	//Exchange: SIFS / DATA / SIFS / ACK	
	return(2 * SIFS + 2 * PROPAGATION_DELAY + ( (double)data_pk_size + (double)ack_pk_size ) / operational_speed);
}

//returns the time for an ack
double compute_ack_time(){
	//Packet size
	Packet			*pkptr;
	int				ack_pk_size;

	//ACk pk size
	pkptr = op_pk_create_fmt("cmac_ack");
	ack_pk_size = op_pk_total_size_get(pkptr);
	op_pk_destroy(pkptr);
	
	//Exchange:  SIFS / ACK	
	return(SIFS + PROPAGATION_DELAY + (double)(ack_pk_size ) / operational_speed);
}

//returns the pk size for an ack
double compute_ack_pk_size(){
	Packet			*pkptr;
	int				ack_pk_size;

	pkptr = op_pk_create_fmt("cmac_ack");
	ack_pk_size = op_pk_total_size_get(pkptr);
	op_pk_destroy(pkptr);
	
	return(ack_pk_size);
}






//-----------------------------------------------------------
//
//			 RECEIVE FROM RADIO
//
//-----------------------------------------------------------




//Receives a frame after an interruption
void receive_packet_from_radio(){
	//Pk to receive
	Packet			*payload;
	Packet			*frame;
	//is the frame valid
	int				accepted;
	//Pk fields
	int				source , destination;
	short			frame_type;
	int				frame_id;
	int				next_hop;
	double			power_ratio;
	//CTR
	double			freq;
	double			t_slot;
	double			offset;	
	//data frame
	frame_struct	*frame_ptr;
	//hellos
	int				dist_sink , dist_border , branch;
	List			*bn_list_tmp;
	double			sync_power;
	//RTS / CTS / DATA / ACK for nav
	int				duration;
	//transmission time for the flow
	double			transmission_time;
	//next hello
	double			next_hello;
	//Control
	char			msg[500];
	

	//Gets the packet
	frame  = op_pk_get(op_intrpt_strm());

	
	//Some pk fields
	op_pk_nfd_get(frame , "Accept" , 		&accepted);
	op_pk_nfd_get(frame , "SOURCE" , 		&source);
	op_pk_nfd_get(frame , "DESTINATION" , 	&destination);
	op_pk_nfd_get(frame , "TYPE" , 			&frame_type);
	op_pk_nfd_get(frame , "Data Packet ID" ,&frame_id);
	
	

	//DEBUG
	debug_print(MEDIUM , DEBUG_NODE , "received a frame from %d to %d (type %s, id %d, accepted %d, reply_required %d , nav main freq %f)\n", source , destination , pk_type_to_str(frame_type , msg) , frame_id , accepted , is_reply_required , get_nav_main_freq());
	if ((my_address == destination) || (destination == BROADCAST)){
		debug_print(LOW , DEBUG_RECEIVE , "received a frame from %d to %d (type %s, id %d, accepted %d, reply_required %d , nav main freq %f)\n", source , destination , pk_type_to_str(frame_type , msg) , frame_id , accepted , is_reply_required , get_nav_main_freq());
		debug_print(LOW , DEBUG_NODE , "received a frame from %d to %d (type %s, id %d, accepted %d, reply_required %d , nav main freq %f)\n", source , destination , pk_type_to_str(frame_type , msg) , frame_id , accepted , is_reply_required , get_nav_main_freq());
	}
	else
	if ((my_address == destination) || (destination == BROADCAST))
		debug_print(LOW , DEBUG_RECEIVE , "received a frame from %d to %d (type %s, id %d, accepted %d, reply_required %d , nav main freq %f)\n", source , destination , pk_type_to_str(frame_type , msg) , frame_id , accepted , is_reply_required , get_nav_main_freq());
	
	
	
	//Valid frame ?
	if ((!accepted) && ((my_address == destination) || (destination == BROADCAST)) ){
		//printf("Frame -> collision (%f)\n" , op_sim_time());
	}
	
	if (accepted){
		//---------------------------------------------------------------------------
		//						REPLY RECEIVED -> BAD or VALID ?
		//---------------------------------------------------------------------------
		//Verify that if we waited a reply, this frame is a valid reply
		//i.e. it does not come from another node, or it presents a bad type....
		switch(last_frame_sent.type){		

			case RTS_PK_TYPE:
				if ((frame_type == CTS_PK_TYPE) && (destination == my_address) && (source == last_frame_sent.destination))
					is_reply_received = OPC_TRUE;
				else
					is_reply_bad = OPC_TRUE;
			break;
			
			case CTS_PK_TYPE:
				if ((frame_type == DATA_UNICAST_PK_TYPE) && (destination == my_address) && (source == last_frame_sent.destination))
					is_reply_received = OPC_TRUE;
				else
					is_reply_bad = OPC_TRUE;
			break;
			
			case DATA_UNICAST_PK_TYPE:
				if ((frame_type == ACK_PK_TYPE) && (destination == my_address) && (source == last_frame_sent.destination))
					is_reply_received = OPC_TRUE;
				else
					is_reply_bad = OPC_TRUE;
			break;
			
			case CTR_PK_TYPE:
				//ANY data frame -> ack since a border node is allowed to send a frame only when it becomes privileged
				if (((frame_type == DATA_UNICAST_PK_TYPE) || (frame_type == DATA_MULTICAST_PK_TYPE) || (frame_type == HELLO_PK_TYPE))  && (source == last_frame_sent.destination))
					is_reply_received = OPC_TRUE;
				else if ((frame_type == CTR_PK_TYPE) && (source == last_frame_sent.destination))
					is_reply_received = OPC_TRUE;
				else if ((frame_type == CTR_ACK_PK_TYPE) && (source == last_frame_sent.destination))
					is_reply_received = OPC_TRUE;
				else if ((frame_type == ACK_PK_TYPE) && (source == last_frame_sent.destination) && (destination == my_address))
					is_reply_received = OPC_TRUE;
				else
					is_reply_bad = OPC_TRUE;
						
				//We are no longer privileged
				if (is_reply_received)
					is_node_privileged = OPC_FALSE;
			break;
		
			default:
				is_reply_bad = OPC_TRUE;
			break;
		}

		
		//---------------------------------------------------------------------------
		//						WHAT TO DO AFTER THIS FRAME ?
		//---------------------------------------------------------------------------
		switch(frame_type){
			case DATA_UNICAST_PK_TYPE :
			case DATA_MULTICAST_PK_TYPE :
			
				op_pk_nfd_get(frame, "POWER_RATIO" , &power_ratio);
				
				
				//Duration of the whole exchange
				transmission_time = 0;
				if (op_pk_nfd_is_set(frame, "DURATION")){
					op_pk_nfd_get(frame , "DURATION", &duration);
					if (duration != 0)
						transmission_time = compute_data_ack_time(duration);
				}
				
					
				//If NAV & data reception -> drop the packet (I won't be able to acknowledge it !)
				if ((destination == my_address) || (destination == BROADCAST)){
				 	
					//gets the payload
				 	op_pk_nfd_get(frame , "PAYLOAD",	&payload);

					
					//Transmission to the upper layer
					if ((!is_frame_id_seen(frame_id)) && ((is_sink) || (MAC_ROUTING == ROUTING_NO)))
						op_pk_send(op_pk_copy(payload), STREAM_TO_UP);
					
					//Forwarding (max power, since we have no info)
					else if (!is_frame_id_seen(frame_id)){
						next_hop = get_next_hop();
						if (next_hop != BROADCAST)
							prepare_data_frame(payload , frame_id , next_hop , 1);					
					}
					
					else 
						printf("frame id %d dropped by %d\n", frame_id , my_address);
					
					
					//This frame is marked as seen
					add_frame_id_seen(frame_id);
					
					
					//sends an acknowledgement (unicast + no nav)
					if ((destination != BROADCAST) && (is_reply_possible(destination , my_main_frequency)))
						generate_ack(source , frame_id , duration , power_ratio);
				}
				else if (destination != my_address)
					update_nav_time(transmission_time , source , my_main_frequency , duration);
			
			break;
				
				
			//Deletes the corresponding DATA_PK from the buffer -> it was correctly transmitted
			case ACK_PK_TYPE:
			
				//Duration of the whole exchange
				transmission_time = 0;
				if (op_pk_nfd_is_set(frame, "DURATION")){
					op_pk_nfd_get(frame , "DURATION", &duration);
					if (duration != 0)
						transmission_time = compute_data_ack_time(duration);
				}
			
				//Registers the ack
				if (destination == my_address){
					del_frame_buffer_with_id(last_frame_sent.frame_id, OPC_TRUE);
				}
				
				//NaV if a reservation is present
				else if (destination != my_address)
					update_nav_time(transmission_time , source , my_main_frequency , duration);
			
			break;
			
			//We must generate a CTS
			case RTS_PK_TYPE:
			
				//Duration of the whole exchange
				op_pk_nfd_get(frame , "DURATION", 		&duration);
				op_pk_nfd_get(frame , "POWER_RATIO", 	&power_ratio);
				transmission_time = compute_cts_data_ack_time(duration);
			
				//We received a RTS for us -> we have to send now a CTS to accept the connection
				// -> No other reservation has been done by neighbors
				// -> I am not in communication (I am waiting for a reply for a transmitted frame)
				// -> If we are privileged, our privileged time has not expired
				//
				if ((destination == my_address) && (is_reply_possible(destination , my_main_frequency)) && (!is_reply_required) && (!is_node_privileged))
					generate_cts(source , duration , frame_id , power_ratio);
			

				//debug
				else if (destination == my_address)
					debug_print(LOW , DEBUG_RECEIVE , "no reply authorized: reply_possible (%d), is rep_required (%d), piviledged %d\n" , is_reply_possible(destination , my_main_frequency) , is_reply_required , is_node_privileged);
			
				//exchange : RTS - CTS - DATA - ACK
				else if (destination != my_address)
					update_nav_time(transmission_time , source , my_main_frequency , duration);

			break;
			
			//We can send the DATA -> place it in the next_frame_to_send
			//The interruption will leave the current state, and we will enter in transmission for the data frame
			case CTS_PK_TYPE:
			
				//Duration of the whole exchange
				op_pk_nfd_get(frame , "DURATION", &duration);
				transmission_time = compute_data_ack_time(duration);
				
			
				//We received a CTS for us -> we have to send now the data packet
				//NB: it is unicast because multicast frame are sent in BROADCAST, wihtout RTS / CTS
				if ((destination == my_address) && (is_reply_possible(destination , my_main_frequency)) && (!is_frame_buffer_empty())){
					frame_ptr = get_unicast_frame_buffer(0);
					if (frame_ptr != NULL)
						change_next_frame(*frame_ptr);
				}

				//A particular case : we received a CTS and meanwhile, the data packet we wanted to send was timeouted and deleted
				else if ((destination == my_address) && (is_reply_possible(destination , my_main_frequency))){
					
					debug_print(LOW, DEBUG_CONTROL , "ERROR: we received a CTS and we do not have any data frame to transmit\n");
					print_unicast_frame_buffer(DEBUG_CONTROL);
					print_multicast_frame_buffer(DEBUG_CONTROL);
				}
			
				//exchange : RTS - CTS - DATA - ACK
				else if (destination != my_address)
					update_nav_time(transmission_time , source , my_main_frequency , duration);

			break;
			
			//We become privileged node
			//-> automatically, we will send a DATA_PK (or a CTR if we have none)
			case CTR_PK_TYPE:
				op_pk_nfd_get(frame , "FREQ" , 		&freq);
				op_pk_nfd_get(frame , "T_SLOT" , 	&t_slot);
				op_pk_nfd_get(frame , "OFFSET" , 	&offset);
				
				//Multi-channel -> this border node is in privileged mode (using another frequency)
				if (nb_channels > 1)
					transmission_time 		= t_slot;
				
				//Single channel -> reserves the medium for the reply (which will be sent through this channel)
				else
					transmission_time =  SIFS + PROPAGATION_DELAY + (double)MTU_MAX / operational_speed;
				
				
				
				if ((destination == my_address) && (is_reply_possible(destination , my_main_frequency))){
					
					//the node becomes privileged
					is_node_privileged 			= OPC_TRUE;
					time_start_privileged		= op_sim_time();
					slot_privileged_duration	= t_slot;
					slot_privileged_offset		= offset;
					my_privileged_frequency 	= freq;
					
					//Error
					if (!is_border_node)
						update_is_border_node();

					
					//Schedules the end of the privileged mode
					if (slot_privileged_duration > offset){
						op_intrpt_schedule_self(op_sim_time() + (slot_privileged_duration - slot_privileged_offset) * PRIV_MIN_RATIO , 	PRIVILEGED_MIN_CODE);
						op_intrpt_schedule_self(op_sim_time() + (slot_privileged_duration - slot_privileged_offset), 					PRIVILEGED_MAX_CODE);
					}
					else
						op_intrpt_schedule_self(op_sim_time() , PRIVILEGED_MAX_CODE);
					debug_print(LOW , DEBUG_NODE , "duration : %f/%f -> min %f max %f\n",  slot_privileged_duration , slot_privileged_offset  , (slot_privileged_duration - offset) * PRIV_MIN_RATIO , (slot_privileged_duration - offset) );
					
					
					//Multi channel case (CTR_ACK -> in F1+F2 if no data)
					if (nb_channels > 1)					
						generate_ctr_ack(source , frame_id , 0);
					
					//No data frame -> ack to send (max power)
					//No ack if I have already finished my slot -> I will send a CTR which will act as a CTR-ACK
					else if ((is_frame_buffer_empty()) && ((slot_privileged_duration - slot_privileged_offset > 0) || (get_child_border_node()== BROADCAST))){
						generate_ack(source , frame_id , 0 , 1);
					}
					
				}
				
				else if (destination != my_address)
					update_nav_time(transmission_time , source , freq , 0);
					
			break;
		   	
			case CTR_ACK_PK_TYPE:
				//A node which sends a CTR-ACK will become unavailable during t_slot
				//Then, it will send a CTR to indicate that it will privilege receiver
				op_pk_nfd_get(frame , "FREQ" , 		&freq);
				op_pk_nfd_get(frame , "T_SLOT" , 	&t_slot);
				
				//And I will update this only if it comes from the 'normal' frequency
				//If it comes throug the privileged frequency -> I already set this NAV (with an higher value since I know that I is busy 2*t_slot)
				if (is_main_freq_active(STREAM_TO_RADIO))
					update_nav_time(t_slot , source , freq , 0);
			//TAG
			break;
			
			
			//Nothing to do, we just ignore the packet
			case CTR_END_PK_TYPE:			
			break;
			
			
			//Updates the neighborhood table
			case HELLO_PK_TYPE:
				op_pk_nfd_get(frame , "BRANCH" , 	&branch);
				op_pk_nfd_get(frame , "NEXT" , 		&next_hello);
			
				if (destination != BROADCAST)
					op_sim_end("An hello is sent in broadcast" , "not in unicast", "please correct the bug", "");
					
				op_pk_nfd_get(frame , "DIST_SINK" , 	&dist_sink);
				op_pk_nfd_get(frame , "DIST_BORDER" , 	&dist_border);
				op_pk_nfd_get(frame , "SYNC_POWER" , 	&sync_power);
				bn_list_tmp = create_bn_list_from_packet(frame);
				update_neighborhood_table(source , dist_sink , dist_border , sync_power , branch , bn_list_tmp , next_hello);
			
			break;
				
			//Reception of a sync frame from the sink 
			//in order to select border nodes
			case SYNC_PK_TYPE:			
				op_pk_nfd_get(frame , "BRANCH" , 	&my_branch);
			
				my_sync_rx_power = last_rx_power * 1E14;
				debug_print(LOW , DEBUG_NODE , "%d -> SYNC Level reception : %f , branch %d\n", my_address , my_sync_rx_power , my_branch);
				
			break;
				
			default:
				sprintf(msg , "TYPE %d" , frame_type);
				op_sim_end("Unknown received packet" , msg , "" , "");
			break;
		}
	}

	//This packet became useless -> destroy it !
	op_pk_destroy(frame);
}



















//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------																										  ---------------------------
//---------------------------												INTERRUPTIONS											  ---------------------------
//---------------------------																										  ---------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------















//-----------------------------------------------------------
//
//			  INTERRUPTIONS
//
//-----------------------------------------------------------



//handle all interruptions when It does not depend on the state machine
void interrupt_process(){
	//control
	char		msg[500];
	//debug
	Boolean		old;
	//reception power 
	double		new_rx_power;
	//next hello to generate
	double		next_hello;
	
	debug_print(LOW , DEBUG_STATE , "type %d (self %d, stat %d , pk %d\n", op_intrpt_type() , OPC_INTRPT_SELF , OPC_INTRPT_STAT , OPC_INTRPT_STRM);
	
	switch (op_intrpt_type()){
	
		//A PACKET WAS RECEIVED !
		case OPC_INTRPT_STRM :
			switch (op_intrpt_strm()){
				case STREAM_FROM_RADIO :
					receive_packet_from_radio();
				break;
				
				case STREAM_FROM_UP:
					receive_packet_from_up();
				break;
				
				default:
					op_sim_end("An unregistered stream sent you a packet" , "" , "" , "");
				break;
			}	
		break;
		
		//Change in the medium state
		case OPC_INTRPT_STAT :
			
			switch(op_intrpt_stat()){
				
				//signal from radio receiver (and registers the power of the last received packet)
				case STAT_FROM_RX :
					new_rx_power = op_stat_local_read(op_intrpt_stat()); 
					old = 	is_rx_busy;

					if (new_rx_power > rx_power_threshold){
						is_rx_busy 		= OPC_TRUE;
						last_rx_power 	= new_rx_power;
						debug_print(MAX, DEBUG_RADIO , "last_rx_power %f -> %f\n", last_rx_power , new_rx_power);
					}
					else
						is_rx_busy 		= OPC_FALSE;
				
					debug_print(MEDIUM , DEBUG_RADIO , "rx_busy %d -> %d (rx pow %f)\n", old , is_rx_busy , new_rx_power * 1E14);
				break;
				
				//Signal from my busy tone receiver	
				case STAT_FROM_RX_BUSY_TONE :
				
					new_rx_power = op_stat_local_read(op_intrpt_stat()); 
					old = is_busy_tone_rx;
					is_busy_tone_rx	= (new_rx_power > rx_power_threshold);	
					if (old != is_busy_tone_rx)
						debug_print(LOW , DEBUG_RADIO , "busy tone %d -> %d (power %f, threshold %f)\n", old , is_busy_tone_rx , new_rx_power*1E14 , rx_power_threshold*1E14);
					
				break;
				
				//2 transmitters but considered logicaly as one single transmitter with a different antenna
				case STAT_FROM_TX_DIREC_SYNC:
				case STAT_FROM_TX :
					old = is_tx_busy ;				
				
					is_tx_busy = (op_stat_local_read(op_intrpt_stat()) == 1.0);
				
					debug_print(MEDIUM , DEBUG_RADIO , "tx_busy %d -> %d\n", old , is_tx_busy);
				break;
				
				//End of busy tone -> we send another packet
				case STAT_FROM_TX_BUSY_TONE:
					
				
					if ((op_stat_local_read(op_intrpt_stat()) != 1.0) && (is_busy_tone_tx)){
						maintain_busy_tone(1E-6);						
				
						debug_print(MEDIUM , DEBUG_RADIO , "keeps on maintaining activity on the busy_tone tx\n");
					}
				break;
					
				default :
					sprintf(msg, "We are not configured to handle the stat from the line %d\n", op_intrpt_stat());
					op_sim_end(msg, "" , "" , "");
			}
		break;
		
		//SELF INTERRUPTS
		case OPC_INTRPT_SELF:
		
			//CTR from the sink
			if (op_intrpt_code() == SINK_CTR_CODE){
				generate_ctr();
				
				//The next CTR from the sink (if several channels, the period of CTR should be reduced)
				if (nb_channels > 1)
					op_intrpt_schedule_self(op_sim_time() + slot_privileged_duration * BETA /  (nb_channels - 1) + MAX_CTR_DELAY_FROM_SINK, SINK_CTR_CODE);
				else
					op_intrpt_schedule_self(op_sim_time() + slot_privileged_duration * BETA + MAX_CTR_DELAY_FROM_SINK, 						SINK_CTR_CODE);
			}

			//SYNC from the sink (if we have a dynamical election of border nodes)
			if ((op_intrpt_code() == SINK_SYNC_CODE) && (is_sink)){
				switch (border_election){
					case BORDER_CENTRALIZED :
						compute_border_nodes_status();
					break;
				
					case BORDER_DYNAMIC :
						generate_sync();
					break;
				}
			}

		
			//HELLO
			if (op_intrpt_code() == HELLO_PK_CODE){
				//The next HELLO
				if(op_sim_time() < 60.0 - INTERVALL_HELLO / 10)
					next_hello = INTERVALL_HELLO / 10 - op_dist_uniform(1);
				else
					next_hello = INTERVALL_HELLO - op_dist_uniform(1);
				
				//prepares the packet
				generate_hello(next_hello);
				
				//schedules the next interruption
				op_intrpt_schedule_self(op_sim_time() + next_hello , HELLO_PK_CODE);
			}
			
			
			//privileged END !
			if (op_intrpt_code() == PRIVILEGED_MAX_CODE){
			
				//End interruption (we have also a min_time interruption)
				if ((is_node_privileged) && (time_start_privileged + slot_privileged_duration <= op_sim_time()))
					generate_ctr();
			}
			
			//Returns to the main frequency
			if (op_intrpt_code() == MAIN_FREQ_RETURN_CODE)			
				change_tx_rx_freq(my_main_frequency , my_main_bandwidth , STREAM_TO_RADIO);
		
			
			
		break;
		
		//Stats, debug ...
		case OPC_INTRPT_ENDSIM :
			//direct function
			end_sim();
		break;
	
		
		//no default -> some interruptions can be treated directly by a particular state
	}
}



//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------																										  ---------------------------
//---------------------------												STATISTICS												  ---------------------------
//---------------------------																										  ---------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------





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











//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------																										  ---------------------------
//---------------------------												DEBUG													  ---------------------------
//---------------------------																										  ---------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------







//-----------------------------------------------------------
//
//			   			TOOLS
//
//-----------------------------------------------------------



//infinity conversion for debug messages
double convert_double(double value){
	if (value == OPC_DBL_INFINITY)
		return(-1);
	else
		return(value);
}

//infinity conversion for debug messages
int convert_int(int value){
	if (value == OPC_INT_INFINITY)
		return(-1);
	else
		return(value);
}



//-----------------------------------------------------------
//
//			   DEBUG FUNCTIONS
//
//-----------------------------------------------------------

//Converts a pk_type in string
char* pk_type_to_str(short pk_type , char *msg){

	switch(pk_type){
		case RTS_PK_TYPE:
			sprintf(msg, "RTS");
		break;
		case CTS_PK_TYPE:
			sprintf(msg, "CTS");
		break;
		case CTR_PK_TYPE:
			sprintf(msg, "CTR");
		break;
		case CTR_ACK_PK_TYPE:
			sprintf(msg, "CTR-ACK");
		break;
		case CTR_END_PK_TYPE:
			sprintf(msg, "CTR-END");
		break;
		case ACK_PK_TYPE:
			sprintf(msg, "ACK");
		break;
		case DATA_UNICAST_PK_TYPE:
			sprintf(msg, "DATA-UNI");
		break;
		case DATA_MULTICAST_PK_TYPE:
			sprintf(msg, "DATA-MULTI");
		break;
		case HELLO_PK_TYPE:
			sprintf(msg, "HELLO");
		break;
		case SYNC_PK_TYPE:
			sprintf(msg, "SYNC");
		break;
		default:
			sprintf(msg, "%d unknown", pk_type);
		break;
	}

	return(msg);
}

// Print all debug messages classified in different files
void debug_print(const int level, const int type , const char* fmt, ...){	
	FILE		*pfile;
	va_list 	argptr;
	
	//Prepares the args
	va_start(argptr, fmt);	
	
	//Particular case of my_debug_file (ths file is not shared with other nodes)
	if (type != DEBUG_NODE)
		pfile = debug_files[type];
	else
		pfile = my_debug_file;
	
	
	if (level <= DEBUG)	{			
		//Normal debug
		if (type != DEBUG_STATE){
			fprintf(pfile , "[%4ds , "	, (int) floor(op_sim_time()));
			fprintf(pfile , "%4dms , "	, (int) (floor(op_sim_time() * 1E3) - 1E3 * floor(op_sim_time())));
			fprintf(pfile , "%4dus , "	, (int) (floor(op_sim_time() * 1E6) - 1E3 * floor(op_sim_time() * 1E3)));
			fprintf(pfile , "%4.1i] "	, my_address);	
			vfprintf(pfile , fmt, argptr);     
		}
		
		
		//print on the console some particular debug messages 
		if (OPC_FALSE){
			printf("[%4ds , "	, (int) floor(op_sim_time()));
			printf("%4dms , "	, (int) (floor(op_sim_time() * 1E3) - 1E3 * floor(op_sim_time())));
			printf("%4dus , "	, (int) (floor(op_sim_time() * 1E6) - 1E3 * floor(op_sim_time() * 1E3)));
			printf("%4.1i] "	, my_address);	
			vprintf(fmt, argptr);   
		}
		
		if (OPC_FALSE){ //((type == DEBUG_SEND) || (type == DEBUG_RECEIVE)){
			fprintf(my_debug_file , "[%4ds , "	, (int) floor(op_sim_time()));
			fprintf(my_debug_file , "%4dms , "	, (int) (floor(op_sim_time() * 1E3) - 1E3 * floor(op_sim_time())));
			fprintf(my_debug_file , "%4dus , "	, (int) (floor(op_sim_time() * 1E6) - 1E3 * floor(op_sim_time() * 1E3)));
			fprintf(my_debug_file , "%4.1i] "	, my_address);	
			vfprintf(my_debug_file , fmt, argptr);     
		}
		
		//put all debugs in a gobal file (useful sometimes to see all the events)
		if (OPC_FALSE){
			fprintf(debug_files[DEBUG_GLOBAL] , "[%4ds , "	, (int) floor(op_sim_time()));
			fprintf(debug_files[DEBUG_GLOBAL] , "%4dms , "	, (int) (floor(op_sim_time() * 1E3) - 1E3 * floor(op_sim_time())));
			fprintf(debug_files[DEBUG_GLOBAL] , "%4dus , "	, (int) (floor(op_sim_time() * 1E6) - 1E3 * floor(op_sim_time() * 1E3)));
			fprintf(debug_files[DEBUG_GLOBAL] , "%4.1i] "	, my_address);	
			vfprintf(debug_files[DEBUG_GLOBAL] , fmt, argptr);     
		}
		
		//fush the printed messages (for all opened file pointers)
		fflush(NULL);	
	}
	
	va_end(argptr);
}	



//opens a debug file
void open_debug_file(const FILE **fout , const char* fmt, ...){	
	char	filename[500];
	char	msg[500];	
	va_list argptr;
	
	//The file was already opened -> nothing to do
	if (*fout != NULL)
		return;
	
	//Args
	va_start(argptr, fmt);
	
	//gets the filename
	strcpy(filename, "");
	vsprintf(filename , fmt, argptr);     
	
	//And open it !
	*fout = fopen(filename, "w");
	if (*fout == NULL){
		sprintf(msg, "%s -> %s", filename , strerror(errno));
		op_sim_end("I can not open the debug file", msg , "" , "");	
	}	
	
	va_end(argptr);
}












//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------																										  ---------------------------
//---------------------------												END SIMULATION											  ---------------------------
//---------------------------																										  ---------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------

//end simulation (debug + eventual stats)
void end_sim(){
	int		i;
	
	if (DEBUG > NO){
	
		//Debug message
		print_neighborhood_table(DEBUG_NODE);
		print_frame_buffer(DEBUG_NODE);

		
		//Close common debug files !
		if (my_stat_id == 0)
			for (i=0 ; i < 20 ; i++)
				if (debug_files[i] != NULL)
					fclose(debug_files[i]);

		//and the debug file reserved for this particular node
		if (my_debug_file != NULL)
			fclose(my_debug_file);
		
	}

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
	void cmac_process (void);
	Compcode cmac_process_init (void **);
	void cmac_process_diag (void);
	void cmac_process_terminate (void);
	void cmac_process_svar (void *, const char *, char **);
#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
cmac_process (void)
	{
	int _block_origin = 0;
	FIN (cmac_process ());
	if (1)
		{


		FSM_ENTER (cmac_process)

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (INIT) enter executives **/
			FSM_STATE_ENTER_UNFORCED_NOLABEL (0, "INIT", "cmac_process () [INIT enter execs]")
				{
				//Ids
				Objid	mac_params_comp_attr_objid;
				Objid	params_attr_objid;
				//Parameters
				Objid	chann_params_comp_attr_objid;
				Objid	subchann_params_attr_objid;
				//transmitters
				int		nb_radio;
				//Channels
				int		num_chann;
				Objid	chann_objid;
				Objid	sub_chann_objid;
				Objid	tx_id , rx_id;
				//Control
				int		i , j;
				char	str[300];
				char	msg[500];
				//Statwires
				Objid	statwire_objid;
				int		num_statwires;
				double	threshold;
				//Filenames
				char	filename[200];
				//Position, routes...
				int		x_int , y_int , x_sink, y_sink;
				int		sink_destination;
				//Id
				int		intf_id;
				//positions
				pos_struct	pos;
				
				
				
					
				//-----------------------------------------------
				//		   		IDENTIFICATION
				//-----------------------------------------------
				
				
				//Process IDS 	
				my_stat_id		= nb_mac_nodes++;
				
				
				if (cmac_timestamp == 0){
					cmac_timestamp = time(NULL);
					global_border_nodes_list = op_prg_list_create();
					positions_list = op_prg_list_create();
				}
				
				
				
				
				//Address
				op_ima_obj_attr_get(op_topo_parent(op_id_self()) , "name" , str);
				my_address = atoi(str);
				op_ima_obj_attr_set (op_id_self(), "Address", &my_address);
				
				
				
				
				//-----------------------------------------------
				//		   			ROUTING
				//-----------------------------------------------
				
				
				
				
				
				//-------------  POSITIONS  -----------------
				
				y_int = my_address /  100;
				x_int = my_address - y_int * 100;
				
				op_ima_obj_attr_get(op_topo_parent(op_id_self()) , "wlan_mac_intf.Destination" , &sink_destination);
				y_sink = (int)(sink_destination /  100);
				x_sink = (int)(sink_destination - y_sink * 100);
				
				
				
				
				
				//-----------  IS_BORDER_NODE  --------------
				
				is_border_node = OPC_FALSE;
				
				
				
				//--------------  IS_SINK  -----------------
				
				//The upper process is the first one since I am the lowest layer
				intf_id = op_topo_assoc(op_id_self() , OPC_TOPO_ASSOC_OUT , OPC_OBJTYPE_PROC , 0);
				if (op_topo_assoc_count(intf_id , OPC_TOPO_ASSOC_OUT , OPC_OBJTYPE_STRM) > 1)
					is_sink = OPC_TRUE;
				if (is_sink){
					is_border_node 		= OPC_TRUE;
					my_sync_rx_power 	= OPC_DBL_INFINITY;
				}
				
				
				
				//---------  DISTANCES / ROUTING  ------------
				
				if (is_sink)
					my_dist_sink = 0;
				else
					my_dist_sink = pow (2, 4) - 1;
				
				//dist border
				if (is_border_node)
					my_dist_border = 0;
				else
					my_dist_border = pow (2, 4) - 1;
				
				
				
				
				
				
				//-----------------------------------------------
				//		  		 PARAMETERS
				//-----------------------------------------------
				
				//Parameters
				op_ima_sim_attr_get(OPC_IMA_INTEGER , "DEBUG",					&DEBUG);
				op_ima_sim_attr_get(OPC_IMA_INTEGER , "BETA",					&BETA);
				op_ima_sim_attr_get(OPC_IMA_INTEGER , "RTS",					&RTS_PK_SIZE);
				op_ima_sim_attr_get(OPC_IMA_INTEGER , "CTR",					&is_ctr_activated);
				op_ima_sim_attr_get(OPC_IMA_INTEGER , "CHANNELS",				&nb_channels);
				op_ima_sim_attr_get(OPC_IMA_INTEGER , "ROUTING_MAC",			&MAC_ROUTING);
				op_ima_sim_attr_get(OPC_IMA_DOUBLE ,  "RADIO_RANGE",			&RADIO_RANGE);
				op_ima_sim_attr_get(OPC_IMA_INTEGER , "BORDER_ELECTION",	   	&border_election);
				
				//No RTS / CTS -> pk_size set to the infinity value
				if (RTS_PK_SIZE == 99999)
					RTS_PK_SIZE = OPC_INT_INFINITY;
				
				
				if (is_sink){
					op_ima_sim_attr_get(OPC_IMA_DOUBLE ,  "PRIVILEGED_MAX_TIME",	&slot_privileged_duration);
					TIME_MAX_PRIVILEGED = slot_privileged_duration * 2;
				}
				else
					slot_privileged_duration 	= 0;
				
				op_ima_obj_attr_get(op_id_self() ,  "Transmission Power",	&POWER_TX);
				
				
				//A border node is allowed to send packets only when it is privileged
				strict_privileged_mode = is_ctr_activated;
				
				//The busy tone is not activated if RTS are not used
				//NB : the busy tone is currently descativated (no better performances for a more complex system)
				BUSY_TONE_ACTIVATED = OPC_FALSE && (RTS_PK_SIZE < MTU_MAX);
				
				
				
				
				
				
				//-----------------------------------------------
				//		   		PROPERTIES
				//-----------------------------------------------
				
				is_node_privileged			= OPC_FALSE;
				last_frame_sent.type		= NO_PK_TYPE;
				next_frame_to_send.type		= NO_PK_TYPE;
				
				unicast_frame_buffer		= op_prg_list_create();
				multicast_frame_buffer		= op_prg_list_create();
				my_neighborhood_table		= op_prg_list_create();
				my_frame_id_seen			= op_prg_list_create();
				bn_list						= op_prg_list_create();
				my_nav_list					= op_prg_list_create();
				
				
				
				//Backoff distribution initialization
				cw 							= MAX_BACKOFF;
				backoff_dist 				= op_dist_load("uniform_int" , 0 , cw);
				
				//no reservation
				is_reply_required 			= OPC_FALSE;
				
				//Power of the last received sync frame
				my_sync_rx_power			= 0;
				
				//Power of the last received packet (when rx busy -> !busy)
				last_rx_power				= 0;
				busy_tone_rx_ignored		= OPC_FALSE;
				
				//next packet == hello
				is_hello_to_send 			= OPC_FALSE;
				
				//Verification of the next start time for data frames (in the source process)
				next_start_time				= 0;
				last_next_start_time_verif 	= 0;
				
				//Branches id
				ctr_last_branch 			= 0;
				sync_last_branch			= 0;
				my_branch					= -1;
				
				
				
				
				
				
				
				
				
				
				
				
				
				//-----------------------------------------------
				//		  				 DEBUG
				//-----------------------------------------------
				
				if (my_stat_id == 0)
					for(i=0 ; i < 20 ; i++)
						debug_files[i] = NULL;
				
				if (DEBUG > NO){
					if (my_stat_id == 0){
						open_debug_file(&debug_files[DEBUG_GLOBAL] ,	"results_cmac/%d_cmac_debug.txt" , 			cmac_timestamp);
						open_debug_file(&debug_files[DEBUG_STATE] ,		"results_cmac/%d_cmac_debug_state.txt" , 	cmac_timestamp);
						open_debug_file(&debug_files[DEBUG_BACKOFF] ,	"results_cmac/%d_cmac_debug_backoff.txt" , 	cmac_timestamp);
						open_debug_file(&debug_files[DEBUG_SEND] ,		"results_cmac/%d_cmac_debug_send.txt" , 	cmac_timestamp);
						open_debug_file(&debug_files[DEBUG_RECEIVE] ,	"results_cmac/%d_cmac_debug_receive.txt" , 	cmac_timestamp);
						open_debug_file(&debug_files[DEBUG_RADIO] ,		"results_cmac/%d_cmac_debug_radio.txt" , 	cmac_timestamp);
						open_debug_file(&debug_files[DEBUG_CONTROL] ,	"results_cmac/%d_cmac_debug_control.txt" , 	cmac_timestamp);
						open_debug_file(&debug_files[DEBUG_TIMEOUT] ,	"results_cmac/%d_cmac_debug_timeout.txt" , 	cmac_timestamp);
						open_debug_file(&debug_files[DEBUG_HELLO] ,		"results_cmac/%d_cmac_debug_hello.txt" , 	cmac_timestamp);
						open_debug_file(&debug_files[DEBUG_UP] ,		"results_cmac/%d_cmac_debug_up.txt" , 		cmac_timestamp);
					}
					open_debug_file(&my_debug_file,					"results_cmac/nodes/%d.txt" , 				my_address);
						
				}
				
				
				
				
				//-----------------------------------------------
				//		  	 	TRANSMISSION
				//-----------------------------------------------
				
				
				//RADIO Parameters
				op_ima_obj_attr_get (op_id_self(), "Wireless LAN Parameters", &mac_params_comp_attr_objid);
				params_attr_objid = op_topo_child (mac_params_comp_attr_objid, OPC_OBJTYPE_GENERIC, 0);
				
					
				//Parameters
				op_ima_obj_attr_get (params_attr_objid, "Data Rate", 						&operational_speed);
				
				//Power reception (to detect radio activity)
				op_ima_obj_attr_get (params_attr_objid, "Packet Reception-Power Threshold", &rx_power_threshold);
				
				//Channel
				op_ima_obj_attr_get (params_attr_objid, "Channel Settings", &chann_params_comp_attr_objid);
				subchann_params_attr_objid = op_topo_child (chann_params_comp_attr_objid, OPC_OBJTYPE_GENERIC, 0);
				op_ima_obj_attr_get (subchann_params_attr_objid, "Bandwidth", 				&my_main_bandwidth);	
				op_ima_obj_attr_get (subchann_params_attr_objid, "Min frequency", 			&my_main_frequency);	
				
				
				if ((!is_ctr_activated) && (strict_privileged_mode))
					op_sim_end("We can not desactivate the CTR" , "and function in BLOCKED_MODE" , "" , "");
				
				
				
				//-----------------------------------------------
				//				NORMAL RADIO
				//-----------------------------------------------
				
				
				//----- TRANSMISSION ----
				
				nb_radio = op_topo_assoc_count(op_id_self() , OPC_TOPO_ASSOC_OUT, OPC_OBJTYPE_RATX);
				
				for (i=0; i<nb_radio ; i++){
					
				
					tx_id = op_topo_assoc (op_id_self(), OPC_TOPO_ASSOC_OUT, OPC_OBJTYPE_RATX, i);
					if (tx_id == OPC_OBJID_INVALID)
						op_sim_end("No attached transmitter\n", "" , "" , "");
				
					//Channels nb
					op_ima_obj_attr_get (tx_id, "channel", &chann_objid);
				
					//id access
					sub_chann_objid = op_topo_child (chann_objid, OPC_OBJTYPE_RATXCH, 0);
				
					//Frequency + bandwidth
					op_ima_obj_attr_set (sub_chann_objid, "bandwidth", 		my_main_bandwidth);
					op_ima_obj_attr_set (sub_chann_objid, "min frequency", 	my_main_frequency);
					op_ima_obj_attr_set (sub_chann_objid, "power", 			POWER_TX);
				
				}
				
				my_current_tx_power = POWER_TX ;
				
				
				//----- RECEPTION ----
				
				nb_radio = op_topo_assoc_count(op_id_self() , OPC_TOPO_ASSOC_IN, OPC_OBJTYPE_RARX);
				
				for (i=0; i<nb_radio ; i++){
				
					rx_id = op_topo_assoc (op_id_self(), OPC_TOPO_ASSOC_IN, OPC_OBJTYPE_RARX, i);
					if (rx_id == OPC_OBJID_INVALID)
						op_sim_end("No attached receiver\n", "" , "" , "");
				
				
					//Nb channels
					op_ima_obj_attr_get (rx_id, "channel", &chann_objid);
					num_chann = op_topo_child_count (chann_objid, OPC_OBJTYPE_RARXCH);
					
				
					//Frequency + bandwidth of the first received
					for (j = 0; j < num_chann; j++){ 	
						//Id
						sub_chann_objid = op_topo_child (chann_objid, OPC_OBJTYPE_RARXCH, j);
					
						//Frequency + bandwidth
						op_ima_obj_attr_set (sub_chann_objid, "bandwidth", 		my_main_bandwidth);
						op_ima_obj_attr_set (sub_chann_objid, "min frequency", 	my_main_frequency);		
						
						//Reception power threshold
						op_ima_obj_state_set (sub_chann_objid, &rx_power_threshold);
					}
				}
				
				
				//-----------------------------------------------
				//				BUSY TONE
				//-----------------------------------------------
				
				//Such that one bit is transmitted in 1us
				busy_tone_speed = 1E6;
				
				if (OPC_TRUE){
				
				//Transmission
					tx_id = op_topo_assoc (op_id_self(), OPC_TOPO_ASSOC_OUT, OPC_OBJTYPE_RATX, STREAM_TO_BUSY_TONE);
					if (tx_id == OPC_OBJID_INVALID)
						op_sim_end("No attached transmitter\n", "" , "" , "");
				
					//Channels nb
					op_ima_obj_attr_get (tx_id, "channel", &chann_objid);
				
					//id access
					sub_chann_objid = op_topo_child (chann_objid, OPC_OBJTYPE_RATXCH, 0);
				
					//Speed
					op_ima_obj_attr_set (sub_chann_objid, "data rate", 		busy_tone_speed);
					op_ima_obj_attr_set (sub_chann_objid, "bandwidth", 		my_main_bandwidth * busy_tone_speed / operational_speed);
					op_ima_obj_attr_set (sub_chann_objid, "min frequency", 	my_main_frequency - SHIFT_FREQ_SEPARATION);
					op_ima_obj_attr_set (sub_chann_objid, "power", 			POWER_TX);
					
					//NB: bandwidth in KHz, frequency in MHz
					if (my_main_bandwidth * busy_tone_speed / operational_speed >= 1000 * SHIFT_FREQ_SEPARATION)
						op_sim_end("The bandiwthd separation between the busy tone " , "and the principal radio is too small." , "Please increase the value of SHIFT_FREQ_BUSY_TONE", "to separate sufficiently the channels");
				
					
				//reception	
					rx_id = op_topo_assoc (op_id_self(), OPC_TOPO_ASSOC_IN, OPC_OBJTYPE_RARX, STREAM_FROM_BUSY_TONE);
					if (rx_id == OPC_OBJID_INVALID)
						op_sim_end("No attached transmitter\n", "" , "" , "");
				
					//Channels nb
					op_ima_obj_attr_get (rx_id, "channel", &chann_objid);
				
					//id access
					sub_chann_objid = op_topo_child (chann_objid, OPC_OBJTYPE_RARXCH, 0);
				
					//Speed
					op_ima_obj_attr_set (sub_chann_objid, "bandwidth", 		my_main_bandwidth * busy_tone_speed / operational_speed);
					op_ima_obj_attr_set (sub_chann_objid, "min frequency", 	my_main_frequency - SHIFT_FREQ_SEPARATION);
				}
				
				
				
				
				
				
				
				
				//-----------------------------------------------
				//		   			STATWIRES
				//-----------------------------------------------
				
				//for all statswirs (tx, rx, busy tone, normal ...)
				num_statwires = op_topo_assoc_count (op_id_self(), OPC_TOPO_ASSOC_IN, OPC_OBJTYPE_STATWIRE);
				for (i = 0; i < num_statwires; i++){
					//Id														*/
					statwire_objid = op_topo_assoc (op_id_self(), OPC_TOPO_ASSOC_IN, OPC_OBJTYPE_STATWIRE, i);
					op_ima_obj_attr_get (statwire_objid, "high threshold trigger", &threshold);
					
					//value
					op_ima_obj_attr_set (statwire_objid, "high threshold trigger", rx_power_threshold);	
				}
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				//-----------------------------------------------
				//		   	SYNCHRO WITH OTHER LAYERS
				//-----------------------------------------------
				
				op_intrpt_schedule_self (op_sim_time (), 0);
				
				
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (1,cmac_process)


			/** state (INIT) exit executives **/
			FSM_STATE_EXIT_UNFORCED (0, "INIT", "cmac_process () [INIT exit execs]")
				{
				
				
				}


			/** state (INIT) transition processing **/
			FSM_TRANSIT_FORCE (2, state2_enter_exec, ;, "default", "", "INIT", "INFO_SYNCHRO")
				/*---------------------------------------------------------*/



			/** state (IDLE) enter executives **/
			FSM_STATE_ENTER_UNFORCED (1, state1_enter_exec, "IDLE", "cmac_process () [IDLE enter execs]")
				{
				
				
				
				
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,cmac_process)


			/** state (IDLE) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "IDLE", "cmac_process () [IDLE exit execs]")
				{
				debug_print(LOW , DEBUG_STATE , "ENTER - IDLE2 - %d\n", my_address);
				
				//Handles the possible interruptions (STREAM || STAT)
				interrupt_process();
				
				
				debug_print(LOW , DEBUG_STATE , "EXIT - IDLE2 - %d\n", my_address);
				
				
				}


			/** state (IDLE) transition processing **/
			FSM_INIT_COND (IS_PK_TO_SEND || PRIVILEGED_END)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("IDLE")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 4, state4_enter_exec, ;, "IS_PK_TO_SEND || PRIVILEGED_END", "", "IDLE", "DEFER")
				FSM_CASE_TRANSIT (1, 1, state1_enter_exec, ;, "default", "", "IDLE", "IDLE")
				}
				/*---------------------------------------------------------*/



			/** state (INFO_SYNCHRO) enter executives **/
			FSM_STATE_ENTER_UNFORCED (2, state2_enter_exec, "INFO_SYNCHRO", "cmac_process () [INFO_SYNCHRO enter execs]")
				{
				/* Schedule a self interrupt to wait for mac interface 	*/
				/* to move to next state after registering				*/
				op_intrpt_schedule_self (op_sim_time (), 0);
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (5,cmac_process)


			/** state (INFO_SYNCHRO) exit executives **/
			FSM_STATE_EXIT_UNFORCED (2, "INFO_SYNCHRO", "cmac_process () [INFO_SYNCHRO exit execs]")
				{
				Objid	mac_layer_intf_id;
				int		*int_ptr;
				List	**list_ptr_tmp;
				
				
				
				//----------------------------------------------
				//					BRANCH LENGTH
				//----------------------------------------------
				
				
				if (my_stat_id == 0)
					MAX_BRANCH_LENGTH = min_int(BETA + 1 , (int)(pow(nb_mac_nodes , 0.5) / 2));
				
				
				
				//----------------------------------------------
				//					SINK 
				//----------------------------------------------
				
				//The sink must send periodically a CTR (create it, not forward it)
				//NB: let the routing info converge (dist_sink and dist_border)
				if (is_sink){
					if (is_ctr_activated)
						op_intrpt_schedule_self(op_sim_time() + 5.0 + INTERVALL_HELLO * 1.1 , SINK_CTR_CODE); 
				
					op_intrpt_schedule_self(op_sim_time() + 0.1 , SINK_SYNC_CODE); 
				}
				
				
				
				
				//----------------------------------------------
				//					HELLOS 
				//----------------------------------------------
				
				//periodical hello for everyone (initally, shorter hello_interval to decrease convergence delays)
				op_intrpt_schedule_self(op_sim_time() + 5.0 + op_dist_uniform(3.0) , HELLO_PK_CODE);
				}


			/** state (INFO_SYNCHRO) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "INFO_SYNCHRO", "IDLE")
				/*---------------------------------------------------------*/



			/** state (EMISSION) enter executives **/
			FSM_STATE_ENTER_UNFORCED (3, state3_enter_exec, "EMISSION", "cmac_process () [EMISSION enter execs]")
				{
				//----------------------------------------------------------------
				//
				//				FRAME TRANSMISSION
				//
				// - The medium is free
				// - The backoff was decremented or not required
				// - The frame 'next_frame_to_send' is ready to be transmitted
				//
				//-----------------------------------------------------------------
				
				//Control
				char		msg[500];
				//Packet
				Packet*		frame_pk;
				
				debug_print(LOW , DEBUG_STATE , "ENTER - EMISSION1 - %d\n", my_address);
				
				
				//------------------------------------------
				//			  TRANSMISSION
				//------------------------------------------
				
				//The transmitter is not busy -> we are not in transmission -> we send our packet
				//- We have a default state self-transition (if the medium keeps on being used)
				//  Consequently we must avoid to send the same frame although the current frame is already in transmission
				if (!is_tx_busy){
				
					//debug
					debug_print(LOW , DEBUG_SEND , "sends a packet to %d (type %s, id %d, my_nav %f, busy %d, power_ratio %f, freq %f)\n",  next_frame_to_send.destination , pk_type_to_str(next_frame_to_send.type , msg) , next_frame_to_send.frame_id , get_nav_main_freq() , IS_MEDIUM_BUSY , next_frame_to_send.power_ratio);
					debug_print(LOW , DEBUG_NODE , "sends a packet to %d (type %s, id %d, my_nav %f, busy %d, power_ratio %f, freq %f)\n",  next_frame_to_send.destination , pk_type_to_str(next_frame_to_send.type , msg) , next_frame_to_send.frame_id , get_nav_main_freq() , IS_MEDIUM_BUSY , next_frame_to_send.power_ratio);
				
				
					//Prepares the packet
					is_tx_busy = OPC_TRUE;
					frame_pk = create_packet(next_frame_to_send);
					
					
					//The reply was sent (if it was a reply!)
					is_reply_required = OPC_TRUE;
				
					
					// ---- TRANSMISSION POWER ----
					
					/*if ((my_current_tx_power != next_frame_to_send.power_ratio) && (next_frame_to_send.type != SYNC_PK_TYPE)){
						change_tx_power(POWER_TX * next_frame_to_send.power_ratio , STREAM_TO_RADIO);
						my_current_tx_power = next_frame_to_send.power_ratio;
					}
					*/
					
					// -----  SPECIAL ACTIONS  -----
				
					
					if (next_frame_to_send.destination == BROADCAST)
						print_nav_list(DEBUG_HELLO);
					
					switch (next_frame_to_send.type){
					
						//Ack sent -> no more busy_tone required
						case ACK_PK_TYPE:
							is_busy_tone_tx = OPC_FALSE;
						break;
						
						//RTS sent -> we ignore further busy tone in reception
						case RTS_PK_TYPE:
							busy_tone_rx_ignored = OPC_TRUE;
						break;
						
						//CTS sent -> we activate the busy tone
						case CTS_PK_TYPE:
							maintain_busy_tone(SIFS);
						break;
						
						//Increases power (to 10W, that is enormous and all the nodes will get it)
						//If a node does not receive the SYNC it is not a problem: the branch will not expand to the network extremities
						case SYNC_PK_TYPE:
					
							if (is_sync_direct_antenna)	
								change_antenna_direction(STREAM_TO_DIRECT_SYNC , sync_last_branch);
							else
								change_tx_power(1000 , STREAM_TO_RADIO);
						break;
						
						case HELLO_PK_TYPE:
							is_hello_to_send = OPC_FALSE;
						break;
					}
					
					
					
					// -----  TRANSMISSSION  -----
					if ((next_frame_to_send.type == SYNC_PK_TYPE) && (is_sync_direct_antenna))
						op_pk_send(frame_pk , STREAM_TO_DIRECT_SYNC);
					else 
						op_pk_send(frame_pk , STREAM_TO_RADIO);
				
					
					//Registers the frame as sent (if a reply is required: ACK/CTS/...)
					last_frame_sent = next_frame_to_send;
					last_frame_sent.released = OPC_FALSE;
					set_next_frame_null();
				}
				//else -> nothing, we just stay in this state while the transmission and keep to treat other interrupts
				
				
				
				
				
				
				
				
				
				//Timeouts -> an interruption will be scheduled when we finished the transmission (the medium becomes idle)
				
				debug_print(LOW , DEBUG_STATE , "EXIT - EMISSION1 - %d\n", my_address);
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (7,cmac_process)


			/** state (EMISSION) exit executives **/
			FSM_STATE_EXIT_UNFORCED (3, "EMISSION", "cmac_process () [EMISSION exit execs]")
				{
				
				
				debug_print(MAX , DEBUG_STATE , "ENTER - EMISSION2 - %d\n", my_address);
				
				
				//Handles the possible interruptions (STREAM || STAT)
				debug_print(LOW , DEBUG_STATE , "EMISSION\n");
				interrupt_process();
				
				
				
				
					
					
				//Transmission finished !
				if (!is_tx_busy){
					debug_print(LOW , DEBUG_SEND , "transmission ended\n");
				
				
					//-----------------------------------------------------
					//	NORMAL TRANSMISSION POWER
					//-----------------------------------------------------
				
				
					//Decrease the power to a normal value
					if (last_frame_sent.type == SYNC_PK_TYPE){
						
						//normal power transmission
						if (!is_sync_direct_antenna)
							change_tx_power(POWER_TX , STREAM_TO_RADIO);
						
						//Next sync if we have a directional antenna
						if ((sync_last_branch != MAX_NB_BRANCHES - 1) && (is_sync_direct_antenna)){
							sync_last_branch++;
							generate_sync();
						}
						else
							sync_last_branch = 0;
					}
						
					
					
					//-----------------------------------------------------
					//	MULTI_CHANNEL -> COMMUTES
					//-----------------------------------------------------
				
					if (nb_channels > 1){
					
						//CTR sent -> changes to the privileged channel
						if (last_frame_sent.type == CTR_PK_TYPE){
							change_tx_rx_freq(my_privileged_frequency , my_main_bandwidth * RATIO_PRIV_BANDWIDTH , STREAM_TO_RADIO);
							
							if (op_ev_valid(main_freq_return_intrpt))
								op_ev_cancel(main_freq_return_intrpt);
							main_freq_return_intrpt = op_intrpt_schedule_self(op_sim_time() + slot_privileged_duration , MAIN_FREQ_RETURN_CODE);
						}
						
						
						//CTR-ACK sent -> changes to the privileged channel
						if ((last_frame_sent.type == CTR_ACK_PK_TYPE) && (last_frame_sent.nb_retry == 0)){
						
							//Channel change
							change_tx_rx_freq(my_privileged_frequency , my_main_bandwidth * RATIO_PRIV_BANDWIDTH , STREAM_TO_RADIO);
							
							//sends a CTR-ACK through the privileged channel if no data frame to send
							if (is_frame_buffer_empty()){
								last_frame_sent.nb_retry++;
								change_next_frame(last_frame_sent);
							}
							
						}
						
						
						
					}
						
						
					//-----------------------------------------------------
					//	Must the node wait a reply for this frame ?
					//-----------------------------------------------------
				
					//BROADCAST Frame 
					if (last_frame_sent.destination == BROADCAST)
						is_reply_required = OPC_FALSE;
					
					//Ack Frame
					else if (last_frame_sent.type == ACK_PK_TYPE)
						is_reply_required = OPC_FALSE;
					
					//Ctr and no farther border node
					else if (last_frame_sent.type == CTR_END_PK_TYPE)
						is_reply_required = OPC_FALSE;
					
					else if (last_frame_sent.type == CTR_ACK_PK_TYPE)
						is_reply_required = OPC_FALSE;
				
					//default behavior	
					else	
						is_reply_required = OPC_TRUE;
				
					
					
					
					
					//-----------------------------------------------------
					//					Privileged mode
					//-----------------------------------------------------	
					//I sent a CTR-END -> I am no longer priviledged
					if (last_frame_sent.type == CTR_END_PK_TYPE){
						is_node_privileged = OPC_FALSE;
						debug_print(LOW , DEBUG_CONTROL , "the node becomes unpriviledged\n");
					}
				
					
					
					
					//-----------------------------------------------------
					//						Timeout
					//-----------------------------------------------------	
					//Timeout of a reply is required
					if (is_reply_required){
				
						//DATA sent -> a ack only is required
						if ((is_node_privileged) && (nb_channels > 1) && (last_frame_sent.type == DATA_UNICAST_PK_TYPE))
							add_frame_timeout(compute_ack_pk_size());
						else 
							add_frame_timeout(MTU_MAX);
					}
						
					//-----------------------------------------------------
					//						Acks
					//-----------------------------------------------------	
					//Deletes the corresponding data frame (no ack required)
					else if ((last_frame_sent.type == DATA_MULTICAST_PK_TYPE) || (last_frame_sent.type == HELLO_PK_TYPE))
						del_frame_buffer_with_id(last_frame_sent.frame_id , OPC_FALSE);
					
				
				}
				debug_print(LOW , DEBUG_STATE , "EXIT - EMISSION2 - %d\n", my_address);
				}


			/** state (EMISSION) transition processing **/
			FSM_INIT_COND (!is_tx_busy)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("EMISSION")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 6, state6_enter_exec, ;, "!is_tx_busy", "", "EMISSION", "END_TRANSMIT")
				FSM_CASE_TRANSIT (1, 3, state3_enter_exec, ;, "default", "", "EMISSION", "EMISSION")
				}
				/*---------------------------------------------------------*/



			/** state (DEFER) enter executives **/
			FSM_STATE_ENTER_UNFORCED (4, state4_enter_exec, "DEFER", "cmac_process () [DEFER enter execs]")
				{
				//Next data frame to send
				frame_struct	*frame_ptr;
				frame_struct	data_frame;
				frame_struct	next_data_frame;
				//transmission time
				double			time_for_first_data;
				double			time_for_second_data;
				//Control
				char			msg[200];
				
				
				debug_print(LOW , DEBUG_STATE , "ENTER - DEFER1 - %d\n", my_address);
				
				
				//------------------------------------------
				//			   GET DATA FRAME
				//------------------------------------------
				
				
				debug_print(MAX , DEBUG_CONTROL , "STATE_DEFER: data_buffer_empty %d, frame_to_send %d (type %s), IS_PK_TO_SEND %d, border %d, priviledged %d\n", is_frame_buffer_empty() , next_frame_to_send.type != NO_PK_TYPE , pk_type_to_str(next_frame_to_send.type , msg) ,  IS_PK_TO_SEND , is_border_node , is_node_privileged);
				if (is_node_privileged)
					debug_print(MAX , DEBUG_CONTROL , "start_priviledged %f, time_min %f, CAN_SLOT_BE_ENDED %d, PRIVILEDGE_END %d (medium %d, high %d)\n", time_start_privileged , slot_privileged_duration * PRIV_MIN_RATIO , time_start_privileged + slot_privileged_duration * PRIV_MIN_RATIO <= op_sim_time() , PRIVILEGED_END , PRIVILEGED_MEDIUM_LIMIT , PRIVILEGED_HIGH_LIMIT);
						
				
				//We have no frame which is scheduled to be transmitted
				if (next_frame_to_send.type == NO_PK_TYPE){
				
				
					//----------------------------------------------
					//			BORDER & PRIVILEGED 
					//----------------------------------------------
					if (is_node_privileged){
					
						//Buffer empty -> I will lose my priviledged status
						if ((PRIVILEGED_MEDIUM_LIMIT) || (PRIVILEGED_HIGH_LIMIT))
							generate_ctr();
				
						else if ((is_frame_buffer_empty()) && (!PRIVILEGED_MEDIUM_LIMIT))
							op_sim_end("We should not be in this state if we have nothing to send", "and we cannot end the priviledge mode" , "" , "");
							
						//I send the first data_frame of the queue
						else{
							//first frame
							frame_ptr			= get_frame_buffer(0);
							data_frame 			= *frame_ptr;
							time_for_first_data = compute_rts_cts_data_ack_time(data_frame.pk_size) ;
							
							//An eventual second frame
							frame_ptr	= get_frame_buffer(1);
							if (frame_ptr != NULL){
								next_data_frame 		= *frame_ptr;
								time_for_second_data 	= compute_rts_cts_data_ack_time(next_data_frame.pk_size) ;
							}
							else
								time_for_second_data = 0;
							
							//We have not the time to send this data packet !
							if (time_for_first_data + op_sim_time() >= slot_privileged_duration + time_start_privileged)
								generate_ctr();
							
							else{
								
								//Medium reservation if a second packet has to be transmitted
								if ((time_for_second_data == 0) || (time_for_first_data + time_for_second_data + op_sim_time() >= slot_privileged_duration + time_start_privileged))
									data_frame.duration = 0;
								else
									data_frame.duration = next_data_frame.pk_size;
								
								//Prepare the next transmission
								change_next_frame(data_frame);
							}
							
							
						}
					}
					
					//----------------------------------------------
					//			NORMAL NODE
					//----------------------------------------------
					else if (!is_frame_buffer_empty()){
				
						//get the first data packet to send
						frame_ptr = get_frame_buffer(0);		
						if (frame_ptr != NULL)
							data_frame = *frame_ptr;
					
						//broadcast frames do not need any RTS
						if ((data_frame.destination == BROADCAST) || (data_frame.pk_size  > RTS_PK_SIZE))
							change_next_frame(data_frame);
						
						//Generates a new RTS, it will be automatically registered as the new frame to send
						else{
				
							generate_rts(data_frame.destination , data_frame.pk_size , data_frame.frame_id , data_frame.power_ratio);
				
							if (DEBUG >= MAX){
								print_unicast_frame_buffer(DEBUG_SEND);
								print_multicast_frame_buffer(DEBUG_SEND);
							}
						}
					}
					else
						printf("It is strange, I entered in DEFER wihtout any packet to send\n");
				}
				
				
				
				
				
				
				
				//debug
				debug_print(MEDIUM , DEBUG_SEND , "has a packet to send to %d (type %s, id %d, nb_retry %d)\n", next_frame_to_send.destination , pk_type_to_str(next_frame_to_send.type , msg) , next_frame_to_send.frame_id , next_frame_to_send.nb_retry);
				
				
				
				
				//------------------------------------------
				//			   INTER FRAME TIME
				//------------------------------------------
				
				
				//The medium becomes free -> we must wait the inter_frame time
				if (!IS_MEDIUM_BUSY){
					if ((is_node_privileged) && ((next_frame_to_send.type == CTR_PK_TYPE) || (next_frame_to_send.type == CTR_END_PK_TYPE)) )
						next_frame_to_send.ifs = PIFS;
					else if ((is_node_privileged) && (nb_channels > 1))
						next_frame_to_send.ifs = 0;
					else if (is_node_privileged)
						next_frame_to_send.ifs = SIFS;
					else if ((next_frame_to_send.type == RTS_PK_TYPE) || (next_frame_to_send.type == DATA_MULTICAST_PK_TYPE) || (next_frame_to_send.type == HELLO_PK_TYPE))
						next_frame_to_send.ifs = DIFS;
					else
						next_frame_to_send.ifs = SIFS;
				
					//We will verify that the medium remains idle
					op_intrpt_schedule_self(op_sim_time() + next_frame_to_send.ifs , DEFER_CODE);
				}
				
				
				
				
				
				debug_print(LOW , DEBUG_STATE , "EXIT - DEFER1 - %d - timeout %f\n", my_address , next_frame_to_send.ifs *1E6);
				debug_print(LOW, DEBUG_STATE , "%d : %d %d %d %d %d %d %d\n", IS_MEDIUM_BUSY , is_rx_busy , is_tx_busy , get_nav_main_freq() >= op_sim_time() , is_busy_tone_rx , !busy_tone_rx_ignored , BUSY_TONE_ACTIVATED , !is_border_node);
				print_nav_list(DEBUG_STATE);
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (9,cmac_process)


			/** state (DEFER) exit executives **/
			FSM_STATE_EXIT_UNFORCED (4, "DEFER", "cmac_process () [DEFER exit execs]")
				{
				//The inter-frame time
				double		time_to_wait;
				//To verify if we must terminate the simulation
				double		max;
				int			node_id;
				//control
				int			i;
				
				
				
				
				
				debug_print(LOW , DEBUG_STATE , "ENTER - DEFER2 - %d\n", my_address);
				
				
				//Handles the possible interruptions (STREAM || STAT)
				interrupt_process();
				
				
				
				
				
				
				//------------------------------------------
				//			   BACKOFF
				//------------------------------------------
				
				// STAT_INTRPT + !MEDIUM_BUSY -> I must wait the IFS (and not take directly a transition)
				// If such an interruptions occurs, we schedule the IFS verification, but don't change the backoff
				// Thus, the default behavior is to have an invalid backoff in order to avoid any state transition
				my_backoff = -1;
				
				
				
				//I try to transmit or decrement the backoff only if :
				// -> No transmission (this is never the case since the transmission state is not here)
				// -> No reception
				// -> The medium was not previously reserved (verify the value of my NAV)
				//
				if ((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == DEFER_CODE)){
					
					if (!IS_MEDIUM_BUSY){
						if ((!is_node_privileged) &&     (     (next_frame_to_send.type == RTS_PK_TYPE) || (next_frame_to_send.destination == BROADCAST) || ((next_frame_to_send.type == DATA_UNICAST_PK_TYPE) && (next_frame_to_send.pk_size < RTS_PK_SIZE))   )      ){
				
							//Takes a new backoff
							my_backoff = op_dist_outcome (backoff_dist);
						
							//Schedules the backoff interrupt (deletes an eventual old interruption)
							if (op_ev_valid(backoff_intrpt))
								op_ev_cancel(backoff_intrpt);			
							backoff_intrpt = op_intrpt_schedule_self(op_sim_time() + SLOT_BACKOFF * my_backoff , BACKOFF_CODE);
					
							//debug
							debug_print(LOW , DEBUG_BACKOFF , "new backoff %d\n", my_backoff);
						}
						else{
							my_backoff = 0;
							debug_print(MEDIUM , DEBUG_BACKOFF , "no backoff required (priviledged %d, next=rts %d, broadcast %d)\n", my_backoff , is_node_privileged , next_frame_to_send.type == RTS_PK_TYPE , next_frame_to_send.destination == BROADCAST);
						}
						
						
					}
					else
						debug_print(MEDIUM , DEBUG_SEND , "the medium is busy, we must wait\n");
				
				
					//The medium is busy and we have the priority -> that is not regular !
					if (PRIORITY_AND_MEDIUM_BUSY){
						debug_print(LOW , DEBUG_SEND , "a collision occurred (we have the priority and the medium is busy, we discard the current flow\n");
						
						//Discard the current packet
						set_next_frame_null();
						
					}
				}
				
				
				
				//------------------------------------------
				//		  CANCEL OBSOLETE TRANSMISSIONS
				//------------------------------------------
				
				if 	((next_frame_to_send.type == RTS_PK_TYPE) && (is_unicast_frame_buffer_empty())){
					debug_print(LOW , DEBUG_SEND , "Tranmission canceled: RTS and the frame buffer is empty\n");
					set_next_frame_null();
				}
				
				
				if 	(((next_frame_to_send.type == DATA_UNICAST_PK_TYPE) || (next_frame_to_send.type == DATA_MULTICAST_PK_TYPE)) && (!is_in_frame_buffer(next_frame_to_send.frame_id))){
					debug_print(LOW , DEBUG_SEND , "Tranmission canceled: the frame is no longer present in the frame buffer\n");
					set_next_frame_null();
				}
				
				if 	(((next_frame_to_send.type == DATA_MULTICAST_PK_TYPE) || (next_frame_to_send.type == DATA_MULTICAST_PK_TYPE)) && IS_BROADCAST_FORBIDDEN){
					debug_print(LOW , DEBUG_SEND , "Tranmission canceled: the broadcast is currently not authorized\n");
					set_next_frame_null();
				}
				
					
				
				
				//------------------------------------------
				//	 TERMINATE SIMULATION (if required)
				//------------------------------------------
				
				//Get the nex start time for data frames 
				// ->but avoids too frequent verifications (it will slow the simulation)
				
				if (last_next_start_time_verif < op_sim_time() - 30.0){
					next_start_time = 0;
					for(i=0 ; i < op_topo_object_count(OPC_OBJTYPE_NDMOB) ; i++){
				
						node_id = op_topo_object(OPC_OBJTYPE_NDMOB , i);
				
						if (op_ima_obj_attr_exists(node_id , "source.Start Time Packet Generation"))
							op_ima_obj_attr_get(node_id , "source.Start Time Packet Generation" , &max);
						if (max > next_start_time)
							next_start_time = max;
					}	
					
					last_next_start_time_verif = op_sim_time();
				}
				
				
				
				
				
				debug_print(LOW , DEBUG_STATE , "EXIT - DEFER2 - %d (%d, %d)\n", my_address, IS_MEDIUM_BUSY , my_backoff);
				
				
				debug_print(MAX , DEBUG_SEND , "busy -> rx_busy %d tx_busy %d nav %f -> %d medium busy %d\n", is_rx_busy , is_tx_busy , get_nav_main_freq() , get_nav_main_freq() > op_sim_time() , IS_MEDIUM_BUSY   );
				}


			/** state (DEFER) transition processing **/
			FSM_INIT_COND (GO_TO_SEND && !END_SIMULATION)
			FSM_TEST_COND (GO_TO_BACKOFF && !END_SIMULATION)
			FSM_TEST_COND (TRANSMISSION_CANCELED && !END_SIMULATION)
			FSM_TEST_COND (END_SIMULATION)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("DEFER")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 3, state3_enter_exec, ;, "GO_TO_SEND && !END_SIMULATION", "", "DEFER", "EMISSION")
				FSM_CASE_TRANSIT (1, 5, state5_enter_exec, ;, "GO_TO_BACKOFF && !END_SIMULATION", "", "DEFER", "BACKOFF")
				FSM_CASE_TRANSIT (2, 1, state1_enter_exec, ;, "TRANSMISSION_CANCELED && !END_SIMULATION", "", "DEFER", "IDLE")
				FSM_CASE_TRANSIT (3, 8, state8_enter_exec, ;, "END_SIMULATION", "", "DEFER", "END")
				FSM_CASE_TRANSIT (4, 4, state4_enter_exec, ;, "default", "", "DEFER", "DEFER")
				}
				/*---------------------------------------------------------*/



			/** state (BACKOFF) enter executives **/
			FSM_STATE_ENTER_UNFORCED (5, state5_enter_exec, "BACKOFF", "cmac_process () [BACKOFF enter execs]")
				{
				
				debug_print(LOW , DEBUG_STATE , "Enter in backoff state\n");
				
				
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (11,cmac_process)


			/** state (BACKOFF) exit executives **/
			FSM_STATE_EXIT_UNFORCED (5, "BACKOFF", "cmac_process () [BACKOFF exit execs]")
				{
				//Inter frame time
				double		time_to_wait;
				
				debug_print(LOW , DEBUG_STATE , "ENTER - BACKOFF2 - %d\n", my_address);
				
				
				
				
				//Handles the possible interruptions (STREAM || STAT)
				interrupt_process();
				
				
				//The backoff is finished
				if ((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == BACKOFF_CODE)){
					debug_print(MEDIUM, DEBUG_BACKOFF , "the backoff is finished, we can transmit our frame\n");
					my_backoff = 0;
				}
				
				
				
				
				//------------------------------------------
				//			   STOP  BACKOFF
				//------------------------------------------
				
				//The medium is busy OR medium reserved -> stop the backoff
				if ((op_ev_valid(backoff_intrpt)) && (IS_MEDIUM_BUSY)){
					
					//Stores the remaining backoff
					my_backoff = ceil((op_ev_time(backoff_intrpt) - op_sim_time()) / SLOT_BACKOFF);
				
					//disable the previous backoff interrupt (the backoff must not be decremented during medium activity)
					op_ev_cancel(backoff_intrpt);
				
					//debug
					debug_print(MEDIUM , DEBUG_BACKOFF , "stoping backoff %d (busy %d, nav %3f)\n", my_backoff , is_rx_busy , (get_nav_main_freq() - op_sim_time()) * 1E6);
				}
				
				
				
				
				
				//------------------------------------------
				//			  RESTART  BACKOFF
				//------------------------------------------
				
				//the medium becomes idle OR reservation expired -> resume the backoff
				if ((!op_ev_valid(backoff_intrpt)) && (!IS_MEDIUM_BUSY) && ((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == DEFER_CODE))){
				
					//Schedules the backoff interrupt
					backoff_intrpt = op_intrpt_schedule_self(op_sim_time() + SLOT_BACKOFF * my_backoff , BACKOFF_CODE);
				
					//debug
					debug_print(MEDIUM , DEBUG_BACKOFF , "resuming backoff %d (busy %d nav %3f)\n", my_backoff , is_rx_busy , (get_nav_main_freq() - op_sim_time()) * 1E6);
				}
				
				
				
				
				
				
				
				//------------------------------------------
				//			   INTER FRAME TIME
				//------------------------------------------
				
				
				//The medium becomes free -> we must wait the inter_frame time
				if ((!op_ev_valid(backoff_intrpt)) && (!IS_MEDIUM_BUSY) && (!op_ev_valid(defer_intrpt))){
				
					//We will verify that the medium remains idle (inter frame space was already computed in the state DEFER)
					defer_intrpt = op_intrpt_schedule_self(op_sim_time() + next_frame_to_send.ifs , DEFER_CODE);
				
					//debug
					debug_print(MEDIUM , DEBUG_BACKOFF , "defer %f for the inter frame time\n", next_frame_to_send.ifs * 1E6);
				}
				
				
				
				
				
				
				//------------------------------------------
				//		  CANCEL OBSOLETE TRANSMISSIONS
				//------------------------------------------
				// This case can occur if we received an ack athough we already entered in backoff mode
				
				if 	((next_frame_to_send.type == RTS_PK_TYPE) && (is_unicast_frame_buffer_empty())){
					debug_print(LOW , DEBUG_SEND , "Tranmission canceled: RTS and the frame buffer is empty\n");
					set_next_frame_null();
				}
				
				
				if 	(((next_frame_to_send.type == DATA_UNICAST_PK_TYPE) || (next_frame_to_send.type == DATA_MULTICAST_PK_TYPE)) && (!is_in_frame_buffer(next_frame_to_send.frame_id))){
					debug_print(LOW , DEBUG_SEND , "Tranmission canceled: the frame is no longer present in the frame buffer\n");
					set_next_frame_null();
				}
				
				if 	(((next_frame_to_send.type == DATA_MULTICAST_PK_TYPE) || (next_frame_to_send.type == DATA_MULTICAST_PK_TYPE)) && IS_BROADCAST_FORBIDDEN){
					debug_print(LOW , DEBUG_SEND , "Tranmission canceled: the broadcast is currently not authorized\n");
					set_next_frame_null();
				}
				
				
				
				
				//------------------------------------------
				//			   DEBUG
				//------------------------------------------
				
				//Debug
				
				debug_print(MAX , DEBUG_BACKOFF , "IS_BACK_TO_DEFER %d backoff %d (conditions %d %d %d %d)\n", IS_BACK_TO_DEFER , my_backoff , next_frame_to_send.type == CTR_PK_TYPE , IS_FRAME_RECEIVED , IS_REPLY_TO_SEND , is_rx_busy);
				
				
				debug_print(MAX , DEBUG_BACKOFF , "TYPE %d (self %d stream %d stat %d), code %d\n", op_intrpt_type() , OPC_INTRPT_SELF , OPC_INTRPT_STRM , OPC_INTRPT_STAT , op_intrpt_code());
				if (op_intrpt_type() == OPC_INTRPT_STRM){
					debug_print(MAX , DEBUG_BACKOFF , "stream %d\n", op_intrpt_strm());
					debug_print(MAX , DEBUG_BACKOFF , "%d %d %d %d %d\n", op_intrpt_type() , OPC_INTRPT_STRM , (op_intrpt_type() == OPC_INTRPT_STRM) , (op_intrpt_strm() == STREAM_FROM_RADIO) , IS_FRAME_RECEIVED);
				}
				
				
				
				
				
				
				
				
				//---------------------------------------------
				//		EXIT BACKOFF -> CANCEL the INTRPT
				//---------------------------------------------
				
				if ((IS_BACK_TO_DEFER) || (TRANSMISSION_CANCELED))
					if (op_ev_valid(backoff_intrpt))
						op_ev_cancel(backoff_intrpt);
				
				
				
				
				
				
				
				
				
				
				
				
				debug_print(LOW , DEBUG_STATE , "EXIT - BACKOFF2 - %d\n", my_address);
				}


			/** state (BACKOFF) transition processing **/
			FSM_INIT_COND (!IS_BACK_TO_DEFER && !TRANSMISSION_CANCELED  && (my_backoff == 0))
			FSM_TEST_COND (!TRANSMISSION_CANCELED && IS_BACK_TO_DEFER)
			FSM_TEST_COND (TRANSMISSION_CANCELED)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("BACKOFF")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 3, state3_enter_exec, ;, "!IS_BACK_TO_DEFER && !TRANSMISSION_CANCELED  && (my_backoff == 0)", "", "BACKOFF", "EMISSION")
				FSM_CASE_TRANSIT (1, 4, state4_enter_exec, ;, "!TRANSMISSION_CANCELED && IS_BACK_TO_DEFER", "", "BACKOFF", "DEFER")
				FSM_CASE_TRANSIT (2, 1, state1_enter_exec, ;, "TRANSMISSION_CANCELED", "", "BACKOFF", "IDLE")
				FSM_CASE_TRANSIT (3, 5, state5_enter_exec, ;, "default", "", "BACKOFF", "BACKOFF")
				}
				/*---------------------------------------------------------*/



			/** state (END_TRANSMIT) enter executives **/
			FSM_STATE_ENTER_FORCED (6, state6_enter_exec, "END_TRANSMIT", "cmac_process () [END_TRANSMIT enter execs]")
				{
				//Control
				char	msg[500];
				
				
				
				//Just a debug message when we start to wait the reply for one of our frame
				if (is_reply_required){
					//debug
					debug_print(MAX , DEBUG_SEND , "%s sent, it must wait a reply\n", pk_type_to_str(last_frame_sent.type , msg));
				
				
					//We did not received yet a reply for our transmission (we just leave it !)
					is_reply_received = OPC_FALSE;
				}
				}


			/** state (END_TRANSMIT) exit executives **/
			FSM_STATE_EXIT_FORCED (6, "END_TRANSMIT", "cmac_process () [END_TRANSMIT exit execs]")
				{
				}


			/** state (END_TRANSMIT) transition processing **/
			FSM_INIT_COND (is_reply_required)
			FSM_TEST_COND (!is_reply_required&& !IS_PK_TO_SEND && ! PRIVILEGED_END)
			FSM_TEST_COND (!is_reply_required && (IS_PK_TO_SEND || PRIVILEGED_END))
			FSM_TEST_LOGIC ("END_TRANSMIT")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 7, state7_enter_exec, ;, "is_reply_required", "", "END_TRANSMIT", "WAIT_NEXT")
				FSM_CASE_TRANSIT (1, 1, state1_enter_exec, ;, "!is_reply_required&& !IS_PK_TO_SEND && ! PRIVILEGED_END", "", "END_TRANSMIT", "IDLE")
				FSM_CASE_TRANSIT (2, 4, state4_enter_exec, ;, "!is_reply_required && (IS_PK_TO_SEND || PRIVILEGED_END)", "", "END_TRANSMIT", "DEFER")
				}
				/*---------------------------------------------------------*/



			/** state (WAIT_NEXT) enter executives **/
			FSM_STATE_ENTER_UNFORCED (7, state7_enter_exec, "WAIT_NEXT", "cmac_process () [WAIT_NEXT enter execs]")
				{
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (15,cmac_process)


			/** state (WAIT_NEXT) exit executives **/
			FSM_STATE_EXIT_UNFORCED (7, "WAIT_NEXT", "cmac_process () [WAIT_NEXT exit execs]")
				{
				//Control
				char		msg[500];
				char		msg2[500];
				//packet and info
				Packet		*frame;
				int			source , destination , type;
				
				
				//Default (will change of the received packet is not that we expect
				is_reply_bad = OPC_FALSE;
				
				
				debug_print(LOW , DEBUG_STATE , "ENTER - WAIT_NEXT2 - %d\n", my_address);
				
				
				//Handles the possible interruptions (STREAM || STAT)
				//NB: if we had a timeout, and we received a packet, we can answer to the received packet and discard temporarily the retransmission (rather delay it)
				interrupt_process();
				
				
				
				//The frame timeouted -> we must retransmit it
				if ((is_reply_bad) || (IS_FRAME_TIMEOUT)){
					
					//Failed -> we increments the nb_retry for the corresponding data_frame
					//NB: it is a unicast frame since multicast frame are not acknowledged
					if ((last_frame_sent.type == DATA_UNICAST_PK_TYPE) || (last_frame_sent.type == RTS_PK_TYPE))
						increment_nb_rety_unicast_frame_buffer(last_frame_sent.frame_id);
						
					debug_print(LOW , DEBUG_TIMEOUT , "the flow to %d is broken for one of my frame (type %s, retry %d , id %d, next type %d) (timeout %d bad_reply %d)\n", last_frame_sent.destination , pk_type_to_str(last_frame_sent.type, msg) , last_frame_sent.nb_retry , last_frame_sent.frame_id , next_frame_to_send.type , IS_FRAME_TIMEOUT , is_reply_bad);
					
					
					//CTR -> we must retransmit it
					if ((last_frame_sent.type == CTR_PK_TYPE) && (last_frame_sent.nb_retry < MAX_NB_RETRY)){
					
						//increase the nb of retries
						last_frame_sent.nb_retry ++;
						
						//And retransmission !
						change_next_frame(last_frame_sent);
						
						//Failed -> go to the main channel
						if (nb_channels > 1)
							change_tx_rx_freq(my_main_frequency , my_main_bandwidth * RATIO_PRIV_BANDWIDTH , STREAM_TO_RADIO);
					}
					
				
					//Error time when a packet failed
					if (!is_node_privileged)
						update_nav_time(EIFS , my_address , my_main_frequency , -1);
					
					
					//The frame timeouted: no more busy tone in transmission
					is_busy_tone_tx 		= OPC_FALSE;
				}
				
				
				
				//The medium is no longer reserved for this flow
				if ((is_reply_received) || (IS_FRAME_TIMEOUT) || is_reply_bad) {
					is_reply_required 		= OPC_FALSE;
					
					//The transmission whatever it is succefull or not is ended: we must take into account the current busy tones
					busy_tone_rx_ignored 	= OPC_FALSE;
				}
				
				
				
				
				
				
				
				
				debug_print(LOW , DEBUG_STATE , "EXIT - WAIT_NEXT2 - %d\n", my_address);
				
				}


			/** state (WAIT_NEXT) transition processing **/
			FSM_INIT_COND (is_reply_received || IS_FRAME_TIMEOUT || is_reply_bad)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("WAIT_NEXT")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 6, state6_enter_exec, ;, "is_reply_received || IS_FRAME_TIMEOUT || is_reply_bad", "", "WAIT_NEXT", "END_TRANSMIT")
				FSM_CASE_TRANSIT (1, 7, state7_enter_exec, ;, "default", "", "WAIT_NEXT", "WAIT_NEXT")
				}
				/*---------------------------------------------------------*/



			/** state (END) enter executives **/
			FSM_STATE_ENTER_UNFORCED (8, state8_enter_exec, "END", "cmac_process () [END enter execs]")
				{
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (17,cmac_process)


			/** state (END) exit executives **/
			FSM_STATE_EXIT_UNFORCED (8, "END", "cmac_process () [END exit execs]")
				{
				}


			/** state (END) transition processing **/
			FSM_TRANSIT_FORCE (8, state8_enter_exec, ;, "default", "", "END", "END")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (0,cmac_process)
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
cmac_process_init (void ** gen_state_pptr)
	{
	int _block_origin = 0;
	static VosT_Address	obtype = OPC_NIL;

	FIN (cmac_process_init (gen_state_pptr))

	if (obtype == OPC_NIL)
		{
		/* Initialize memory management */
		if (Vos_Catmem_Register ("proc state vars (cmac_process)",
			sizeof (cmac_process_state), Vos_Vnop, &obtype) == VOSC_FAILURE)
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
		((cmac_process_state *)(*gen_state_pptr))->current_block = 0;

		FRET (OPC_COMPCODE_SUCCESS)
		}
	}



void
cmac_process_diag (void)
	{
	/* No Diagnostic Block */
	}




void
cmac_process_terminate (void)
	{
	int _block_origin = __LINE__;

	FIN (cmac_process_terminate (void))

	if (1)
		{

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
/* local variable prs_ptr in cmac_process_svar function. */
#undef my_address
#undef is_reply_required
#undef is_reply_bad
#undef is_reply_received
#undef next_frame_to_send
#undef last_frame_sent
#undef unicast_frame_buffer
#undef multicast_frame_buffer
#undef operational_speed
#undef is_border_node
#undef is_node_privileged
#undef is_sink
#undef my_dist_sink
#undef my_dist_border
#undef my_neighborhood_table
#undef nb_channels
#undef is_tx_busy
#undef my_current_tx_power
#undef is_rx_busy
#undef rx_power_threshold
#undef my_sync_rx_power
#undef my_nav_list
#undef cw
#undef my_backoff
#undef backoff_intrpt
#undef backoff_dist
#undef time_start_privileged
#undef frame_timeout_intrpt
#undef timeout_intrpt
#undef main_freq_return_intrpt
#undef DEBUG
#undef defer_intrpt
#undef ctr_last_branch
#undef last_rx_power
#undef my_main_frequency
#undef my_main_bandwidth
#undef is_busy_tone_rx
#undef busy_tone_speed
#undef is_busy_tone_tx
#undef busy_tone_rx_ignored
#undef BUSY_TONE_ACTIVATED
#undef my_debug_file
#undef my_stat_id
#undef is_hello_to_send
#undef my_frame_id_seen
#undef next_start_time
#undef last_next_start_time_verif
#undef is_ctr_activated
#undef strict_privileged_mode
#undef is_sync_direct_antenna
#undef sync_last_branch
#undef my_branch
#undef my_privileged_frequency
#undef slot_privileged_duration
#undef slot_privileged_offset
#undef bn_list
#undef BETA
#undef RTS_PK_SIZE
#undef MAC_ROUTING
#undef POWER_TX
#undef border_election
#undef RADIO_RANGE



void
cmac_process_svar (void * gen_ptr, const char * var_name, char ** var_p_ptr)
	{
	cmac_process_state		*prs_ptr;

	FIN (cmac_process_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (char *)OPC_NIL;
		FOUT;
		}
	prs_ptr = (cmac_process_state *)gen_ptr;

	if (strcmp ("my_address" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_address);
		FOUT;
		}
	if (strcmp ("is_reply_required" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->is_reply_required);
		FOUT;
		}
	if (strcmp ("is_reply_bad" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->is_reply_bad);
		FOUT;
		}
	if (strcmp ("is_reply_received" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->is_reply_received);
		FOUT;
		}
	if (strcmp ("next_frame_to_send" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->next_frame_to_send);
		FOUT;
		}
	if (strcmp ("last_frame_sent" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->last_frame_sent);
		FOUT;
		}
	if (strcmp ("unicast_frame_buffer" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->unicast_frame_buffer);
		FOUT;
		}
	if (strcmp ("multicast_frame_buffer" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->multicast_frame_buffer);
		FOUT;
		}
	if (strcmp ("operational_speed" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->operational_speed);
		FOUT;
		}
	if (strcmp ("is_border_node" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->is_border_node);
		FOUT;
		}
	if (strcmp ("is_node_privileged" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->is_node_privileged);
		FOUT;
		}
	if (strcmp ("is_sink" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->is_sink);
		FOUT;
		}
	if (strcmp ("my_dist_sink" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_dist_sink);
		FOUT;
		}
	if (strcmp ("my_dist_border" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_dist_border);
		FOUT;
		}
	if (strcmp ("my_neighborhood_table" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_neighborhood_table);
		FOUT;
		}
	if (strcmp ("nb_channels" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->nb_channels);
		FOUT;
		}
	if (strcmp ("is_tx_busy" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->is_tx_busy);
		FOUT;
		}
	if (strcmp ("my_current_tx_power" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_current_tx_power);
		FOUT;
		}
	if (strcmp ("is_rx_busy" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->is_rx_busy);
		FOUT;
		}
	if (strcmp ("rx_power_threshold" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->rx_power_threshold);
		FOUT;
		}
	if (strcmp ("my_sync_rx_power" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_sync_rx_power);
		FOUT;
		}
	if (strcmp ("my_nav_list" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_nav_list);
		FOUT;
		}
	if (strcmp ("cw" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->cw);
		FOUT;
		}
	if (strcmp ("my_backoff" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_backoff);
		FOUT;
		}
	if (strcmp ("backoff_intrpt" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->backoff_intrpt);
		FOUT;
		}
	if (strcmp ("backoff_dist" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->backoff_dist);
		FOUT;
		}
	if (strcmp ("time_start_privileged" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->time_start_privileged);
		FOUT;
		}
	if (strcmp ("frame_timeout_intrpt" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->frame_timeout_intrpt);
		FOUT;
		}
	if (strcmp ("timeout_intrpt" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->timeout_intrpt);
		FOUT;
		}
	if (strcmp ("main_freq_return_intrpt" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->main_freq_return_intrpt);
		FOUT;
		}
	if (strcmp ("DEBUG" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->DEBUG);
		FOUT;
		}
	if (strcmp ("defer_intrpt" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->defer_intrpt);
		FOUT;
		}
	if (strcmp ("ctr_last_branch" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->ctr_last_branch);
		FOUT;
		}
	if (strcmp ("last_rx_power" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->last_rx_power);
		FOUT;
		}
	if (strcmp ("my_main_frequency" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_main_frequency);
		FOUT;
		}
	if (strcmp ("my_main_bandwidth" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_main_bandwidth);
		FOUT;
		}
	if (strcmp ("is_busy_tone_rx" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->is_busy_tone_rx);
		FOUT;
		}
	if (strcmp ("busy_tone_speed" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->busy_tone_speed);
		FOUT;
		}
	if (strcmp ("is_busy_tone_tx" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->is_busy_tone_tx);
		FOUT;
		}
	if (strcmp ("busy_tone_rx_ignored" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->busy_tone_rx_ignored);
		FOUT;
		}
	if (strcmp ("BUSY_TONE_ACTIVATED" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->BUSY_TONE_ACTIVATED);
		FOUT;
		}
	if (strcmp ("my_debug_file" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_debug_file);
		FOUT;
		}
	if (strcmp ("my_stat_id" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_stat_id);
		FOUT;
		}
	if (strcmp ("is_hello_to_send" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->is_hello_to_send);
		FOUT;
		}
	if (strcmp ("my_frame_id_seen" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_frame_id_seen);
		FOUT;
		}
	if (strcmp ("next_start_time" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->next_start_time);
		FOUT;
		}
	if (strcmp ("last_next_start_time_verif" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->last_next_start_time_verif);
		FOUT;
		}
	if (strcmp ("is_ctr_activated" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->is_ctr_activated);
		FOUT;
		}
	if (strcmp ("strict_privileged_mode" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->strict_privileged_mode);
		FOUT;
		}
	if (strcmp ("is_sync_direct_antenna" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->is_sync_direct_antenna);
		FOUT;
		}
	if (strcmp ("sync_last_branch" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->sync_last_branch);
		FOUT;
		}
	if (strcmp ("my_branch" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_branch);
		FOUT;
		}
	if (strcmp ("my_privileged_frequency" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_privileged_frequency);
		FOUT;
		}
	if (strcmp ("slot_privileged_duration" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->slot_privileged_duration);
		FOUT;
		}
	if (strcmp ("slot_privileged_offset" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->slot_privileged_offset);
		FOUT;
		}
	if (strcmp ("bn_list" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->bn_list);
		FOUT;
		}
	if (strcmp ("BETA" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->BETA);
		FOUT;
		}
	if (strcmp ("RTS_PK_SIZE" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->RTS_PK_SIZE);
		FOUT;
		}
	if (strcmp ("MAC_ROUTING" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->MAC_ROUTING);
		FOUT;
		}
	if (strcmp ("POWER_TX" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->POWER_TX);
		FOUT;
		}
	if (strcmp ("border_election" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->border_election);
		FOUT;
		}
	if (strcmp ("RADIO_RANGE" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->RADIO_RANGE);
		FOUT;
		}
	*var_p_ptr = (char *)OPC_NIL;

	FOUT;
	}

