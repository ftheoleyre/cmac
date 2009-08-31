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
//				PARAMETERS
//-----------------------------------------------

//mac layer names
#define		CMAC							1
#define		IEEE80211						2

//some specific time values
#define		TIME_BEFORE_HELLO_CONVERGENCE	5.0 + HELLO_PK_PERIOD * 1.1
#define		TIME_START_DATA					60.0

//debug
#define		LOG_SUFFIX_NODES				"nodes"

//ktree algo
#define		KTREE_ALGO_SYNC					1
#define		KTREE_ALGO_MAXTREE				2
#define		KTREE_ALGO_NONE					0



//-----------------------------------------------
//				CONSTANTS
//-----------------------------------------------


//reserved addresses
#define		ADDR_BROADCAST					-1
#define		ADDR_INVALID					-2

//backoff
#define		MAX_BACKOFF						32
#define		MAX_EXPO_BACKOFF				1024

//Power
#define		MAX_KTREE_DIST					10

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

//time between 2 hellos
#define		HELLO_PK_PERIOD					1.0

//The maximum number of stabilities to store
#define		MAX_STAB						10

//Two stabilities with STAB_STEP difference are considered equal
#define		STAB_STEP						2




//-----------------------------------------------
//				CTR / PRIVILEGE
//-----------------------------------------------


// PRIVILEGED MODE
#define		PRIV_MIN_RATIO					0.9

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

#define		ROUTING_MAC_KTREE				2
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

#define		DEBUG_BACKOFF					1

#define		DEBUG_DATA						2
#define		DEBUG_RADIO						3

#define		DEBUG_CONTROL					4
#define		DEBUG_CMAC						5
#define		DEBUG_HELLO						6

#define		DEBUG_TIMEOUT					7

#define		DEBUG_NODE						8

//current MAX=11


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
#define		DIST_KTREE_SIZE					4


//MTU in bits
#define		MTU_MAX							2400.0


	
//fields names
#define		FIELD_PK_SOURCE						"SOURCE"
#define		FIELD_PK_DESTINATION				"DESTINATION"
#define		FIELD_PK_ID							"Data Packet ID"	
#define		FIELD_PK_DURATION					"DURATION"	
#define		FIELD_PK_TYPE						"TYPE"
#define		FIELD_PK_ACCEPT						"Accept"
#define		FIELD_PK_POWER_RATIO				"POWER_RATIO"

//hellos
#define		FIELD_PK_HELLO_DIST_SINK			"DIST_SINK"
#define		FIELD_PK_HELLO_DIST_KTREE			"DIST_KTREE"
#define		FIELD_PK_HELLO_SYNC_POWER			"SYNC_POWER"
#define		FIELD_PK_HELLO_BRANCH				"BRANCH"
#define		FIELD_PK_HELLO_NEXT					"NEXT"
#define		FIELD_PK_HELLO_NB_KTREE_CHILDREN	"NB_KTREE_CHILDREN"
#define		FIELD_PK_HELLO_KTREE_CHILDREN		"KTREE_CHILDREN"
//Maximum number of branches in the packet
#define		PK_HELLO_MAX_NB_BRANCHES			4

//data
#define		FIELD_PK_DATA_PAYLOAD				"PAYLOAD"

//ctr
#define		FIELD_PK_CTR_FREQ					"FREQ"
#define		FIELD_PK_CTR_TSLOT					"T_SLOT"
#define		FIELD_PK_CTR_OFFSET					"OFFSET"

//sync
#define		FIELD_PK_SYNC_BRANCH				"BRANCH"




#endif
