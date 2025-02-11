/*
 *  cmac_interface.h
 *  
 *  Created by Fabrice Theoleyre on 29/09/09.
 *  Copyright 2009 CNRS / LIG. All rights reserved.
 *
 */


#ifndef _CMAC_INTERFACE_H_
#define _CMAC_INTERFACE_H_



//--------------------------------------
//
//			CONSTANTS
//
//--------------------------------------

//Radio range (in meters)
#define		PHYSIC_RADIO_RANGE			21

//Addresses
#define		MIN_ADDRESS					101


#define		TIMEOUT_ID					5.0





//--------------------------------------
//
//			TRAFFIC GENERATION
//
//--------------------------------------

//to start the packet generation
#define		TIME_START_PK_GENERATION	60.0

//The maximum delay allowed to consider the reception of a packet valid
#define		MAX_DELAY					5.0






//--------------------------------------
//
//		FIELD NB in the PAYLOAD
//
//--------------------------------------

//field pk number for the data payload
#define		FIELD_PAYLOAD_SOURCE	0
#define		FIELD_PAYLOAD_ID		1




//--------------------------------------
//
//	TO UPDATE STATS ON DATA FRAMES
//
//--------------------------------------

//updates the route for this frame
void cmac_interface_add_in_route(int frame_id, int addr);


#endif
