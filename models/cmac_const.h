/*
 *  cmac_const.h
 *  
 *  Created by Fabrice Theoleyre on 29/09/09.
 *  Copyright 2009 CNRS / LIG. All rights reserved.
 *
 */


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

//debug
#define		LOG_SUFFIX_NODES				"nodes"

//ktree algo
#define		KTREE_ALGO_SYNC					1
#define		KTREE_ALGO_MAXTREE				2
#define		KTREE_ALGO_STATIC				3



//-----------------------------------------------
//				CONSTANTS
//-----------------------------------------------


//reserved addresses
#define		ADDR_BROADCAST					-1
#define		ADDR_INVALID					-2

//backoff
#define		MAX_BACKOFF						32
#define		MAX_EXPO_BACKOFF				1024

//others
#define		SHORT_INFINITY					255

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
//#define		TIMEOUT_DATA_FRAME				0.5
//I can miss one privileged slot, not two!
#define		TIMEOUT_DATA_FRAME				(2 * NB_BRANCHES * TIME_MAX_PRIVILEGED * CTR_HOP_SPACING)






//-----------------------------------------------
//				HELLOS
//-----------------------------------------------

//time between 2 hellos
#define		HELLO_PK_PERIOD								5.0
#define		HELLO_PK_PERIOD_BEFORE_CONVERGENCE			5.0
#define		HELLO_PK_TIMEOUT							3

//the stability metric is averaged over the last MAX_NB hellos
#define		STAB_NB										8

//The minimum acceptable stability for one radio link
#define		STAB_MIN									0.87

//some specific time values
#define		TIME_BEFORE_HELLO_CONVERGENCE				59.0
#define		FREEZE_HELLO_DISCOVERY_AFTER_CONVERGENCE	OPC_FALSE
#define		TIME_START_DATA								60.0
#define		TIME_GUARD_END_DATA							0.5
#define		BLOCK_KTREE_AFTER_START_DATA				OPC_FALSE



//-----------------------------------------------
//				CTR / PRIVILEGE
//-----------------------------------------------


//the 
#define		PRIVILEGED_MIN_TIME_RATIO		0.3





//-----------------------------------------------
//				SYNC
//-----------------------------------------------

#define		SYNC_DIRECT_ANTENNA				OPC_TRUE



//-----------------------------------------------
//				DATA
//-----------------------------------------------


#define		DATA_BUFFER_SIZE_MAX			40




//-----------------------------------------------
//				MULTI CHANNEL
//-----------------------------------------------

#define		CHANNEL_MAIN_ID					0
#define		CHANNEL_RESERVED				-1


//bandwidth
#define		BANDWIDTH_MHZ_80211A			20
#define		BANDWIDTH_MHZ_80211BG			22



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
#define		DEBUG_KTREE						5
#define		DEBUG_HELLO						6

#define		DEBUG_TIMEOUT					7

#define		DEBUG_NODE						8

#define		DEBUG_PRINTF					9


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
#define		HEADERS_PK_SIZE					(57 + 32*2 + 4 + 16 + 4 + 32)
#define		NB_TIER_SIZE					4
#define		DURATION_SIZE					16
#define		SINK_DIST_SIZE					4
#define		KTREE_DIST_SIZE					4

//this corresponds to an 'infinite' distance
#define		DIST_SINK_MAX					15	

//MTU in bits
#define		MTU_MAX								(8 * 2400.0)


	
//fields names
#define		FIELD_PK_SOURCE						"source"
#define		FIELD_PK_DESTINATION				"destination"
#define		FIELD_PK_ID							"Data Packet ID"	
#define		FIELD_PK_NAV_DURATION				"nav_duration"		//NB: in usec
#define		FIELD_PK_TYPE						"type"
#define		FIELD_PK_ACCEPT						"Accept"
#define		FIELD_PK_POWER_RATIO				"power_ratio"

//hellos
#define		FIELD_PK_HELLO_SINK_DIST			"sink_dist"
#define		FIELD_PK_HELLO_KTREE_DIST			"ktree_dist"
#define		FIELD_PK_HELLO_NEXT					"next_hello"
#define		FIELD_PK_HELLO_PARENT				"parent"
#define		FIELD_PK_HELLO_NB_CMAC_CHILDREN		"nb_cmac_children"
#define		FIELD_PK_HELLO_CMAC_CHILDREN		"cmac_children"
#define		FIELD_PK_HELLO_NEIGH_TABLE			"neigh_table"
#define		FIELD_PK_HELLO_NEIGH_TABLE_SIZE		"neigh_table_size"
//SYNC power algo
#define		FIELD_PK_HELLO_SYNC_POWER			"sync_power"
//Maxtree algo
#define		FIELD_PK_HELLO_SUBTREE_SIZE			"subtree_size"
#define		FIELD_PK_HELLO_SAVINGS				"savings"


//data
#define		FIELD_PK_DATA_PAYLOAD				"payload"

//ctr
#define		FIELD_PK_CTR_CHANNEL				"channel"
#define		FIELD_PK_CTR_TSLOT					"t_slot"
#define		FIELD_PK_CTR_OFFSET					"offset"
#define		FIELD_PK_CTR_BRANCHID				"branch_id"

//sync
#define		FIELD_PK_SYNC_BRANCH				"branch"




#endif
