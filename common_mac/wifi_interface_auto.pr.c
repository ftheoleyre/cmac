/* Process model C form file: wifi_interface_auto.pr.c */
/* Portions of this file copyright 1992-2002 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
static const char wifi_interface_auto_pr_c [] = "MIL_3_Tfile_Hdr_ 81A 30A modeler 7 44BDEDE7 44BDEDE7 1 ares-theo-1 ftheoley 0 0 none none 0 0 none 0 0 0 0 0 0                                                                                                                                                                                                                                                                                                                                                                                                                 ";
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

/***** Include Files. *****/
#include <stdlib.h>

#include <math.h>


/* Address assignment definitions.	*/
#include "oms_auto_addr_support.h"

/* Topology analysis-related definitions. */
#include "oms_tan.h"

/* Process registry-related definitions. */
#include "oms_pr.h"



/***** Transition Macros ******/

#define		RATE_ADAPT_CODE		100
#define		RATE_STOP_CODE		101

#define 	MAC_LAYER_PKT_ARVL		((op_intrpt_type() == OPC_INTRPT_STRM) && (op_intrpt_strm() == STREAM_FROM_MAC))
#define 	APPL_LAYER_PKT_ARVL		((op_intrpt_type() == OPC_INTRPT_STRM) && (op_intrpt_strm() == STREAM_FROM_UP))
#define 	MAC_BROADCAST			-1
#define 	ENDSIM					(op_intrpt_type() == OPC_INTRPT_ENDSIM)
#define 	RATE_ADAPTATION			((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() > RATE_ADAPT_CODE))



/***** Functional declaration ******/
static void			wlan_mac_higher_layer_intf_sv_init ();
static void			wlan_mac_higher_layer_register_as_arp ();





//--------------------------------------
//
//			PROCESSES
//
//--------------------------------------

#define		MAC_PROCESS_NAME		"mac."




//--------------------------------------
//
//			ROUTING TYPES
//
//--------------------------------------



#define		NO_ROUTING					-1
#define		XY_ROUTING					0
#define		OPT_ROUTING					1
#define		SHORT_ROUTING				2
#define		SIDES_ROUTING				3
#define		ALPHA_ROUTING				4
#define		OPT2_ROUTING				5




//--------------------------------------
//
//			POSITION TYPES
//
//--------------------------------------

#define		GRID_POSITION				1
#define		RANDOM_POSITION				2
#define		AS_IS_POSITION				3
#define		RANDOM_GRID_POSITION		4




//--------------------------------------
//
//			CONSTANTS
//
//--------------------------------------

//Addresses
#define		MIN_ADDRESS					101


#define		TIMEOUT_ID					5.0





//--------------------------------------
//
//			RATE ADAPTATION
//
//--------------------------------------

//STats and adaptation every PK_ID_MODULO packets
#define		PK_ID_MODULO				1000

//We consider a capacity achievable if its delivery ratio is superior to MIN_DELIVERY_RATIO
#define		MIN_DELIVERY_RATIO			0.95

//The required precision for the inter_pk_time
#define		PRECISION_REQUIRED			0.95

//Time between two tests 
//(to flush all the packets in the MAC layers, else a buffer can be full when a new rate is tested)
#define		INTERVALL_RATES_TEST		60.0

//to start the packet generation
#define		TIME_START_PK_GENERATION	60.0

//The maximum delay allowed to consider the reception of a packet valid
#define		MAX_DELAY					5.0

//The inter packet time decreases in x%
#define		STEP_RATE_ADAPT		   		0.02




//--------------------------------------
//
//				STREAMS
//
//--------------------------------------

#define		STREAM_FROM_MAC				0
#define		STREAM_TO_MAC				0

#define		STREAM_FROM_UP				1
#define		STREAM_TO_UP				1




//-----------------------------------------------
//				FRAME ID LIST
//-----------------------------------------------


typedef struct{
	int		id;
	double	timeout;
}id_struct;



//--------------------------------------
//
//			GENERAL STATS
//
//--------------------------------------



int		nb_nodes = 0;
int		timestamp = 0;

//-----------------------------------------------
//				FRAME ID LIST
//-----------------------------------------------


typedef struct{
	int		id;
	double	timeout;
}id_timeout_struct;


//--------------------------------------
//
//		STATS ABOUT PACKETS
//
//--------------------------------------

List 	*pk_list = NULL;
int		current_pk_id = 0;

typedef struct {
	int		source;
	int		destination;
	int		hops;
	int		pk_id;
	Boolean	received;
	double	time_sent;
	double	time_received;
} pk_info;


//--------------------------------------
//
//			THROUGHPUT
//
//--------------------------------------

double	achievable_inter_pk_time	= 0;
double	bad_inter_pk_time			= 0;
double	current_inter_pk_time		= 0;

int		last_pk_id_stats			= 0;
int		lowest_pk_id_authorized		= 0;
Boolean	rate_adapt_scheduled	   	= 0;





//--------------------------------------
//
//			ROUTES OF ALL NODES
//
//--------------------------------------

// all_routes : a list of routes toward the sink (a route is a list of ids: source, next_hop, .... , sink)
List* all_routes = NULL;





//--------------------------------------
//
//			MAX_ADDRESSES
//
//--------------------------------------


int		max_x_int = 0;
int		max_y_int = 0;






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
	Objid	                  		my_objid;
	Objid	                  		my_node_objid;
	int	                    		instrm_from_mac_;
	int	                    		outstrm_to_mac_;
	OmsT_Aa_Address_Handle	 		oms_aa_handle;
	int	                    		mac_address;
	Ici*	                   		wlan_mac_req_iciptr;
	int	                    		next_hop;
	int	                    		pk_destination;
	int	                    		routing_type;
	double	                 		radio_range;
	int	                    		self_position;
	int	                    		my_stat_id;
	int	                    		rate_adaptation;
	int	                    		beta;
	List*	                  		my_route_to_sink;
	Boolean	                		is_border_node;
	int	                    		mac_backoff_type;
	List*	                  		id_list;
	int	                    		DEBUG;
	int	                    		is_sink;
	double	                 		grid_side;
	} wifi_interface_auto_state;

#define pr_state_ptr            		((wifi_interface_auto_state*) SimI_Mod_State_Ptr)
#define my_objid                		pr_state_ptr->my_objid
#define my_node_objid           		pr_state_ptr->my_node_objid
#define instrm_from_mac_        		pr_state_ptr->instrm_from_mac_
#define outstrm_to_mac_         		pr_state_ptr->outstrm_to_mac_
#define oms_aa_handle           		pr_state_ptr->oms_aa_handle
#define mac_address             		pr_state_ptr->mac_address
#define wlan_mac_req_iciptr     		pr_state_ptr->wlan_mac_req_iciptr
#define next_hop                		pr_state_ptr->next_hop
#define pk_destination          		pr_state_ptr->pk_destination
#define routing_type            		pr_state_ptr->routing_type
#define radio_range             		pr_state_ptr->radio_range
#define self_position           		pr_state_ptr->self_position
#define my_stat_id              		pr_state_ptr->my_stat_id
#define rate_adaptation         		pr_state_ptr->rate_adaptation
#define beta                    		pr_state_ptr->beta
#define my_route_to_sink        		pr_state_ptr->my_route_to_sink
#define is_border_node          		pr_state_ptr->is_border_node
#define mac_backoff_type        		pr_state_ptr->mac_backoff_type
#define id_list                 		pr_state_ptr->id_list
#define DEBUG                   		pr_state_ptr->DEBUG
#define is_sink                 		pr_state_ptr->is_sink
#define grid_side               		pr_state_ptr->grid_side

/* This macro definition will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE
#define FIN_PREAMBLE	wifi_interface_auto_state *op_sv_ptr = pr_state_ptr;


/* Function Block */

enum { _block_origin = __LINE__ };
static void
wlan_mac_higher_layer_intf_sv_init ()
	{
	int			type_of_service;

	/** Initializes all state variables used in this	**/
	/** process model.									**/
	FIN (wlan_mac_higher_layer_intf_sv_init ());

	/* Object identifier for the surrounding module and node.	*/
	my_objid = op_id_self ();
	my_node_objid = op_topo_parent (my_objid);

	/* Determine the destination to which packet should	*/
	/* be sent,and the prioritization to be provided to	*/
	/* the transmitted packet.							*/
	op_ima_obj_attr_get (my_objid, "Type of Service", 	  &type_of_service);

	/* Some interface control information is needed to	*/
	/* indicate to the MAC of the destination to which	*/
	/* a given packet needs to be sent. Create it.		*/
	wlan_mac_req_iciptr = op_ici_create ("wlan_mac_request");
	op_ici_attr_set (wlan_mac_req_iciptr, "type_of_service", type_of_service);
	op_ici_attr_set (wlan_mac_req_iciptr, "protocol_type",   0x800);

	FOUT;
	}

static void
wlan_mac_higher_layer_register_as_arp ()
	{
	char				proc_model_name [128];
	OmsT_Pr_Handle		own_process_record_handle;
	Prohandle			own_prohandle;

	/** Register this process in the model-wide process registry.	**/
	FIN (wlan_mac_higher_layer_register_as_arp ());

	/* Obtain the process model name and process handle.	*/
	op_ima_obj_attr_get (my_objid, "process model", proc_model_name);
	own_prohandle = op_pro_self ();

	/* Register this process in the model-wide process registry	*/
	own_process_record_handle = (OmsT_Pr_Handle) oms_pr_process_register (
			my_node_objid, my_objid, own_prohandle, proc_model_name);

	/* Register this protocol attribute and the element address	*/
	/* of this process into the model-wide registry.			*/
	oms_pr_attr_set (own_process_record_handle,
		"protocol",		OMSC_PR_STRING,		"arp",
		OPC_NIL);

	FOUT;
	}





//-----------------------------------------------------------
//
//					DUPLICATA DETECTION
//
//-----------------------------------------------------------


