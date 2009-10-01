/*
 *  cmac_process.h
 *  
 *  Created by Fabrice Theoleyre on 29/09/09.
 *  Copyright 2009 CNRS / LIG. All rights reserved.
 *
 */


#ifndef	_CMAC_H_
#define _CMAC_H_


#include 	<math.h>
#include 	<string.h>
#include 	<stdarg.h>
#include 	<stdio.h>
#include 	<stdlib.h>
#include	<errno.h>
#include	<opnet.h>
#include 	<errno.h>
#include 	<dirent.h>
#include 	<sys/types.h>
#include 	<sys/stat.h>

#include 	<wlan_support.h>

#include	"cmac_const.h"
#include	"tools_fig.h"





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
	int		dist_ktree;
	double	sync_rx_power;
	List	*ktree_children;	
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
//			ELECTION OF KTREE CHILDREN
//-----------------------------------------------


typedef struct{
	int		addr;
	double	pow;
	short	stab;
	short	branch;
}election_struct;



//-----------------------------------------------
//			geo POSITION
//-----------------------------------------------


typedef struct{
	double	x;
	double	y;
}pos_struct;




//-----------------------------------------------
//				PROTOTYPES
//-----------------------------------------------

//parameters
int		get_nb_nodes();
int 	nodeid_to_addr(int nodeid);
int 	addr_to_nodeid(int addr);


//debug
void	debug_print(const int level, const int type , const char* fmt, ...);
char* 	pk_type_to_str(short pk_type , char *msg, int length);
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
void 	update_neighborhood_table(int source , int dist_sink , int dist_ktree, double sync_rx_power, int branch, List *bn_list_tmp , double next_hello);
char* 	print_ktree_cildren(char *msg, int length);


//Stability
int 	compute_stability(int stab[]);
void 	update_stability(int stab[], short value);
void 	init_stability(int stab[], short value);


//To compute duration for NAV
double 	compute_rts_cts_data_ack_time(int data_pk_size);
double 	compute_cts_data_ack_time(int data_pk_size);
double 	compute_data_ack_time(int data_pk_size);


//Busy tone
void 	maintain_busy_tone(double time);


//antennas
void 	change_antenna_direction(int stream , int branch);
void 	change_tx_power(double power , int stream);



#endif
