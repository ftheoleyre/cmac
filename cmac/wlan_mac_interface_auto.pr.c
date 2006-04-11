/* Process model C form file: wlan_mac_interface_auto.pr.c */
/* Portions of this file copyright 1992-2002 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
static const char wlan_mac_interface_auto_pr_c [] = "MIL_3_Tfile_Hdr_ 81A 30A modeler 7 443BA383 443BA383 1 ares-theo-1 ftheoley 0 0 none none 0 0 none 0 0 0 0 0 0                                                                                                                                                                                                                                                                                                                                                                                                                 ";
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

#define 	MAC_LAYER_PKT_ARVL	((intrpt_type == OPC_INTRPT_STRM) && (intrpt_strm == instrm_from_mac))
#define 	APPL_LAYER_PKT_ARVL	((intrpt_type == OPC_INTRPT_STRM) && (intrpt_strm != instrm_from_mac))
#define 	MAC_BROADCAST		-1
#define 	ENDSIM				(intrpt_type == OPC_INTRPT_ENDSIM)
#define 	RATE_ADAPTATION		((intrpt_type == OPC_INTRPT_SELF) && (intrpt_code > RATE_ADAPT_CODE))



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
	int	                    		instrm_from_mac;
	int	                    		outstrm_to_mac;
	OmsT_Aa_Address_Handle	 		oms_aa_handle;
	int	                    		mac_address;
	Ici*	                   		wlan_mac_req_iciptr;
	int	                    		next_hop;
	int	                    		pk_destination;
	int	                    		routing_type;
	int	                    		range;
	int	                    		my_stat_id;
	int	                    		rate_adaptation;
	int	                    		alpha;
	Boolean	                		is_border_node;
	int	                    		mac_backoff_type;
	List*	                  		id_list;
	int	                    		DEBUG_INTF;
	} wlan_mac_interface_auto_state;

#define pr_state_ptr            		((wlan_mac_interface_auto_state*) SimI_Mod_State_Ptr)
#define my_objid                		pr_state_ptr->my_objid
#define my_node_objid           		pr_state_ptr->my_node_objid
#define instrm_from_mac         		pr_state_ptr->instrm_from_mac
#define outstrm_to_mac          		pr_state_ptr->outstrm_to_mac
#define oms_aa_handle           		pr_state_ptr->oms_aa_handle
#define mac_address             		pr_state_ptr->mac_address
#define wlan_mac_req_iciptr     		pr_state_ptr->wlan_mac_req_iciptr
#define next_hop                		pr_state_ptr->next_hop
#define pk_destination          		pr_state_ptr->pk_destination
#define routing_type            		pr_state_ptr->routing_type
#define range                   		pr_state_ptr->range
#define my_stat_id              		pr_state_ptr->my_stat_id
#define rate_adaptation         		pr_state_ptr->rate_adaptation
#define alpha                   		pr_state_ptr->alpha
#define is_border_node          		pr_state_ptr->is_border_node
#define mac_backoff_type        		pr_state_ptr->mac_backoff_type
#define id_list                 		pr_state_ptr->id_list
#define DEBUG_INTF              		pr_state_ptr->DEBUG_INTF

/* This macro definition will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE
#define FIN_PREAMBLE	wlan_mac_interface_auto_state *op_sv_ptr = pr_state_ptr;


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

	/* Stream indices to and from the WLAN MAC process.	*/
	/* these will be set in the "exit execs" of "init".	*/
	outstrm_to_mac  = OPC_INT_UNDEF;
	instrm_from_mac = OPC_INT_UNDEF;

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
		next_hop_tmp = next_hop_tmp - signe (x_dev) * min (fabs(x_dev) , floor(sqrt( pow(range , 2) - pow(y_dev, 2) ))  );				
	}
	else if (fabs(y_dev) < range_tmp){
		
		next_hop_tmp = mac_address - y_dev * 100 ;
		next_hop_tmp = next_hop_tmp - signe (x_dev) * floor(sqrt( pow(range , 2) - pow(y_dev, 2) ));				

	}
	else if (fabs(x_dev) < range_tmp) {
		
		next_hop_tmp = mac_address - x_dev ;
		next_hop_tmp = next_hop_tmp - signe (y_dev) * 100 * floor(sqrt( pow(range , 2) - pow(x_dev, 2) ));				

	}
	//diag
	else{
		if (dev > 0)
			next_hop_tmp = mac_address - signe(x_dev) * dev - signe(y_dev) * dev *100;
		else if (x_dev >= y_dev)
			next_hop_tmp = mac_address - signe(x_dev) * range;
		else if (y_dev > x_dev)
			next_hop_tmp = mac_address - signe(y_dev) * range * 100;
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
	int		dev = range_tmp / sqrt(2);		// Max mobility via diagonale

	y_dev = (int) (mac_address / 100) - (int) (destination_tmp /100);
	x_dev = mac_address - destination_tmp - y_dev*100;

	//printf("addr %d %d %d\n", mac_address , x_dev , y_dev);
	
	//YX Routing
	if (x_dev > y_dev){
		if (y_dev != 0){
			if (fabs(y_dev) < range_tmp){		
			
				next_hop_tmp = mac_address - y_dev * 100 ;
				next_hop_tmp = next_hop_tmp - signe (x_dev) * floor(sqrt( pow(range , 2) - pow(y_dev, 2) ));				

			}
			else
				next_hop_tmp = mac_address - signe(y_dev) * range_tmp * 100;

			next_hop_y_dev = (int) (next_hop_tmp / 100) - (int) (destination_tmp /100);
			next_hop_x_dev = next_hop_tmp - destination_tmp - y_dev*100;
			
			//Next hop in the interference circle and not in one of the authorized axis -> bypass the circle
			while (	(is_in_circle(next_hop_tmp , x_center, y_center , radius)) 
					&& 
					(!is_in_circle(mac_address , x_center, y_center , radius)) 
					&& 
					(next_hop_y_dev != 0)
				){				
				next_hop_tmp += 100 + 1;			
			}
			
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
				next_hop_tmp = next_hop_tmp - 100 * signe (y_dev) * floor(sqrt( pow(range , 2) - pow(x_dev, 2) ));
			}
			else
				next_hop_tmp = mac_address - signe(x_dev) * range_tmp;

			next_hop_y_dev = (int) (next_hop_tmp / 100) - (int) (destination_tmp /100);
			next_hop_x_dev = next_hop_tmp - destination_tmp - y_dev*100;
			
			//Next hop in the interference circle and not in one of the authorized axis -> bypass the circle
			while (	(is_in_circle(next_hop_tmp , x_center, y_center , radius)) 
					&& 
					(!is_in_circle(mac_address , x_center, y_center , radius)) 
					&& 
					(next_hop_x_dev != 0)
				){				
				next_hop_tmp += 100 + 1;
			}

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
int get_next_hop_via_sides_short_bis_routing(int range_tmp , int destination_tmp){
	int 	next_hop_tmp;
	int		x_dev , y_dev;
	
	y_dev = (int) (mac_address / 100) - (int) (destination_tmp /100);
	x_dev = mac_address - destination_tmp - y_dev*100;
		
	//YX Routing
	if (fabs(x_dev) > fabs(y_dev)){
		if (y_dev != 0){
			if (fabs(y_dev) < range_tmp){		
			
				next_hop_tmp = mac_address - y_dev * 100 ;
				next_hop_tmp = next_hop_tmp - floor(sqrt( pow(range , 2) - pow(y_dev, 2) ));				

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
				next_hop_tmp = next_hop_tmp - 100 * floor(sqrt( pow(range , 2) - pow(x_dev, 2) ));

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
		return((double)0.0);
	
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
	int		CTR;
	int		BLOCKED_MODE;
	int		RTS;
	double	PRIVILEGED_MAX_TIME;
	double	GRID_RANGE;
	int		BUSY_TONE_ACTIVATED;

	
	
	
	//-------------------------------------------
	//				PARAMETERS
	//-------------------------------------------
	
	op_ima_sim_attr_get(OPC_INTEGER , 		"CTR" , 					&CTR);
	op_ima_sim_attr_get(OPC_INTEGER , 		"BLOCKED_MODE" , 			&BLOCKED_MODE);
	op_ima_sim_attr_get(OPC_DOUBLE , 		"PRIVILEGED_MAX_TIME" , 	&PRIVILEGED_MAX_TIME);
	op_ima_sim_attr_get(OPC_IMA_INTEGER , 	"RTS" , 					&RTS);
	op_ima_sim_attr_get(OPC_IMA_DOUBLE, 	"GRID_RANGE",				&GRID_RANGE);
	op_ima_sim_attr_get(OPC_IMA_INTEGER, 	"BUSY_TONE_ACTIVATED",		&BUSY_TONE_ACTIVATED);
	
	BUSY_TONE_ACTIVATED = BUSY_TONE_ACTIVATED && RTS;


	fprintf(pfile , " ------------------------------------------------------------------------------------------------------------\n");
	fprintf(pfile , "|                                           SIMULATION PARAMETERS                                            |\n");
	fprintf(pfile , " ------------------------------------------------------------------------------------------------------------\n");

	fprintf(pfile , "------------------------------------------------  GENERAL ---------------------------------------------------\n");
	fprintf(pfile , "Number of nodes							:	%d\n", 			nb_nodes);
	fprintf(pfile , "Radio Range								:	%d\n", 			range);
	fprintf(pfile , "Grid Range								:	%f\n", 				GRID_RANGE);
	fprintf(pfile , "Duration								:	%f\n", 				op_sim_time());
	fprintf(pfile , "RTS									:	%d\n", 				RTS);
	fprintf(pfile , "Busy Tone								:	%d\n", 				BUSY_TONE_ACTIVATED);
	fprintf(pfile , "CTR									:	%d\n", 				CTR);
	fprintf(pfile , "Blocked Mode							:	%d\n", 				BLOCKED_MODE);
	fprintf(pfile , "Privileged Max Time						:	%f\n", 			PRIVILEGED_MAX_TIME);
	fprintf(pfile , "Inter Packet Time							:	%f\n", 			current_inter_pk_time);
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

	printf(" %f - %d %d\n", delivery_ratio , low_pk_id + PK_ID_MODULO / 10 , high_pk_id);	
	
	
	
	
	//-------------------------------------------
	//					WRITING
	//-------------------------------------------
	sprintf(filename , "results_80211/%d-nodes_%d-range_%d-stats.txt", timestamp , nb_nodes , range);
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
	printf("NEW RATE : %f (delivery ratio %f) (best %f) (worst %f) (%f)\n", current_inter_pk_time , delivery_ratio, achievable_inter_pk_time , bad_inter_pk_time , (achievable_inter_pk_time - bad_inter_pk_time) / 2);
	
	

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
	void wlan_mac_interface_auto (void);
	Compcode wlan_mac_interface_auto_init (void **);
	void wlan_mac_interface_auto_diag (void);
	void wlan_mac_interface_auto_terminate (void);
	void wlan_mac_interface_auto_svar (void *, const char *, char **);
#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
wlan_mac_interface_auto (void)
	{
	int _block_origin = 0;
	FIN (wlan_mac_interface_auto ());
	if (1)
		{
		List*				proc_record_handle_list_ptr;
		int					record_handle_list_size;
		OmsT_Pr_Handle		process_record_handle;
		Objid				mac_module_objid;
		Boolean				dest_addr_okay = OPC_FALSE;
		double				ne_address = OPC_DBL_UNDEF;
		int					curr_dest_addr = OMSC_AA_AUTO_ASSIGN;
		Packet*				pkptr;
		int					intrpt_type = OPC_INT_UNDEF;
		int					intrpt_strm = OPC_INT_UNDEF;
		int					intrpt_code = OPC_INT_UNDEF;
		OmsT_Aa_Address_Info * ith_address_info_ptr;


		FSM_ENTER (wlan_mac_interface_auto)

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (init) enter executives **/
			FSM_STATE_ENTER_UNFORCED_NOLABEL (0, "init", "wlan_mac_interface_auto () [init enter execs]")
				{
				//Addresses
				char		str[500];
				int			my_address;
				//Control
				int			i;
				//Topology
				int			node_id;
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
				sprintf(mac_name, "%sAddress", MAC_PROCESS_NAME);
				op_ima_obj_attr_set(op_topo_parent(op_id_self()), mac_name , my_address);
				
				
				
				
				
				//--------------------------------------------
				//
				//					PARAMETERS
				//
				//--------------------------------------------
				
				
				op_ima_sim_attr_get(OPC_IMA_INTEGER, 	"RADIO_RANGE" , 					&range);
				op_ima_obj_attr_get(op_id_self(), 		"Destination" , 					&pk_destination);
				op_ima_sim_attr_get(OPC_IMA_INTEGER , 	"RATE_ADAPTATION" , 				&rate_adaptation);
				op_ima_sim_attr_get(OPC_IMA_INTEGER , 	"DEBUG" , 							&DEBUG_INTF);
				
				
				if (current_inter_pk_time == 0)
					op_ima_sim_attr_get(OPC_IMA_DOUBLE , 	"INITIAL_INTER_PK_TIME" , 		&current_inter_pk_time);
				
				
				
				//----------------------------------------------------
				//
				//						STATS
				//
				//-----------------------------------------------------
				
				
				if (pk_list == NULL)
					pk_list = op_prg_list_create();
				
				id_list = op_prg_list_create();
				
				
				
				
				//----------------------------------------------------
				//
				//			   INTER PACKET TIME
				//
				//-----------------------------------------------------
				
				
				
				
				//Sets this value for all the nodes (even if we do not adapt the rate: the packet generation must start after TIME_START_PK_GENERATION)
				if (OPC_TRUE){//rate_adaptation){
					for(i=0 ; i < op_topo_object_count(OPC_OBJTYPE_NDMOB) ; i++){
				  	
						node_id = op_topo_object(OPC_OBJTYPE_NDMOB , i);
						if (op_ima_obj_attr_exists(node_id , "source.Packet Interarrival Arg1"))
							op_ima_obj_attr_set(node_id , "source.Packet Interarrival Arg1" , 	current_inter_pk_time);
						
						if (op_ima_obj_attr_exists(node_id , "source.Start Time Packet Generation"))
							op_ima_obj_attr_set(node_id , "source.Start Time Packet Generation" , TIME_START_PK_GENERATION);
					}
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
			FSM_EXIT (1,wlan_mac_interface_auto)


			/** state (init) exit executives **/
			FSM_STATE_EXIT_UNFORCED (0, "init", "wlan_mac_interface_auto () [init exit execs]")
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
			FSM_STATE_ENTER_UNFORCED (1, state1_enter_exec, "idle", "wlan_mac_interface_auto () [idle enter execs]")
				{
				
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,wlan_mac_interface_auto)


			/** state (idle) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "idle", "wlan_mac_interface_auto () [idle exit execs]")
				{
				/* The only interrupt expected in this state is a	*/
				/* stream interrupt. It can be either from the MAC	*/
				/* layer for a packet destined for this node or		*/
				/* from the application layer for a packet destined	*/
				/* for some other node.								*/
				intrpt_type = op_intrpt_type ();
				
				if (intrpt_type == OPC_INTRPT_STRM){
					intrpt_strm = op_intrpt_strm ();
					pkptr = op_pk_get (intrpt_strm);
				}
				
				if (intrpt_type == OPC_INTRPT_SELF){
					intrpt_code = op_intrpt_code ();
				}
				
				//printf("%d %d\n", intrpt_type , OPC_INTRPT_STRM);
				
				}


			/** state (idle) transition processing **/
			FSM_INIT_COND (APPL_LAYER_PKT_ARVL)
			FSM_TEST_COND (MAC_LAYER_PKT_ARVL)
			FSM_TEST_COND (ENDSIM)
			FSM_TEST_LOGIC ("idle")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 2, state2_enter_exec, ;, "APPL_LAYER_PKT_ARVL", "", "idle", "appl layer arrival")
				FSM_CASE_TRANSIT (1, 3, state3_enter_exec, ;, "MAC_LAYER_PKT_ARVL", "", "idle", "mac layer arrival")
				FSM_CASE_TRANSIT (2, 6, state6_enter_exec, ;, "ENDSIM", "", "idle", "END_SIM")
				}
				/*---------------------------------------------------------*/



			/** state (appl layer arrival) enter executives **/
			FSM_STATE_ENTER_FORCED (2, state2_enter_exec, "appl layer arrival", "wlan_mac_interface_auto () [appl layer arrival enter execs]")
				{
				
				
				pk_info	*info_ptr;
				
				
				//Sets source and destination
				op_pk_fd_set(pkptr , 0 , OPC_FIELD_TYPE_INTEGER , mac_address , 16);
				op_pk_fd_set(pkptr , 1 , OPC_FIELD_TYPE_INTEGER , pk_destination , 16);
				op_pk_fd_set(pkptr , 2 , OPC_FIELD_TYPE_INTEGER , current_pk_id , 16);
				
				//Next hop to the mac layer
				curr_dest_addr = next_hop;
				
				//Stats
				info_ptr = (pk_info*) op_prg_mem_alloc(sizeof(pk_info));
				info_ptr->source		= mac_address;
				info_ptr->destination	= pk_destination;
				info_ptr->hops			= 0;
				info_ptr->pk_id			= current_pk_id++;
				info_ptr->received		= OPC_FALSE;
				info_ptr->time_sent		= op_sim_time();
				op_prg_list_insert(pk_list, info_ptr, OPC_LISTPOS_TAIL);
				
				
				
				
				/* Set this information in the interface control	*/
				/* information to be sent to the MAC layer.			*/
				op_ici_attr_set (wlan_mac_req_iciptr, "dest_addr", curr_dest_addr);
				
				/* Install the control informationand send it to	*/
				/* the MAC layer.									*/
				op_ici_install (wlan_mac_req_iciptr);
				op_pk_send (pkptr, outstrm_to_mac);
				
				
				
				
				
				}


			/** state (appl layer arrival) exit executives **/
			FSM_STATE_EXIT_FORCED (2, "appl layer arrival", "wlan_mac_interface_auto () [appl layer arrival exit execs]")
				{
				}


			/** state (appl layer arrival) transition processing **/
			FSM_INIT_COND (!ENDSIM)
			FSM_TEST_COND (ENDSIM)
			FSM_TEST_LOGIC ("appl layer arrival")

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 1, state1_enter_exec, ;, "!ENDSIM", "", "appl layer arrival", "idle")
				FSM_CASE_TRANSIT (1, 6, state6_enter_exec, ;, "ENDSIM", "", "appl layer arrival", "END_SIM")
				}
				/*---------------------------------------------------------*/



			/** state (mac layer arrival) enter executives **/
			FSM_STATE_ENTER_FORCED (3, state3_enter_exec, "mac layer arrival", "wlan_mac_interface_auto () [mac layer arrival enter execs]")
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
				
					//if ((pk_destination == mac_address) && (last_pk_id_stats + PK_ID_MODULO < pk_id_tmp)){
					if (last_pk_id_stats + PK_ID_MODULO < pk_id_tmp){
						
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
				
							//Stop temporay any transmission
							//printf("CUT !!!!!!!!!\n");
							//update_packet_arrival_in_source_process(current_inter_pk_time , op_sim_time() + MAX_DELAY + 1.0);
						}
					}
				
				
				
					//The stream 0 is directed to the sink (and stream 1 to the MAC layer)
					//In other words if the stream 0 exists, I am a sink !
					if (op_strm_connected(OPC_STRM_OUT , 0) == OPC_TRUE){
				
						
						//-----------------------------------
						//		Forward to the upper layer
						//-----------------------------------
						op_pk_send (pkptr, 0);
					
					
					}
					else {
						//-----------------------------------
						//		Forward to the next hop
						//-----------------------------------
				
				
						//sets the next hop address
						curr_dest_addr = next_hop;
				
					
						//ICI + Transmission to the MAC layer
						op_ici_attr_set (wlan_mac_req_iciptr, "dest_addr", curr_dest_addr);
						op_ici_install (wlan_mac_req_iciptr);
						op_pk_send (pkptr, outstrm_to_mac);
						
					}
				
				}
				
				}


			/** state (mac layer arrival) exit executives **/
			FSM_STATE_EXIT_FORCED (3, "mac layer arrival", "wlan_mac_interface_auto () [mac layer arrival exit execs]")
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
			FSM_STATE_ENTER_UNFORCED (4, state4_enter_exec, "wait", "wlan_mac_interface_auto () [wait enter execs]")
				{
				
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (9,wlan_mac_interface_auto)


			/** state (wait) exit executives **/
			FSM_STATE_EXIT_UNFORCED (4, "wait", "wlan_mac_interface_auto () [wait exit execs]")
				{
				char	msg[500];
				//Inter pk time
				int		node_id;
				//Real coordinates
				double	x , y;
				//Grid coordinates
				int		x_int , y_int ;
				int		x_sink , y_sink;
				double	x_center , y_center;
				int		x_dev , y_dev;
				double	radius;
				//Control
				int		i;
				//Routes
				List 	*my_route_tmp;
				int		*int_ptr;
				//Mac process name
				char	mac_name[400];
				//The range for the grid (-1 if disabled)
				double	POSITION_PARAMETER;
				int		POSITION;
				int		is_sink;
				int		mac_process_id;
				
				
				
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
				
				//Static
				if (record_handle_list_size !=  1)
					{
						//Streams
						instrm_from_mac = STREAM_FROM_MAC;
						outstrm_to_mac 	= STREAM_TO_MAC;
				
						//Address
						sprintf(mac_name, "%sAddress", MAC_PROCESS_NAME);
						op_ima_obj_attr_get(op_topo_parent(op_id_self()), mac_name , &mac_address);
					}
				//Or dynamic
				else
					{
					//Handle process
					process_record_handle = (OmsT_Pr_Handle) op_prg_list_access (proc_record_handle_list_ptr, 0);
				 
					// Module objid
					oms_pr_attr_get (process_record_handle, "module objid", OMSC_PR_OBJID, &mac_module_objid);
				 
					//Streams nb
					oms_tan_neighbor_streams_find (my_objid, mac_module_objid, &instrm_from_mac, &outstrm_to_mac);
				 
					// Address
					oms_pr_attr_get (process_record_handle, "address",             OMSC_PR_NUMBER,  &ne_address);
					oms_pr_attr_get (process_record_handle, "auto address handle", OMSC_PR_ADDRESS, &oms_aa_handle);
				 
					//new mac						*/
					mac_address = (int) ne_address;
					}
				
				
				
				
				
				
				
				//--------------------------------------------
				//
				//					POSITION
				//
				//--------------------------------------------
				
				op_ima_sim_attr_get(OPC_IMA_INTEGER,"POSITION",				&POSITION);
				op_ima_sim_attr_get(OPC_IMA_DOUBLE, "POSITION_PARAMETER", 	&POSITION_PARAMETER);
				
				mac_process_id = op_topo_assoc(op_id_self() , OPC_TOPO_ASSOC_OUT , OPC_OBJTYPE_PROC , STREAM_TO_MAC);
				op_ima_obj_attr_get(mac_process_id , "Is Sink" , &is_sink);
				
				
				switch(POSITION){
				
					//No automatic position
					case 0:
					
					break;
					
					//GRID
					case 1:
				
						y_int = mac_address /  100;
						x_int = mac_address - y_int * 100;
				
						y_sink = (int)(pk_destination /  100);
						x_sink = (int)(pk_destination - y_sink * 100);
				
						x = (double)x_int * POSITION_PARAMETER / (double)range;
						y = (double)y_int * POSITION_PARAMETER / (double)range;
				
						op_ima_obj_attr_set(op_id_parent(op_id_self()) , "x position" , x);
						op_ima_obj_attr_set(op_id_parent(op_id_self()) , "y position" , y);
					break;
					
					//RANDOM
					case 2:
						if (!is_sink){
							x = op_dist_uniform(POSITION_PARAMETER);
							y = op_dist_uniform(POSITION_PARAMETER);
						}
						else{
							x = POSITION_PARAMETER / 2;
							y = POSITION_PARAMETER / 2;
						}
						op_ima_obj_attr_set(op_id_parent(op_id_self()) , "x position" , x);
						op_ima_obj_attr_set(op_id_parent(op_id_self()) , "y position" , y);
					break;
				}
				
				
				
				
				//Synchronization before the next state
				op_intrpt_schedule_self(op_sim_time() , 0);
				
				
				
				}


			/** state (wait) transition processing **/
			FSM_TRANSIT_FORCE (7, state7_enter_exec, ;, "default", "", "wait", "stats_init")
				/*---------------------------------------------------------*/



			/** state (init2) enter executives **/
			FSM_STATE_ENTER_UNFORCED (5, state5_enter_exec, "init2", "wlan_mac_interface_auto () [init2 enter execs]")
				{
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (11,wlan_mac_interface_auto)


			/** state (init2) exit executives **/
			FSM_STATE_EXIT_UNFORCED (5, "init2", "wlan_mac_interface_auto () [init2 exit execs]")
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
			FSM_STATE_ENTER_UNFORCED (6, state6_enter_exec, "END_SIM", "wlan_mac_interface_auto () [END_SIM enter execs]")
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
				//Simulation parameters
				int			CTR;
				int			BLOCKED_MODE;
				double		PRIVILEGED_MAX_TIME;
				
				
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
					if (DEBUG_INTF)
						debug_write_pk_info();
				
					
					//-------------------------------------------
					//					WRITING
					//-------------------------------------------
					sprintf(filename , "results_80211/%d-nodes_%d-range_%d-results.txt", timestamp , nb_nodes , range);
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
			FSM_EXIT (13,wlan_mac_interface_auto)


			/** state (END_SIM) exit executives **/
			FSM_STATE_EXIT_UNFORCED (6, "END_SIM", "wlan_mac_interface_auto () [END_SIM exit execs]")
				{
				}


			/** state (END_SIM) transition processing **/
			FSM_TRANSIT_MISSING ("END_SIM")
				/*---------------------------------------------------------*/



			/** state (stats_init) enter executives **/
			FSM_STATE_ENTER_UNFORCED (7, state7_enter_exec, "stats_init", "wlan_mac_interface_auto () [stats_init enter execs]")
				{
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (15,wlan_mac_interface_auto)


			/** state (stats_init) exit executives **/
			FSM_STATE_EXIT_UNFORCED (7, "stats_init", "wlan_mac_interface_auto () [stats_init exit execs]")
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
				
				
				
				
				//-----------------------------------------------------
				//				END-TO-END 		ROUTES 
				//-----------------------------------------------------
				/*
				
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
				while (next_hop_to_find != pk_destination){
					
				
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
				
				*/
				}


			/** state (stats_init) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "stats_init", "idle")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (0,wlan_mac_interface_auto)
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
wlan_mac_interface_auto_init (void ** gen_state_pptr)
	{
	int _block_origin = 0;
	static VosT_Address	obtype = OPC_NIL;

	FIN (wlan_mac_interface_auto_init (gen_state_pptr))

	if (obtype == OPC_NIL)
		{
		/* Initialize memory management */
		if (Vos_Catmem_Register ("proc state vars (wlan_mac_interface_auto)",
			sizeof (wlan_mac_interface_auto_state), Vos_Vnop, &obtype) == VOSC_FAILURE)
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
		((wlan_mac_interface_auto_state *)(*gen_state_pptr))->current_block = 0;

		FRET (OPC_COMPCODE_SUCCESS)
		}
	}



void
wlan_mac_interface_auto_diag (void)
	{
	/* No Diagnostic Block */
	}




void
wlan_mac_interface_auto_terminate (void)
	{
	int _block_origin = __LINE__;

	FIN (wlan_mac_interface_auto_terminate (void))

	Vos_Catmem_Dealloc (pr_state_ptr);

	FOUT;
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in wlan_mac_interface_auto_svar function. */
#undef my_objid
#undef my_node_objid
#undef instrm_from_mac
#undef outstrm_to_mac
#undef oms_aa_handle
#undef mac_address
#undef wlan_mac_req_iciptr
#undef next_hop
#undef pk_destination
#undef routing_type
#undef range
#undef my_stat_id
#undef rate_adaptation
#undef alpha
#undef is_border_node
#undef mac_backoff_type
#undef id_list
#undef DEBUG_INTF



void
wlan_mac_interface_auto_svar (void * gen_ptr, const char * var_name, char ** var_p_ptr)
	{
	wlan_mac_interface_auto_state		*prs_ptr;

	FIN (wlan_mac_interface_auto_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (char *)OPC_NIL;
		FOUT;
		}
	prs_ptr = (wlan_mac_interface_auto_state *)gen_ptr;

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
	if (strcmp ("instrm_from_mac" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->instrm_from_mac);
		FOUT;
		}
	if (strcmp ("outstrm_to_mac" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->outstrm_to_mac);
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
	if (strcmp ("range" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->range);
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
	if (strcmp ("alpha" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->alpha);
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
	if (strcmp ("DEBUG_INTF" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->DEBUG_INTF);
		FOUT;
		}
	*var_p_ptr = (char *)OPC_NIL;

	FOUT;
	}

