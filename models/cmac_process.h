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
//				ATTRIBUTES
//-----------------------------------------------

extern int nb_channels;



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
	Boolean	bidirect;
	double	timeout;
	int		sink_dist;
	int		ktree_dist;
	List	*cmac_children;			//children in the k-tree core
	int		parent;
	short	*savings;
	short	subtree_size;
	double	sync_rx_power;
	Boolean	*stability;
	short	stability_ptr;			//the current pointer for the stability metric (cell in the array)
} neigh_struct;

typedef struct{
	int		address;
	short	branch_id;
} child_struct;

typedef struct{
	int		address;
	short	saving;
} saving_comp_struct;



//-----------------------------------------------
//			TREE OF SHORTEST PATHS TO THE SINK
//-----------------------------------------------


typedef struct{
	int		parent;
	short	sink_dist;
	short	ktree_dist;
	short	subtree_size;
	List	*cmac_children;
	Boolean	is_in_ktree;
} sink_tree_struct;



//-----------------------------------------------
//				FRAME ID LIST
//-----------------------------------------------


typedef struct{
	int		source;
	int		id;
	double	timeout;
}id_timeout_struct;



//-----------------------------------------------
//				STATS ABOUT PACKETS
//-----------------------------------------------



typedef struct {
	int		source;
	int		hops;
	int		pk_id;
	double	time_sent;
	double	time_received;
	List	*route;
} pk_info;


//-----------------------------------------------
//					NAV
//-----------------------------------------------


typedef struct{
	int		address;
	double	timeout;
	short	channel;
}nav_struct;





//-----------------------------------------------
//			COMPARISON FOR NEXT HOP
//-----------------------------------------------


typedef struct{
	int		addr;
	short	sink_dist;
	short	ktree_dist;
} compar_struct;






//-----------------------------------------------
//			ELECTION OF KTREE CHILDREN
//-----------------------------------------------


typedef struct{
	int		addr;
	double	pow;
	short	branch;
}election_struct;





//-----------------------------------------------
//				PROTOTYPES
//-----------------------------------------------

//parameters
int		get_nb_nodes();
int 	nodeid_to_addr(int nodeid);
int 	addr_to_nodeid(int addr);


//tools (grid positions);
int get_x_coord_in_grid(int addr);
int get_y_coord_in_grid(int addr);


//interface
Boolean is_cmac_child_of(neigh_struct *neigh_ptr, int my_address_tmp);


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
void 	stability_update(neigh_struct *neigh_ptr, Boolean received);
void 	stability_init(neigh_struct *neigh_ptr);
double 	stability_get(neigh_struct *neigh_ptr);

void 	print_neighborhood_table(int debug_type);
void  	generate_hello(double next_hello);
void 	update_neighborhood_table(int source , Boolean bidirect, int sink_dist , int ktree_dist, double sync_rx_power, int parent, short subtree_size, short *savings, int branch , List *my_ktree_children_tmp , double next_hello);
char* 	print_ktree_cildren(char *msg, int length);


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
