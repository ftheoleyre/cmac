#ifndef _CMAC_TYPES_H__

#define _CMAC_TYPES_H__





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
#define		MAX_STAB						10

//Two stabilities with STAB_STEP difference are considered equal
#define		STAB_STEP						2




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
//				ROUTING
//-----------------------------------------------

#define		ROUTING_MAC_BORDER				2
#define		ROUTING_MAC_SHORT				1
#define		ROUTING_MAC_NO					0







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
//					FRAME
//-----------------------------------------------

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
//				STATS ABOUT PACKETS
//-----------------------------------------------



typedef struct {
	int		source;
	int		destination;
	int		hops;
	int		pk_id;
	Boolean	received;
	double	time_sent;
	double	time_received;
} pk_info;


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


typedef struct{
	int		addr;
	double	pow;
	short	stab;
	short	branch;
}election_struct;



#endif