//Deletes the frame_id which became obsolete
void del_timeouted_id(Vartype* tot , int code){
	//double		older_entry = 0;
	int				i;
	id_struct		*ptr;
	
	//Walks in the list
	for(i= op_prg_list_size(id_list)-1 ; i>= 0 ; i--){
		ptr = op_prg_list_access(id_list , i);
		
		//Timeouted entry !
		if (ptr->timeout <= op_sim_time()){
			ptr = op_prg_list_remove(id_list , i);
			op_prg_mem_free(ptr);
		}
	}
	
	
	//Next verification
	if (op_prg_list_size(id_list) != 0)
		op_intrpt_schedule_call(op_sim_time() + TIMEOUT_ID , 0 , del_timeouted_id , NULL);
}


//Is thie frame_id already in the list ?
Boolean is_id_seen(int pk_id){
	id_struct		*ptr;
	int				i;
	
	for(i=0 ; i < op_prg_list_size(id_list) ; i++){
		ptr = op_prg_list_access(id_list , i);
		
		if (ptr->id == pk_id)
			return(OPC_TRUE);
	}
	
	return(OPC_FALSE);
}



//adds the frame_id as already seen
void add_id_seen(int pk_id){
	id_struct	*ptr;
	
	//the frame id is already in the list
	if (is_id_seen(pk_id))
		return;
	
	//Empty list -> timeouted verificaiton
	if (op_prg_list_size(id_list) == 0)
		op_intrpt_schedule_call(op_sim_time() + 1.0 , 0 , del_timeouted_id , NULL);
	
	//New entry
	ptr = op_prg_mem_alloc(sizeof(id_struct));
	ptr->id			= pk_id;
	ptr->timeout	= op_sim_time() + TIMEOUT_ID;
	op_prg_list_insert(id_list , ptr , OPC_LISTPOS_TAIL);
}









//-----------------------------------------------------------
//
//							TOOLS
//
//-----------------------------------------------------------


//is this int positive or negative ?
short signe(int value){
	if (value < 0)
		return -1;
	else if (value > 0)
		return 1;
	else
		return 0;
}


//returns the euclidyan distance
double get_distance(double x1 , double y1 ,double x2 , double y2){
	return (sqrt (  pow(x1 - x2, 2) + pow(y1 - y2, 2)));
}	


//Is the node in the circle of radius r and center x/y ?
short is_in_circle(int address , double x , double y , double radius){
	int		x_addr , y_addr;

	y_addr = address /  100;
	x_addr = address - y_addr * 100;

	if (get_distance(x_addr , y_addr, x , y) < radius)
		return(OPC_TRUE);
	else
		return(OPC_FALSE);
}




//-----------------------------------------------------------
//
//							ROUTINGS
//
//-----------------------------------------------------------


//-----------------------------------------------------------
//						SHORT - ROUTING
//-----------------------------------------------------------
//returns the next hop via the shortest route tin hops (almost the same as XY routing, but without shortcuts)
int get_next_hop_via_shortest_routing(int range_tmp , int destination_tmp){
	int 	next_hop_tmp;
	int		x_dev , y_dev;
	int		dev = range_tmp / sqrt(2);		// Max mobility via diagonale
	
	y_dev = (int) (mac_address / 100) - (int) (destination_tmp /100);
	x_dev = mac_address - destination_tmp - y_dev*100;
		
	
	//printf("mac %d, x_dev %d y_dev %d\n", mac_address, x_dev , y_dev);
	
	//X-axis
	if (x_dev == 0){
		next_hop_tmp = mac_address - signe(y_dev) * 100 * min (range_tmp , fabs(y_dev));
	}
	//X-axis
	else if (y_dev == 0){
		next_hop_tmp = mac_address - signe(x_dev) * min (range_tmp , fabs(x_dev));
	}	
	//Sides diag
	else if ((fabs(y_dev) < range_tmp) && (fabs(x_dev) < range_tmp)){
		next_hop_tmp = mac_address - y_dev * 100 ;
		next_hop_tmp = next_hop_tmp - signe (x_dev) * min (fabs(x_dev) , floor(sqrt( pow(radio_range , 2) - pow(y_dev, 2) ))  );				
	}
	else if (fabs(y_dev) < range_tmp){
		
		next_hop_tmp = mac_address - y_dev * 100 ;
		next_hop_tmp = next_hop_tmp - signe (x_dev) * floor(sqrt( pow(radio_range , 2) - pow(y_dev, 2) ));				

	}
	else if (fabs(x_dev) < range_tmp) {
		
		next_hop_tmp = mac_address - x_dev ;
		next_hop_tmp = next_hop_tmp - signe (y_dev) * 100 * floor(sqrt( pow(radio_range , 2) - pow(x_dev, 2) ));				

	}
	//diag
	else{
		if (dev > 0)
			next_hop_tmp = mac_address - signe(x_dev) * dev - signe(y_dev) * dev *100;
		else if (x_dev >= y_dev)
			next_hop_tmp = mac_address - signe(x_dev) * range_tmp;
		else if (y_dev > x_dev)
			next_hop_tmp = mac_address - signe(y_dev) * range_tmp * 100;
	}
		
	
	return(next_hop_tmp);
}





//-----------------------------------------------------------
//				  	OPT-CORNER  -  ROUTING
//-----------------------------------------------------------
//returns the next hop for the optimal routing scheme, via the axis (but avoiding the interferences circle)
int get_next_hop_via_opt_corner_sides_routing(int range_tmp , int destination_tmp , double x_center , double y_center , double radius){
	int 	next_hop_tmp;
	int		x_dev , y_dev , next_hop_x_dev , next_hop_y_dev;
	int		next_hop_tmp_sav;
	int		dev = range_tmp / sqrt(2);		// Max mobility via diagonale

	y_dev = (int) (mac_address / 100) - (int) (destination_tmp /100);
	x_dev = mac_address - destination_tmp - y_dev*100;

	//printf("addr %d %d %d\n", mac_address , x_dev , y_dev);
	
	//YX Routing
	if (x_dev > y_dev){
		if (y_dev != 0){
			if (fabs(y_dev) < range_tmp){		
			
				next_hop_tmp = mac_address - y_dev * 100 ;
				next_hop_tmp = next_hop_tmp - signe (x_dev) * floor(sqrt( pow(radio_range , 2) - pow(y_dev, 2) ));				

			}
			else
				next_hop_tmp = mac_address - signe(y_dev) * range_tmp * 100;


			//backup
			next_hop_tmp_sav = next_hop_tmp;

			
			next_hop_y_dev = (int) (next_hop_tmp / 100) - (int) (destination_tmp /100);
			next_hop_x_dev = next_hop_tmp - destination_tmp - y_dev*100;
			
			//Next hop in the interference circle and not in one of the authorized axis -> bypass the circle
			while (	(is_in_circle(next_hop_tmp , x_center, y_center , radius)) 
					&& 
					(!is_in_circle(mac_address , x_center, y_center , radius)) 
					&& 
					(next_hop_y_dev != 0)
				){				
				next_hop_tmp += 1;			
			}
			
			//Overflow (outside the grid)
			y_dev = (int) (next_hop_tmp / 100);
			x_dev = next_hop_tmp - y_dev*100;
			if ((x_dev > max_x_int) || (y_dev > max_y_int))
				next_hop_tmp = next_hop_tmp_sav;

		}
		else{
			next_hop_tmp = mac_address - min(range_tmp , x_dev);
		}
	}
	//XY Routing
	else{
		if (x_dev != 0){
			if (fabs(x_dev) < range_tmp){
			
				next_hop_tmp = mac_address - x_dev ;
				next_hop_tmp = next_hop_tmp - 100 * signe (y_dev) * floor(sqrt( pow(radio_range , 2) - pow(x_dev, 2) ));
			}
			else
				next_hop_tmp = mac_address - signe(x_dev) * range_tmp;

			//backup
			next_hop_tmp_sav = next_hop_tmp;
			
			next_hop_y_dev = (int) (next_hop_tmp / 100) - (int) (destination_tmp /100);
			next_hop_x_dev = next_hop_tmp - destination_tmp - y_dev*100;
			
			//Next hop in the interference circle and not in one of the authorized axis -> bypass the circle
			while (	(is_in_circle(next_hop_tmp , x_center, y_center , radius)) 
					&& 
					(!is_in_circle(mac_address , x_center, y_center , radius)) 
					&& 
					(next_hop_x_dev != 0)
				){				
				next_hop_tmp += 100 ;
				next_hop_x_dev = next_hop_tmp - destination_tmp - y_dev*100;
			}
			
			//Overflow (outside the grid)
			y_dev = (int) (next_hop_tmp / 100);
			x_dev = next_hop_tmp - y_dev*100;
			if ((x_dev > max_x_int) || (y_dev > max_y_int))
				next_hop_tmp = next_hop_tmp_sav;
				

		}
		else{
			next_hop_tmp = mac_address - min(range_tmp * 100 , y_dev * 100);
		}
	}
	
	
	
	return(next_hop_tmp);	
}



 
//-----------------------------------------------------------
//						XY - ROUTING
//-----------------------------------------------------------
//returns the next hop via the XY routing toward the destination (Along Y and then along X)
int get_next_hop_via_xy_routing(int range_tmp , int destination_tmp){
	int 	next_hop_tmp;
	int		x_dev , y_dev;
	char	msg[200];
	int		dev = range_tmp / sqrt(2);		// Max mobility via diagonale
	
	y_dev = (int) (mac_address / 100) - (int) (destination_tmp /100);
	x_dev = mac_address - destination_tmp - y_dev*100;
		

	//Along the Y-coordinate
	if (y_dev > 0){
		next_hop_tmp = mac_address - 100 * range_tmp;
		while ((int) (next_hop_tmp / 100) < (int) (pk_destination / 100))
			next_hop_tmp += 100;
	}
	else if (y_dev < 0){
		next_hop_tmp = mac_address + 100 * range_tmp;
		
		while (((int) (next_hop_tmp / 100) > (int) (pk_destination / 100)) || (next_hop_tmp < MIN_ADDRESS))
			next_hop_tmp -= 100;
	}		
		
	//Along the X-coordinate
	else if (x_dev > 0) {
		next_hop_tmp = mac_address - range_tmp;
		while ((next_hop_tmp < pk_destination) || (next_hop_tmp < MIN_ADDRESS))
			next_hop_tmp ++;
	}
	else if (x_dev < 0) {
		next_hop_tmp = mac_address + range_tmp;
		while (next_hop_tmp > pk_destination)
			next_hop_tmp --;
	}
	else if (!is_sink){
		sprintf(msg , "No next hop toward %d for the node %d", pk_destination , mac_address);
		op_sim_end("Fatal error" , msg , "" , "");
	}
		
	//Final result !
	return(next_hop_tmp);
}




//-----------------------------------------------------------
//						SIDES - ROUTING
//-----------------------------------------------------------
//returns the next hop via a routing via axis centered on the destination
int get_next_hop_via_sides_routing(int range_tmp , int destination_tmp){
	int 	next_hop_tmp;
	int		x_dev , y_dev;
	int		dev = range_tmp / sqrt(2);		// Max mobility via diagonale
	
	y_dev = (int) (mac_address / 100) - (int) (destination_tmp /100);
	x_dev = mac_address - destination_tmp - y_dev*100;
	
	//YX Routing
	if (fabs(x_dev) > fabs(y_dev)){
		if (y_dev != 0)
				next_hop_tmp = mac_address -  100 * signe(y_dev) * min(range_tmp , fabs(y_dev));
		else
			next_hop_tmp = mac_address - signe(x_dev) * min(range_tmp , fabs(x_dev));
	}
	//XY Routing
	else{
		if (x_dev != 0)
				next_hop_tmp = mac_address - signe(x_dev) * min(range_tmp , fabs(x_dev));
		
		else
			next_hop_tmp = mac_address - signe(y_dev) * 100 * min(range_tmp , fabs(y_dev));
	}

	return(next_hop_tmp);
}



//-----------------------------------------------------------
//		SIDES 2 - ROUTING (combination sides + short)
//-----------------------------------------------------------
//returns the next hop via a routing via axis centered on the destination
int get_next_hop_via_sides_short_bis_routing(int range_tmp , int destination_tmp , double x_center , double y_center , double radius){
	int 	next_hop_tmp;
	int		x_dev , y_dev;
	
	y_dev = (int) (mac_address / 100) - (int) (destination_tmp /100);
	x_dev = mac_address - destination_tmp - y_dev*100;
		
	//YX Routing
	if (fabs(x_dev) > fabs(y_dev)){
		if (y_dev != 0){
			if (fabs(y_dev) < range_tmp){		
			
				next_hop_tmp = mac_address - y_dev * 100 ;
				next_hop_tmp = next_hop_tmp - floor(sqrt( pow(radio_range , 2) - pow(y_dev, 2) ));				

			}
			else
				next_hop_tmp = mac_address -  range_tmp * 100;

			
			
		}
		else{
			next_hop_tmp = mac_address - signe(x_dev) * min(range_tmp , fabs(x_dev));
		}
	}
	//XY Routing
	else{
		if (x_dev != 0){
			if (fabs(x_dev) < range_tmp){
				next_hop_tmp = mac_address - x_dev ;
				next_hop_tmp = next_hop_tmp - 100 * floor(sqrt( pow(radio_range , 2) - pow(x_dev, 2) ));

			}
			else
				next_hop_tmp = mac_address - range_tmp;
		}
		else{
			next_hop_tmp = mac_address - signe(y_dev) * min(range_tmp * 100 , fabs(y_dev) * 100);
		}
	}

	return(next_hop_tmp);
}




//-----------------------------------------------------------
//						OPT2 ROUTING
//-----------------------------------------------------------

//a quite particular route
int get_next_hop_via_opt2_routing(int range_tmp , int destination_tmp){
	int 	next_hop_tmp = -1;
	int		x_dev , y_dev;
	int		i;
	
	//the Sink
	if (destination_tmp == mac_address)
		return(mac_address);
	
	
	//move values
	y_dev = (int) (mac_address / 100) - (int) (destination_tmp /100);
	x_dev = mac_address - destination_tmp - y_dev*100;
	
	
	
	//Border case
	if ((x_dev == 0) || (y_dev == 0)){
	
		for(i=1 ; i <= range_tmp; i++)
			if ((y_dev == 0) && (x_dev % i == 0))
				next_hop_tmp = mac_address -  i;
			
		
		for(i=1 ; i <= range_tmp; i++)
			if ((x_dev == 0) && (y_dev % i == 0))
				next_hop_tmp = mac_address -  i * 100;
	
	}
	
	//Diag Routing
	else if ((fabs(x_dev) == fabs(y_dev)) && (fabs(x_dev) < 2 * range_tmp)){
		next_hop_tmp = mac_address -  100 * signe(y_dev);
	}
	
	//YX Routing
	else if (fabs(x_dev) > fabs(y_dev)){
		next_hop_tmp = mac_address -  100 * signe(y_dev);

	}
	//XY Routing
	else{
		next_hop_tmp = mac_address - signe(x_dev);
	}
	
	return(next_hop_tmp);
}














//-----------------------------------------------------------
//
//							STATISTICS
//
//-----------------------------------------------------------

//returns the struct pk_info associated to pk_id 
pk_info* get_pk_info_in_list(List* ll, int pk_id){
	pk_info		*elem;
	int			i;
	
	for(i=0; i<op_prg_list_size(ll); i++){
		elem = op_prg_list_access(ll , i);
		if (elem->pk_id == pk_id){
			return(elem);
		}
	}
	return(NULL);
}



//returns the delivery ratio for the packets between id1 and id2
double compute_delivery_ratio(List* ll, int id1, int id2){
	pk_info		*elem;
	int			i;
	int			nb_pk_sent 		= 0;
	int			nb_pk_received 	= 0;
	
	
	//Error
	if (id2 > op_prg_list_size(ll))
		return(0);
	
	for(i=0 ; i < op_prg_list_size(ll) ; i++){
		elem = op_prg_list_access(ll , i);
		
		//nb of sent packets
		if ((elem->pk_id <= id2) && (elem->pk_id >= id1))
			nb_pk_sent ++;
		
		//this packet was received
		if ((elem->pk_id <= id2) && (elem->pk_id >= id1) && (elem->received))
			nb_pk_received++;
	}
	return((double)nb_pk_received / nb_pk_sent);
}


//returns the average delay for the packets between id1 and id2
double compute_delay(List* ll, int id1, int id2){
	pk_info		*elem;
	int			i;
	double		delay 			= 0;
	int			nb_pk_sent 		= 0;
	int			nb_pk_received 	= 0;
	
	
	//Error
	if (id2 > op_prg_list_size(ll))
		return(0);
	
	for(i=0 ; i < op_prg_list_size(ll) ; i++){
		elem = op_prg_list_access(ll , i);
		
		//nb of sent packets
		if ((elem->pk_id <= id2) && (elem->pk_id >= id1))
			nb_pk_sent ++;
		
		//this packet was received
		if ((elem->pk_id <= id2) && (elem->pk_id >= id1) && (elem->received)){
			nb_pk_received++;
			delay += elem->time_received - elem->time_sent;
		}
	}
	return(delay / nb_pk_received);
}



//returns the max delay for the packets between id1 and id2
double compute_max_delay(List* ll, int id1, int id2){
	pk_info		*elem;
	int			i;
	double		max_delay 		= 0;
	int			nb_pk_sent 		= 0;
	int			nb_pk_received 	= 0;
	
	
	//Error
	if (id2 > op_prg_list_size(ll))
		return(0);
	
	for(i=0 ; i < op_prg_list_size(ll) ; i++){
		elem = op_prg_list_access(ll , i);
		
		//nb of sent packets
		if ((elem->pk_id <= id2) && (elem->pk_id >= id1))
			nb_pk_sent ++;
		
		//this packet was received
		if ((elem->pk_id <= id2) && (elem->pk_id >= id1) && (elem->received)){
			nb_pk_received++;
			if (elem->time_received - elem->time_sent > max_delay)
				max_delay = elem->time_received - elem->time_sent;
		}
	}
	return(max_delay);
}





//-----------------------------------------------------------
//
//							DEBUG
//
//-----------------------------------------------------------


//Writes a file containing info about the received/generated packets
void debug_write_pk_info(){

	int			i;
	FILE* 		pfile;
	char		filename[200];
	char		msg[500];
	//Stats
	pk_info		*elem;
	int			nb_pk_received = 0;
	int			nb_pk_sent = 0;

	sprintf(filename , "results_80211/%d_debug_pk.txt", (int) timestamp + (int)op_dist_uniform(128));
	pfile = fopen(filename , "w");


	if (pfile == NULL){
		sprintf(msg, "Filename %s creates an error: %s", filename, strerror(errno));
		printf("ERROR: %d (%s)\n", errno, strerror(errno));
		op_sim_end(msg, "" , "" , "");
	}

	fprintf(pfile , "SRC	DEST	HOPS	RECEIVED	DELAY		PK_ID	SENT\n");
	for(i=0; i <op_prg_list_size(pk_list) ; i++){
		elem = op_prg_list_access(pk_list, i);
		fprintf(pfile , "%d	%d	%d	%d		%f	%d	%f\n" , elem->source , elem->destination , elem->hops , elem->received , elem->time_received - elem->time_sent , elem->pk_id , elem->time_sent);
		
	}
	fclose(pfile);
}





//-----------------------------------------------------------
//
//			   INTERMEDIARY	STATS
//
//-----------------------------------------------------------


//Prints parameters, etc ...
void print_headers_stat_file(FILE *pfile){
	//Simulation parameters
	double	RTS;
	int		BETA;
	int		routing_mac = 0;
	int		routing_up = 0;
	int		position;
	double	position_parameter;
	double	priv_maxtime;
	int		channels;

	
	
	
	//-------------------------------------------
	//				PARAMETERS
	//-------------------------------------------
	
	op_ima_sim_attr_get(OPC_IMA_DOUBLE , 	"RTS" , 			&RTS);
	op_ima_sim_attr_get(OPC_IMA_INTEGER , 	"BETA" , 			&BETA);
	
	if (op_ima_sim_attr_exists("ROUTING_MAC"))
		op_ima_sim_attr_get(OPC_IMA_INTEGER , 	"ROUTING_MAC" , &routing_mac);
	else
		routing_mac = -1;
	
	if (op_ima_sim_attr_exists("ROUTING_UP"))
		op_ima_sim_attr_get(OPC_IMA_INTEGER , 	"ROUTING_UP" ,	&routing_up);
	else
		routing_up = -1;
	
	if (op_ima_sim_attr_exists("POSITION"))
		op_ima_sim_attr_get(OPC_IMA_INTEGER , 	"POSITION" ,	&position);
	else
		position = -1;
	
	if (op_ima_sim_attr_exists("POSITION_PARAMETER"))
		op_ima_sim_attr_get(OPC_IMA_DOUBLE , 	"POSITION_PARAMETER" ,	&position_parameter);
	else
		position_parameter = -1;
	
	if (op_ima_sim_attr_exists("PRIVILEGED_MAX_TIME"))
		op_ima_sim_attr_get(OPC_IMA_DOUBLE , 	"PRIVILEGED_MAX_TIME" ,	&priv_maxtime);
	else
		priv_maxtime = -1;
	
	if (op_ima_sim_attr_exists("CHANNELS"))
		op_ima_sim_attr_get(OPC_IMA_INTEGER , 	"CHANNELS" ,	&channels);
	else
		channels = -1;
	


	fprintf(pfile , " ------------------------------------------------------------------------------------------------------------\n");
	fprintf(pfile , "|                                           SIMULATION PARAMETERS                                            |\n");
	fprintf(pfile , " ------------------------------------------------------------------------------------------------------------\n");

	fprintf(pfile , "------------------------------------------------  GENERAL ---------------------------------------------------\n");
	fprintf(pfile , "Number of nodes							:	%d\n", 			nb_nodes);
	fprintf(pfile , "Radio Range								:	%f\n", 			radio_range);
	fprintf(pfile , "Grid Side								:	%f\n", 				grid_side);
	fprintf(pfile , "Position								:	%d\n", 				position);
	fprintf(pfile , "Position parameter						:	%f\n", 				position_parameter);
	fprintf(pfile , "Duration								:	%f\n", 				op_sim_time());
	fprintf(pfile , "RTS									:	%f\n", 				RTS);
	fprintf(pfile , "Inter Packet Time							:	%f\n", 			current_inter_pk_time);
	fprintf(pfile , "Privileged Max Time						:	%f\n", 			priv_maxtime);
	fprintf(pfile , "Nb of channels							:	%d\n", 				channels);
	fprintf(pfile , "Routing MAC								:	%d\n", 			routing_mac);
	fprintf(pfile , "Routing Up								:	%d\n", 				routing_up);
	fprintf(pfile , "Beta									:	%d\n", 				BETA);
	fprintf(pfile , "\n");
	fprintf(pfile , "------------------------------------------------  FLOWS ---------------------------------------------------\n");
	fprintf(pfile , "Is rate dynamical ?						:	%d\n", 			rate_adaptation);
	fprintf(pfile , "Rate adapted every 						:	%d\n", 			PK_ID_MODULO);
	fprintf(pfile , "Maximal Difference valid / bad				:	%f\n", 			PRECISION_REQUIRED);
	fprintf(pfile , "Valid Capacity if dlivery ratio >				:	%f\n", 		MIN_DELIVERY_RATIO);
	fprintf(pfile , "Time between tests						:	%f\n", 				INTERVALL_RATES_TEST);
}



//Write delivery ratio, delay, max_delay
void write_intermediary_stats(int low_pk_id , int high_pk_id){
	//results
	double	delivery_ratio;
	double	delay;
	double	max_delay;
	//control
	FILE	*pfile;
	char	msg[500];
	char	filename[150];
	

	//-------------------------------------------
	//				STATS
	//-------------------------------------------

	
	delivery_ratio	= compute_delivery_ratio(pk_list , low_pk_id + PK_ID_MODULO / 10 , high_pk_id);
	delay			= compute_delay			(pk_list , low_pk_id + PK_ID_MODULO / 10 , high_pk_id);
	max_delay		= compute_max_delay		(pk_list , low_pk_id + PK_ID_MODULO / 10 , high_pk_id);

	//printf(" %f - %d %d\n", delivery_ratio , low_pk_id + PK_ID_MODULO / 10 , high_pk_id);	
	
	
	
	
	//-------------------------------------------
	//					WRITING
	//-------------------------------------------
	sprintf(filename , "results_80211/%d-nodes_%d-stats.txt", timestamp , nb_nodes);
	pfile = fopen(filename , "w");


	if (pfile == NULL){
		sprintf(msg, "Filename %s creates an error: %s", filename, strerror(errno));
		printf("ERROR: %d (%s)\n", errno, strerror(errno));
		op_sim_end(msg, "" , "" , "");
	}
	
	//Simulation parameters
	print_headers_stat_file(pfile);
	
		
	fprintf(pfile , "\n\n\n ------------------------------------------------------------------------------------------------------------\n");
	fprintf(pfile , "|                                                  RESULTS                                                   |\n");
	fprintf(pfile , " ------------------------------------------------------------------------------------------------------------\n");

	fprintf(pfile , "Delivery Ratio							:	%f\n", 					delivery_ratio);
	fprintf(pfile , "Average Delay (in s)						:	%f\n", 				delay);
	fprintf(pfile , "Max Delay (in s) 							:	%f\n", 				max_delay);
	
	fclose(pfile);
}	

//-----------------------------------------------------------
//
//			   ADAPT THE RATE (inter packet time)
//
//-----------------------------------------------------------


//Updates the parameter value in the source process
void update_packet_arrival_in_source_process(double inter_pk_time , double start_time){
	//topo
	int		node_id;
	//control
	int		i;
	
	//Sets the new inter pk time for all the nodes (execpt the sink :-)
	for(i=0 ; i < op_topo_object_count(OPC_OBJTYPE_NDMOB) ; i++){
   	
		node_id = op_topo_object(OPC_OBJTYPE_NDMOB , i);
		if (op_ima_obj_attr_exists(node_id , "source.Packet Interarrival Arg1"))
			op_ima_obj_attr_set(node_id , "source.Packet Interarrival Arg1" , inter_pk_time);
		
		if (op_ima_obj_attr_exists(node_id , "source.Start Time Packet Generation"))
			op_ima_obj_attr_set(node_id , "source.Start Time Packet Generation" , start_time);
	}
}

//adapt the inter packet time
void adapt_rate(void *arg , int code){
	//stats
	int		pk_id;
	double	delivery_ratio;
	double	next_start_time;

	//pk_id_tmp
	pk_id = *(int*)arg;
	op_prg_mem_free(arg);
	
	//Next scheduled
	rate_adapt_scheduled 		= OPC_FALSE;

	
	//delivery ratio
	delivery_ratio = compute_delivery_ratio(pk_list , lowest_pk_id_authorized + PK_ID_MODULO / 100 , pk_id);
	printf(" %f - %d %d\n", delivery_ratio , lowest_pk_id_authorized + PK_ID_MODULO /100 , pk_id);
	if (lowest_pk_id_authorized + PK_ID_MODULO /100  > pk_id)
		return;
	
	//Other packets in transit must be dropped
	lowest_pk_id_authorized 	= current_pk_id;
	last_pk_id_stats			= current_pk_id;
	
	//Inter packet time change -> Is the delivery ratio sufficient ?
	if (delivery_ratio > MIN_DELIVERY_RATIO){
		
		//Update best value
		if ((achievable_inter_pk_time > current_inter_pk_time) || (achievable_inter_pk_time == 0))
			achievable_inter_pk_time = current_inter_pk_time;
			
		//Next value to test
		current_inter_pk_time = achievable_inter_pk_time - achievable_inter_pk_time * STEP_RATE_ADAPT;
		
		//Stop the traffic for INTERVALL_RATES_TEST (flush the buffers)
		next_start_time = op_sim_time() + INTERVALL_RATES_TEST;
			
	}
	//The delivery ratio is lower than the treshhold -> The capacity is inferior
	else
		next_start_time = OPC_DBL_INFINITY;
			
	
	//updates the values in the source process
	update_packet_arrival_in_source_process(current_inter_pk_time , next_start_time);


	//DEBUG
	//debug_write_pk_info();	
	printf(" %f (%f) ", current_inter_pk_time , delivery_ratio);
	
	

}


//We compute the required stats -> stop the simulation
void stop_rate(void *arg , int code){
	//max pk_id to compute our stats
	int		pk_id;
	
	//pk_id_tmp
	pk_id = *(int*)arg;
	op_prg_mem_free(arg);
	
	//Next scheduled
	rate_adapt_scheduled 		= OPC_FALSE;

	//not enough packets
	if (lowest_pk_id_authorized + PK_ID_MODULO / 10 > pk_id)
		return;
	
	//Write all stats in a file
	write_intermediary_stats(lowest_pk_id_authorized , pk_id);

	//Other packets in transit must be dropped
	lowest_pk_id_authorized 	= current_pk_id;

	
	//simulation end !
	op_sim_end("The stats were computed" , "" , "" , "");
}




//-----------------------------------------------------------
//
//			   ROUTES MANAGEMENT
//
//-----------------------------------------------------------


//Prints the content of a route (route_length, source, ..., destination)
void print_route(List* route_tmp){
	int		i;
	int		*int_ptr;

	printf("ROUTE : ");
	for(i=0 ; i < op_prg_list_size(route_tmp) ; i++){
		int_ptr = op_prg_list_access(route_tmp, i);
	
		printf(" %d" , *int_ptr);
		}
	printf("\n");
}










//-----------------------------------------------------------
//
//			  			 TRANSMISSION
//
//-----------------------------------------------------------

void pk_send_to_mac(Packet * pkptr , int next_hop_tmp){
	double	power_ratio;
	int		x_dist , y_dist;
	double	dist;

	//Power ratio (compared to the maximum power)
	y_dist = (mac_address - next_hop_tmp) / 100;
	x_dist = mac_address - next_hop_tmp - y_dist * 100;
	dist = sqrt( pow(x_dist , 2) + pow(y_dist , 2));

	//radio propagation model in alpha = 4
	if (routing_type == OPT2_ROUTING)
		power_ratio = pow(dist / (radio_range / grid_side) , 4);
	else
		power_ratio = 1;


	//Packet transmission
	op_ici_attr_set (wlan_mac_req_iciptr, "dest_addr", 		next_hop_tmp);
//	op_ici_attr_set (wlan_mac_req_iciptr, "power_ratio", 	power_ratio);
	op_ici_install (wlan_mac_req_iciptr);
	op_pk_send (pkptr, STREAM_TO_MAC);

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
	void wifi_interface_auto (void);
	Compcode wifi_interface_auto_init (void **);
	void wifi_interface_auto_diag (void);
	void wifi_interface_auto_terminate (void);
	void wifi_interface_auto_svar (void *, const char *, char **);
#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
wifi_interface_auto (void)
	{
	int _block_origin = 0;
	FIN (wifi_interface_auto ());
	if (1)
		{
		List*				proc_record_handle_list_ptr;
		int					record_handle_list_size;
		OmsT_Pr_Handle		process_record_handle;
		Objid				mac_module_objid;
		Boolean				dest_addr_okay = OPC_FALSE;
		double				ne_address = OPC_DBL_UNDEF;
		OmsT_Aa_Address_Info * ith_address_info_ptr;


		FSM_ENTER (wifi_interface_auto)

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (init) enter executives **/
			FSM_STATE_ENTER_UNFORCED_NOLABEL (0, "init", "wifi_interface_auto () [init enter execs]")
				{
				//Addresses
				char		str[500];
				int			my_address;
				//Control
				int			i;
				//Topology
				int			node_id;
				int			mac_id;
				//MAC process name
				char		mac_name[400];
				
				
				
				
				/* Initialize the state variables used by this model.	*/
				wlan_mac_higher_layer_intf_sv_init ();
				
				/* Register this process as "arp" so that lower layer	*/
				/* MAC process can connect to it.						*/
				wlan_mac_higher_layer_register_as_arp ();
				
				/* Schedule a self interrupt to wait for lower layer	*/
				/* wlan MAC process to initialize and register itself in	*/
				/* the model-wide process registry.						*/
				op_intrpt_schedule_self (op_sim_time (), 0);
				
				
				
				
				//----------------------------------------------------
				//
				//					My address
				//
				//-----------------------------------------------------
				
				op_ima_obj_attr_get(op_topo_parent(op_id_self()) , "name" , str);
				my_address = atoi(str);
				mac_id = op_topo_assoc (op_id_self(), OPC_TOPO_ASSOC_OUT, OPC_OBJTYPE_PROC, STREAM_TO_MAC);
				op_ima_obj_attr_set(mac_id , "Address" , 	my_address);
				
				
				
				
				
				//--------------------------------------------
				//
				//					PARAMETERS
				//
				//--------------------------------------------
				
				
				op_ima_sim_attr_get(OPC_IMA_INTEGER, 	"ROUTING_UP" , 						&routing_type);
				op_ima_sim_attr_get(OPC_IMA_DOUBLE, 	"RADIO_RANGE" , 					&radio_range);
				op_ima_sim_attr_get(OPC_IMA_INTEGER, 	"BETA" , 							&beta);
				op_ima_obj_attr_get(op_id_self(), 		"Destination" , 					&pk_destination);
				op_ima_sim_attr_get(OPC_IMA_INTEGER , 	"BACKOFF_TYPE" , 					&mac_backoff_type);
				op_ima_sim_attr_get(OPC_IMA_INTEGER , 	"RATE_ADAPTATION" , 				&rate_adaptation);
				op_ima_sim_attr_get(OPC_IMA_INTEGER , 	"DEBUG" , 							&DEBUG);
				
				
				if (current_inter_pk_time == 0)
					op_ima_sim_attr_get(OPC_IMA_DOUBLE , 	"INITIAL_INTER_PK_TIME" , 		&current_inter_pk_time);
				
				
				
				
				//----------------------------------------------------
				//
				//						STATS
				//
				//-----------------------------------------------------
				
				
				if (pk_list == NULL)
					pk_list = op_prg_list_create();
				
				if (all_routes == NULL)
					all_routes = op_prg_list_create();
				
				id_list = op_prg_list_create();
				
				my_route_to_sink = op_prg_list_create();
				
				
				
				//----------------------------------------------------
				//
				//			   INTER PACKET TIME
				//
				//-----------------------------------------------------
				
				
				for(i=0 ; i < op_topo_object_count(OPC_OBJTYPE_NDMOB) ; i++){
				 	
					node_id = op_topo_object(OPC_OBJTYPE_NDMOB , i);
					if (op_ima_obj_attr_exists(node_id , "source.Packet Interarrival Arg1"))
						op_ima_obj_attr_set(node_id , "source.Packet Interarrival Arg1" , 	current_inter_pk_time);
						
					if (op_ima_obj_attr_exists(node_id , "source.Start Time Packet Generation"))
						op_ima_obj_attr_set(node_id , "source.Start Time Packet Generation" , TIME_START_PK_GENERATION);
				}
				
				
				
				
				//----------------------------------------------------
				//
				//						CONTROL
				//
				//-----------------------------------------------------
				
				if (nb_nodes == 0)
					timestamp = time(NULL);
				
				my_stat_id = nb_nodes;
				nb_nodes++;
				
				
				
				
				
				
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (1,wifi_interface_auto)


			/** state (init) exit executives **/
			FSM_STATE_EXIT_UNFORCED (0, "init", "wifi_interface_auto () [init exit execs]")
				{
				/* Schedule a self interrupt to wait for lower layer	*/
				/* wlan MAC process to initialize and register itself in*/
				/* the model-wide process registry.						*/
				op_intrpt_schedule_self (op_sim_time (), 0);
				
				}


			/** state (init) transition processing **/
			FSM_TRANSIT_FORCE (5, state5_enter_exec, ;, "default", "", "init", "init2")
				/*---------------------------------------------------------*/



			/** state (idle) enter executives **/
			FSM_STATE_ENTER_UNFORCED (1, state1_enter_exec, "idle", "wifi_interface_auto () [idle enter execs]")
				{
				
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,wifi_interface_auto)


			/** state (idle) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "idle", "wifi_interface_auto () [idle exit execs]")
				{
				/* The only interrupt expected in this state is a	*/
				/* stream interrupt. It can be either from the MAC	*/
				/* layer for a packet destined for this node or		*/
				/* from the application layer for a packet destined	*/
				/* for some other node.								*/
				/*
				
				intrpt_type = op_intrpt_type ();
				
				if (intrpt_type == OPC_INTRPT_STRM){
					intrpt_strm = op_intrpt_strm ();
					pkptr = op_pk_get (intrpt_strm);
				}
				
				if (intrpt_type == OPC_INTRPT_SELF){
					intrpt_code = op_intrpt_code ();
				}
				
				
				*/
				}


			/** state (idle) transition processing **/
			FSM_INIT_COND (APPL_LAYER_PKT_ARVL && !ENDSIM)
			FSM_TEST_COND (MAC_LAYER_PKT_ARVL && !ENDSIM)
			FSM_TEST_COND (ENDSIM)
			FSM_TEST_LOGIC ("idle")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 2, state2_enter_exec, ;, "APPL_LAYER_PKT_ARVL && !ENDSIM", "", "idle", "appl layer arrival")
				FSM_CASE_TRANSIT (1, 3, state3_enter_exec, ;, "MAC_LAYER_PKT_ARVL && !ENDSIM", "", "idle", "mac layer arrival")
				FSM_CASE_TRANSIT (2, 6, state6_enter_exec, ;, "ENDSIM", "", "idle", "END_SIM")
				}
				/*---------------------------------------------------------*/



			/** state (appl layer arrival) enter executives **/
			FSM_STATE_ENTER_FORCED (2, state2_enter_exec, "appl layer arrival", "wifi_interface_auto () [appl layer arrival enter execs]")
				{
				
				pk_info	*info_ptr;
				Packet	*pkptr;
				
				pkptr = op_pk_get(op_intrpt_strm());
				
				
				
				//Sets source and destination
				op_pk_fd_set(pkptr , 0 , OPC_FIELD_TYPE_INTEGER , mac_address , 16);
				op_pk_fd_set(pkptr , 1 , OPC_FIELD_TYPE_INTEGER , pk_destination , 16);
				op_pk_fd_set(pkptr , 2 , OPC_FIELD_TYPE_INTEGER , current_pk_id , 16);
				
				
				//Stats
				info_ptr = (pk_info*) op_prg_mem_alloc(sizeof(pk_info));
				info_ptr->source		= mac_address;
				info_ptr->destination	= pk_destination;
				info_ptr->hops			= 0;
				info_ptr->pk_id			= current_pk_id++;
				info_ptr->received		= OPC_FALSE;
				info_ptr->time_sent		= op_sim_time();
				op_prg_list_insert(pk_list, info_ptr, OPC_LISTPOS_TAIL);
				
				pk_send_to_mac(pkptr, next_hop);
				
				}


			/** state (appl layer arrival) exit executives **/
			FSM_STATE_EXIT_FORCED (2, "appl layer arrival", "wifi_interface_auto () [appl layer arrival exit execs]")
				{
				}


			/** state (appl layer arrival) transition processing **/
			FSM_INIT_COND (ENDSIM)
			FSM_TEST_COND (!ENDSIM)
			FSM_TEST_LOGIC ("appl layer arrival")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 6, state6_enter_exec, ;, "ENDSIM", "", "appl layer arrival", "END_SIM")
				FSM_CASE_TRANSIT (1, 1, state1_enter_exec, ;, "!ENDSIM", "", "appl layer arrival", "idle")
				}
				/*---------------------------------------------------------*/



			/** state (mac layer arrival) enter executives **/
			FSM_STATE_ENTER_FORCED (3, state3_enter_exec, "mac layer arrival", "wifi_interface_auto () [mac layer arrival enter execs]")
				{
				//Src and destination
				int			source , dest;
				int			i;
				//Information about the packet to receive/forward
				pk_info		*info_ptr;
				int			pk_id_tmp;
				//Sim end
				double		next_start_time;
				//Debug
				char		msg[500];
				//Inter_pk time
				double		delivery_ratio;
				int			node_id;
				//tmp vaue
				int			*int_ptr;
				Packet		*pkptr;
				//power
				double		power_ratio;
				int			x_dist , y_dist;
				double		dist;
				
				
				
				
				pkptr = op_pk_get(op_intrpt_strm());
				
				
				
				
				//-----------------------------------
				//			SOURCE / DEST
				//-----------------------------------
				op_pk_fd_get(pkptr , 0 , &source);
				op_pk_fd_get(pkptr , 1 , &dest);
				op_pk_fd_get(pkptr , 2 , &pk_id_tmp);
				
				if (dest != pk_destination){
					sprintf(msg , "%d received a packet to forward to %d, but its own destination is set to %d", mac_address, dest , pk_destination);
					op_sim_end(msg , "It is a bug: the routes to the sink are not uniform" , "" , "");
				}
				
				
				//printf("%d from %d to %d\n", mac_address, source , dest);
				
				
				if (pk_id_tmp < lowest_pk_id_authorized){ // || (is_id_seen(pk_id_tmp))){
					op_pk_destroy(pkptr);
				}
				else{
				
				
					//-----------------------------------
					//				STATS
					//-----------------------------------
					info_ptr = get_pk_info_in_list(pk_list , pk_id_tmp);
					if (info_ptr == NULL)
						op_sim_end("The pk_id is not found in the list of generated packets" , "Do you use one single wlan_mac_intf ?" , "" , "");
					else{
						info_ptr->hops ++;
						if (mac_address == dest){
							info_ptr->received 		= OPC_TRUE;
							info_ptr->time_received = op_sim_time();
						}
					}
				
				
					//-----------------------------------
					//		  THROUGHPUT ADAPTATION
					//-----------------------------------
					//NB: 4th condition to avoid several modifications (two packets are received by the sink, and current_pk_id did not change)
				
					//if (pk_id_tmp > 500)
					//printf("%d %d %d\n", rate_adaptation , my_stat_id == 0 , (last_pk_id_stats + PK_ID_MODULO < pk_id_tmp));
					
					
					if ((pk_destination == mac_address) && (last_pk_id_stats + PK_ID_MODULO < pk_id_tmp)){
						
						//NB: new packets keeps on be transmitted: the load must remain constant in order to have valid measures
				
						if (rate_adaptation){
							//Control (Memory)
							last_pk_id_stats = pk_id_tmp;
						
							//Schedules the rate adaptation procedure (MAX_DELAY accepted)
							if (!rate_adapt_scheduled){
								int_ptr = op_prg_mem_alloc(sizeof(int));
								*int_ptr = pk_id_tmp;
								op_intrpt_schedule_call(op_sim_time() + MAX_DELAY , RATE_ADAPT_CODE , adapt_rate , int_ptr); 
								rate_adapt_scheduled = OPC_TRUE;
								//op_intrpt_schedule_self(op_sim_time() + MAX_DELAY , RATE_ADAPT_CODE + pk_id_tmp);
							}
					
						}
						else{
							//Control (Memory)
							last_pk_id_stats = pk_id_tmp;
						
							//Schedules the rate stop procedure (MAX_DELAY accepted)
							if (!rate_adapt_scheduled){
								int_ptr = op_prg_mem_alloc(sizeof(int));
								*int_ptr = pk_id_tmp;
								op_intrpt_schedule_call(op_sim_time() + MAX_DELAY , RATE_ADAPT_CODE , stop_rate , int_ptr); 
								rate_adapt_scheduled = OPC_TRUE;
							}
						}
					}
				
				
				
					//The stream 0 is directed to the sink (and stream 1 to the MAC layer)
					//In other words if the stream 0 exists, I am a sink !
					if (op_strm_connected(OPC_STRM_OUT , STREAM_TO_UP) == OPC_TRUE)
				
						
						//-----------------------------------
						//		Forward to the upper layer
						//-----------------------------------
						op_pk_send (pkptr, STREAM_TO_UP);
					
					
					else 
						//-----------------------------------
						//		Forward to the next hop
						//-----------------------------------
						pk_send_to_mac(pkptr , next_hop);
				
				
				
				}
				
				}


			/** state (mac layer arrival) exit executives **/
			FSM_STATE_EXIT_FORCED (3, "mac layer arrival", "wifi_interface_auto () [mac layer arrival exit execs]")
				{
				}


			/** state (mac layer arrival) transition processing **/
			FSM_INIT_COND (ENDSIM)
			FSM_TEST_COND (!ENDSIM)
			FSM_TEST_LOGIC ("mac layer arrival")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 6, state6_enter_exec, ;, "ENDSIM", "", "mac layer arrival", "END_SIM")
				FSM_CASE_TRANSIT (1, 1, state1_enter_exec, ;, "!ENDSIM", "", "mac layer arrival", "idle")
				}
				/*---------------------------------------------------------*/



			/** state (wait) enter executives **/
			FSM_STATE_ENTER_UNFORCED (4, state4_enter_exec, "wait", "wifi_interface_auto () [wait enter execs]")
				{
				
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (9,wifi_interface_auto)


			/** state (wait) exit executives **/
			FSM_STATE_EXIT_UNFORCED (4, "wait", "wifi_interface_auto () [wait exit execs]")
				{
				char	msg[500];
				//Inter pk time
				int		node_id;
				int		mac_id;
				//Real coordinates
				double	x , y;
				//Grid coordinates
				int		x_int , y_int ;
				int		x_sink , y_sink;
				double	x_center , y_center;
				int		x_dev , y_dev;
				double	radius , radius2;
				//position parameters
				double	simulation_area;
				int		position_method;
				//Control
				int		i;
				//Routes
				List 	*my_route_tmp;
				int		*int_ptr;
				//Mac process name
				char	mac_name[400];
				
				
				
				
				
				//--------------------------------------------
				//
				//			MAC COHABITATION
				//
				//--------------------------------------------
				
				
				
				
				
				/* Obtain the MAC layer information for the local MAC	*/
				/* process from the model-wide registry.				*/
				proc_record_handle_list_ptr = op_prg_list_create ();
				oms_pr_process_discover (my_objid, proc_record_handle_list_ptr,
					"node objid",	OMSC_PR_OBJID,		my_node_objid,
					"protocol", 	OMSC_PR_STRING,		"mac",
					OPC_NIL);
				 
				/* If the MAC process regostered itself, then there	*/
				/* must be a valid match							*/
				record_handle_list_size = op_prg_list_size (proc_record_handle_list_ptr);
					
				
				
				
				
				//--------------------------------------------
				//
				//			MAC ADDRESS + STREAMS
				//
				//--------------------------------------------
				
				//Address
				mac_id = op_topo_assoc (op_id_self(), OPC_TOPO_ASSOC_OUT, OPC_OBJTYPE_PROC, STREAM_TO_MAC);
				op_ima_obj_attr_get(mac_id , "Address" , 	&mac_address);
				
				//Is this node a sink ? 
				//i.e. there exists a upper process receiving the packets
				if (op_strm_connected(OPC_STRM_OUT , STREAM_TO_UP) == OPC_TRUE)
					is_sink = OPC_TRUE;
				
				
				//--------------------------------------------
				//
				//					POSITION
				//
				//--------------------------------------------
				
				y_int = mac_address /  100;
				x_int = mac_address - y_int * 100;
				
				y_sink = (int)(pk_destination /  100);
				x_sink = (int)(pk_destination - y_sink * 100);
				
				
				//POSITION
				op_ima_sim_attr_get(OPC_IMA_INTEGER, 	"POSITION",	&position_method);
				switch (position_method){
				
					//A grid in which each node is exactly POSITION_PAREMETER far from the previous node (horizontalle and vertically)
					//- Be careful: POSITION_PARAMETER <= RADIo_RANGE
					case GRID_POSITION :
						op_ima_sim_attr_get(OPC_IMA_DOUBLE , "POSITION_PARAMETER" , &grid_side);
						x = (double)x_int * grid_side;
						y = (double)y_int * grid_side;
					
						if (grid_side > radio_range)
							op_sim_end("The grid is mis-configured: ", "nodes are too far from each other", "and the network will surely be disconnected" , "");
					break;
					
					
					//A pseudo grid: 
					//- Virtual nodes are organized as a grid. 
					//- Real nodes are placed randomly around the virtual nodes (in a square centered on the virtual node, and of side POSITION_PARAMETER)
					//- Be careful: POSITION_PARAMETER <= 1/2 * RADIO_RANGE (else the network can be and will surely be disconnected)
					case RANDOM_GRID_POSITION :
						op_ima_sim_attr_get(OPC_IMA_DOUBLE , "POSITION_PARAMETER" , &grid_side);
						
						if (is_sink){
							x = (double)x_int * grid_side;
							y = (double)y_int * grid_side;
						}
						else{
							x = (double)x_int * grid_side + op_dist_uniform(grid_side) - grid_side / 2;
							y = (double)y_int * grid_side + op_dist_uniform(grid_side) - grid_side / 2;
						}
					
						if (grid_side > radio_range / 2)
							op_sim_end("The pseudo-grid is mis-configured: ", "nodes are too far from each other", "and the network will surely be disconnected" , "");
					break;
					
					
					//The original position remains unchanged
					case AS_IS_POSITION :
						op_ima_obj_attr_get(op_id_parent(op_id_self()) , "x position" , &x);
						op_ima_obj_attr_get(op_id_parent(op_id_self()) , "y position" , &y);
					break;
					
					
					//Nodes are placed randomly on the area (a square of side POSITION_PARAMETER)
					case RANDOM_POSITION :
						op_ima_sim_attr_get(OPC_IMA_DOUBLE , "POSITION_PARAMETER" , &simulation_area);
						if (!is_sink){
							x = op_dist_uniform(simulation_area);
							y = op_dist_uniform(simulation_area);
						}
						else{
							x = simulation_area / 2;
							y = simulation_area / 2;
						}
					break;
					default:
						sprintf(msg, "%d unknown" , position_method);
						op_sim_end("Bad position method" , msg , "" , "");
					break;
				}
				
				
				//registers the computed position
				op_ima_obj_attr_set(op_id_parent(op_id_self()) , "x position" , x);
				op_ima_obj_attr_set(op_id_parent(op_id_self()) , "y position" , y);
				
				
				if (x_int > max_x_int)
					max_x_int = x_int;
				if (y_int > max_y_int)
					max_y_int = y_int;
				
				
				
				
				
				//--------------------------------------------
				//
				//					NEXT HOP
				//
				//--------------------------------------------
				
				
				//X and y deviation
				y_dev = (int) (mac_address / 100) - (int) (pk_destination /100);
				x_dev = mac_address - pk_destination - y_dev*100;
				
				
				if ((GRID_POSITION != position_method) && (routing_type != NO_ROUTING))
					op_sim_end("We cannot implement a centralized routing protocol ", "on a random topology" , "We must have a grid" , "");
				
				switch(routing_type){
				
					//----------------------------------------
					//				NO ROUTING
					//----------------------------------------
					case NO_ROUTING :
						next_hop = 0;
					break;
					
				
					
					//----------------------------------------
					//				XY ROUTING
					//----------------------------------------
					case XY_ROUTING :
						next_hop = get_next_hop_via_xy_routing(radio_range / grid_side , pk_destination);
					break;
					
					
					
					
					//----------------------------------------
					//		   	OPTIMAL CORNER ROUTING
					//----------------------------------------
					case OPT_ROUTING :
						radius = (double)beta * radio_range / grid_side / 2;
						x_center = x_sink + beta * radio_range / grid_side / (2 * sqrt(2));
						y_center = y_sink + beta * radio_range / grid_side / (2 * sqrt(2));
						
						//The sink MUST be in the corner
						if (pk_destination != MIN_ADDRESS){
							sprintf(msg , "Min address (%d) != destination (%d)" , MIN_ADDRESS , pk_destination);
						}
						
					
						//Shortest path (in the maximum clique)
						if (get_distance(x_center , y_center , (double)x_int , (double)y_int) <= radius)
							next_hop = get_next_hop_via_shortest_routing(radio_range / grid_side , pk_destination); 
						//SIDES - modified (to avoid the interferences circle)
						else
							next_hop = get_next_hop_via_opt_corner_sides_routing(radio_range / grid_side , pk_destination , x_center , y_center , radius); 
						
					if (mac_address == pk_destination){
						printf("SINK %d %d\n", x_sink , y_sink);
						printf("CENTER : %f %f\n",x_center , y_center);
					}
					
					break;
				
					
					
					
				   	//------------------------------------------------
					//				OPT_CORNER 2
					//------------------------------------------------
					case OPT2_ROUTING :
						radius = (double)beta * (double)radio_range / grid_side;
						x_center = x_sink;
						y_center = y_sink;		
				
						//special path (in the beta-center)
						if (get_distance(x_center , y_center , (double)x_int , (double)y_int) <= (double)radius){
							next_hop = get_next_hop_via_opt2_routing(radio_range / grid_side , pk_destination); 
						}
						//SIDES
						else{
							next_hop = get_next_hop_via_opt_corner_sides_routing(radio_range / grid_side , pk_destination , x_center , y_center , radius); 
						}
						
					break;
				
					
					
					
					
				   	//----------------------------------------
					//		   	SHORTEST ROUTES
					//----------------------------------------
					case SHORT_ROUTING :
						
						next_hop = get_next_hop_via_shortest_routing(radio_range / grid_side , pk_destination); 
					
					break;
						
					
					
					
				   	//------------------------------------------------
					//	SHORTEST ROUTES VIA THE SIDES OF THE SQUARE
					//------------------------------------------------
					case SIDES_ROUTING :
						
						next_hop = get_next_hop_via_sides_routing(radio_range / grid_side , pk_destination); 
					
					break;
					
					
					
				
				   	//------------------------------------------------
					//				'F_alpha' ROUTES
					//------------------------------------------------
					case ALPHA_ROUTING :
						radius = (double)beta * (double)radio_range / grid_side;
						x_center = x_sink;
						y_center = y_sink;		
						
						//Shortest path (in the beta-center)
						if (get_distance(x_center , y_center , (double)x_int , (double)y_int) <= (double)radius){
							//printf("IN  ");
							next_hop = get_next_hop_via_shortest_routing(radio_range / grid_side , pk_destination); 
						}
						//SIDES
						else{
							//printf("OUT  ");
							next_hop = get_next_hop_via_sides_routing(radio_range / grid_side , pk_destination); 
						}
					
					break;
				
					
					
						
					
					
				   	//----------------------------------------
					//		   			ERROR
					//----------------------------------------
					default:
						sprintf(msg , "The routing type %d", routing_type);
						op_sim_end(msg , "is unknown", "Please change it" , "");
					break;
						
						
				}
				
				
				//printf("Next hop %d -> %d\n" , mac_address , next_hop);
				
				
				
				//----------------------------------------
				//		   			BORDER NODE
				//----------------------------------------
				
				if ((x_int == x_sink) || (y_int == y_sink))
					is_border_node = OPC_TRUE;
				else
					is_border_node = OPC_FALSE;
				
				
				//----------------------------------------
				//		   			ERROR
				//----------------------------------------
				
				
				if ((next_hop == mac_address) && (!is_sink) && (routing_type != NO_ROUTING)){
					sprintf(msg, "%d has the next hopd %d (routing algo %d) (%d %d %d)", mac_address , next_hop , routing_type , op_strm_connected(OPC_STRM_OUT , STREAM_TO_UP) , OPC_STRM_OUT , STREAM_TO_UP);
					op_sim_end("Bug in the routing algorithm in the mac-interface process" , msg , "" , "");
				}
				
				
				
				//----------------------------------------
				//		   			ROUTES
				//----------------------------------------
				
				
				//My route : me, next_hop, and must be continued
				my_route_tmp	= op_prg_list_create();
				
				//Size
				int_ptr			= op_prg_mem_alloc( sizeof(int) );
				if (mac_address == next_hop)
					*int_ptr		= 0;
				else
					*int_ptr		= 1;
				op_prg_list_insert(my_route_tmp, int_ptr , OPC_LISTPOS_TAIL);
					
				//Source
				int_ptr			= op_prg_mem_alloc( sizeof(int) );
				*int_ptr		= mac_address;
				op_prg_list_insert(my_route_tmp, int_ptr , OPC_LISTPOS_TAIL);
					
				//next hop
				int_ptr			= op_prg_mem_alloc( sizeof(int) );
				*int_ptr		= next_hop;
				op_prg_list_insert(my_route_tmp, int_ptr , OPC_LISTPOS_TAIL);
				
				
				//Insert the start of my route in the list of all routes
				op_prg_list_insert(all_routes , my_route_tmp , OPC_LISTPOS_TAIL);
				
				
				
				//Synchronization before the next state
				op_intrpt_schedule_self(op_sim_time() , 0);
				
				
				
				
				
				}


			/** state (wait) transition processing **/
			FSM_TRANSIT_FORCE (7, state7_enter_exec, ;, "default", "", "wait", "stats_init")
				/*---------------------------------------------------------*/



			/** state (init2) enter executives **/
			FSM_STATE_ENTER_UNFORCED (5, state5_enter_exec, "init2", "wifi_interface_auto () [init2 enter execs]")
				{
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (11,wifi_interface_auto)


			/** state (init2) exit executives **/
			FSM_STATE_EXIT_UNFORCED (5, "init2", "wifi_interface_auto () [init2 exit execs]")
				{
				/* Schedule a self interrupt to wait for lower layer	*/
				/* Wlan MAC process to finalize the MAC address			*/
				/* registration and resolution.							*/
				op_intrpt_schedule_self (op_sim_time (), 0);
				}


			/** state (init2) transition processing **/
			FSM_TRANSIT_FORCE (4, state4_enter_exec, ;, "default", "", "init2", "wait")
				/*---------------------------------------------------------*/



			/** state (END_SIM) enter executives **/
			FSM_STATE_ENTER_UNFORCED (6, state6_enter_exec, "END_SIM", "wifi_interface_auto () [END_SIM enter execs]")
				{
				FILE* 		pfile;
				char		backoff_type_str[20], routing_type_str[20] , filename[200];
				char		msg[500];
				int			i;
				int			RTS;
				//Stats
				pk_info		*elem;
				int			nb_pk_received = 0;
				int			nb_pk_sent = 0;
				//delay
				double		delay_tmp;
				double		cumulated_delay = 0;
				double		max_delay = 0;
				
				
				if (my_stat_id == 0){
				
				
					//-------------------------------------------
					//				PACKETS
					//-------------------------------------------
				
					for(i=0; i <op_prg_list_size(pk_list) ; i++){
						elem = op_prg_list_access(pk_list, i);
					
						nb_pk_sent ++;
					
						//Received !!
						if (elem->received){
							nb_pk_received ++;
							
							//Delays
							delay_tmp = elem->time_received - elem->time_sent;
							cumulated_delay += delay_tmp;
							if (delay_tmp > max_delay)
								max_delay = delay_tmp;
						}
					}
				
					//All info about the packets in a common file
					if (DEBUG)
						debug_write_pk_info();
				
					
					//-------------------------------------------
					//					WRITING
					//-------------------------------------------
					sprintf(filename , "results_80211/%d-nodes_%d-results.txt", timestamp , nb_nodes);
					pfile = fopen(filename , "w");
				
				
					if (pfile == NULL){
						sprintf(msg, "Filename %s creates an error: %s", filename, strerror(errno));
						printf("ERROR: %d (%s)\n", errno, strerror(errno));
						op_sim_end(msg, "" , "" , "");
					}
						
					print_headers_stat_file(pfile);
				
					fprintf(pfile , "\n\n\n ------------------------------------------------------------------------------------------------------------\n");
					fprintf(pfile , "|                                                  GENERAL                                                   |\n");
					fprintf(pfile , " ------------------------------------------------------------------------------------------------------------\n");
				
					fprintf(pfile , "Nb packets sent							:	%d\n", 			nb_pk_sent);
					fprintf(pfile , "Delivery Ratio							:	%f\n", 				(double)nb_pk_received / nb_pk_sent);
				
				
					fprintf(pfile , "\n\n\n ------------------------------------------------------------------------------------------------------------\n");
					fprintf(pfile , "|                                                 CAPACITY                                                   |\n");
					fprintf(pfile , " ------------------------------------------------------------------------------------------------------------\n");
				
				
					fprintf(pfile , "\n\n------------------------------------------------  CAPACITY ---------------------------------------------------\n");
					fprintf(pfile , "Valid inter packet time						:	%f\n", 		achievable_inter_pk_time);
					fprintf(pfile , "Current inter packet time					:	%f\n", 			current_inter_pk_time);
				
					fprintf(pfile , "\n\n------------------------------------------------  DELAY ---------------------------------------------------\n");
					fprintf(pfile , "Average Delay							:	%f\n", 				cumulated_delay / nb_pk_received);
					fprintf(pfile , "Max Delay 								:	%f\n", 				max_delay);
				
				
					
					
					
					
					//-------------------------------------------
					//		CLOSE
					//-------------------------------------------
				
				
					fclose(pfile);
				
				
				}
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (13,wifi_interface_auto)


			/** state (END_SIM) exit executives **/
			FSM_STATE_EXIT_UNFORCED (6, "END_SIM", "wifi_interface_auto () [END_SIM exit execs]")
				{
				}


			/** state (END_SIM) transition processing **/
			FSM_TRANSIT_MISSING ("END_SIM")
				/*---------------------------------------------------------*/



			/** state (stats_init) enter executives **/
			FSM_STATE_ENTER_UNFORCED (7, state7_enter_exec, "stats_init", "wifi_interface_auto () [stats_init enter execs]")
				{
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (15,wifi_interface_auto)


			/** state (stats_init) exit executives **/
			FSM_STATE_EXIT_UNFORCED (7, "stats_init", "wifi_interface_auto () [stats_init exit execs]")
				{
				//Routes
				List 		*route_tmp = NULL;
				int			*int_ptr = NULL;
				short		my_route_nb = 0;
				List		*my_route_tmp = NULL;
				int			next_hop_to_find = 0;
				Boolean		next_route;
				//control
				int			i;
				int			nb = 0;
				int			MAX_ROUTE_LENGTH = 50;
				
				
				
				//-----------------------------------------------------
				//				END-TO-END 		ROUTES 
				//-----------------------------------------------------
				
				
				if (routing_type != NO_ROUTING){
					//get my route nb
					for(i = 0 ; (my_route_nb == 0) && (i< op_prg_list_size(all_routes)) ; i++){
						//For each route
						route_tmp = op_prg_list_access(all_routes, i);
					
						//Source = second element
						int_ptr = op_prg_list_access(route_tmp, 1);
						if (*int_ptr == mac_address){
							my_route_nb = i;
							my_route_tmp = route_tmp;
						
							//the next hop is the third element
							int_ptr = op_prg_list_access(route_tmp, 2);
							next_hop_to_find = *int_ptr;
						}
					}
				
					if (next_hop_to_find == 0)
						op_sim_end("Error while extracting all the routes in the network" , "From the next hop information" , "" , "");
				
				
					//Complete my route thanks to the next_hops of all the nodes
					while ((next_hop_to_find != pk_destination) && (nb++ < MAX_ROUTE_LENGTH)){
					
						next_route = OPC_FALSE;
						for(i = 0 ; (i< op_prg_list_size(all_routes)) && (!next_route) ; i++){
							//For each route
							route_tmp = op_prg_list_access(all_routes, i);
								
							//Source = second element
							int_ptr = op_prg_list_access(route_tmp, 1);
							//printf("SRC %d, search %d\n", *int_ptr , next_hop_to_find);
						
							if (*int_ptr == next_hop_to_find){
								//Increments the route length
								int_ptr = op_prg_list_access(my_route_tmp, 0);
								*int_ptr = *int_ptr + 1;
						
								//Finds the next hop
								int_ptr = op_prg_list_access(route_tmp, 2);
								next_hop_to_find = *int_ptr;
							
								//adds it in my route
								int_ptr = op_prg_mem_alloc(sizeof(int));
								*int_ptr = next_hop_to_find;
								op_prg_list_insert(my_route_tmp , int_ptr , OPC_LISTPOS_TAIL);
							
								//Searches for the next hop
								next_route = OPC_TRUE;
							}
						
						}
					}
				
					//IN stats var (to be accessed by the mac layer)
					op_prg_list_elems_copy(my_route_tmp , my_route_to_sink);
					//print_route(my_route_to_sink);
				}
				
				
				
				
				}


			/** state (stats_init) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "stats_init", "idle")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (0,wifi_interface_auto)
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
wifi_interface_auto_init (void ** gen_state_pptr)
	{
	int _block_origin = 0;
	static VosT_Address	obtype = OPC_NIL;

	FIN (wifi_interface_auto_init (gen_state_pptr))

	if (obtype == OPC_NIL)
		{
		/* Initialize memory management */
		if (Vos_Catmem_Register ("proc state vars (wifi_interface_auto)",
			sizeof (wifi_interface_auto_state), Vos_Vnop, &obtype) == VOSC_FAILURE)
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
		((wifi_interface_auto_state *)(*gen_state_pptr))->current_block = 0;

		FRET (OPC_COMPCODE_SUCCESS)
		}
	}



void
wifi_interface_auto_diag (void)
	{
	/* No Diagnostic Block */
	}




void
wifi_interface_auto_terminate (void)
	{
	int _block_origin = __LINE__;

	FIN (wifi_interface_auto_terminate (void))

	Vos_Catmem_Dealloc (pr_state_ptr);

	FOUT;
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in wifi_interface_auto_svar function. */
#undef my_objid
#undef my_node_objid
#undef instrm_from_mac_
#undef outstrm_to_mac_
#undef oms_aa_handle
#undef mac_address
#undef wlan_mac_req_iciptr
#undef next_hop
#undef pk_destination
#undef routing_type
#undef radio_range
#undef self_position
#undef my_stat_id
#undef rate_adaptation
#undef beta
#undef my_route_to_sink
#undef is_border_node
#undef mac_backoff_type
#undef id_list
#undef DEBUG
#undef is_sink
#undef grid_side



void
wifi_interface_auto_svar (void * gen_ptr, const char * var_name, char ** var_p_ptr)
	{
	wifi_interface_auto_state		*prs_ptr;

	FIN (wifi_interface_auto_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (char *)OPC_NIL;
		FOUT;
		}
	prs_ptr = (wifi_interface_auto_state *)gen_ptr;

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
	if (strcmp ("instrm_from_mac_" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->instrm_from_mac_);
		FOUT;
		}
	if (strcmp ("outstrm_to_mac_" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->outstrm_to_mac_);
		FOUT;
		}
	if (strcmp ("oms_aa_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->oms_aa_handle);
		FOUT;
		}
	if (strcmp ("mac_address" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->mac_address);
		FOUT;
		}
	if (strcmp ("wlan_mac_req_iciptr" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->wlan_mac_req_iciptr);
		FOUT;
		}
	if (strcmp ("next_hop" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->next_hop);
		FOUT;
		}
	if (strcmp ("pk_destination" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pk_destination);
		FOUT;
		}
	if (strcmp ("routing_type" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->routing_type);
		FOUT;
		}
	if (strcmp ("radio_range" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->radio_range);
		FOUT;
		}
	if (strcmp ("self_position" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->self_position);
		FOUT;
		}
	if (strcmp ("my_stat_id" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_stat_id);
		FOUT;
		}
	if (strcmp ("rate_adaptation" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->rate_adaptation);
		FOUT;
		}
	if (strcmp ("beta" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->beta);
		FOUT;
		}
	if (strcmp ("my_route_to_sink" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->my_route_to_sink);
		FOUT;
		}
	if (strcmp ("is_border_node" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->is_border_node);
		FOUT;
		}
	if (strcmp ("mac_backoff_type" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->mac_backoff_type);
		FOUT;
		}
	if (strcmp ("id_list" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->id_list);
		FOUT;
		}
	if (strcmp ("DEBUG" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->DEBUG);
		FOUT;
		}
	if (strcmp ("is_sink" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->is_sink);
		FOUT;
		}
	if (strcmp ("grid_side" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->grid_side);
		FOUT;
		}
	*var_p_ptr = (char *)OPC_NIL;

	FOUT;
	}

