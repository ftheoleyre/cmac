/* Process model C form file: wifi_interface_auto.pr.c */
/* Portions of this file copyright 1992-2007 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
const char wifi_interface_auto_pr_c [] = "MIL_3_Tfile_Hdr_ 120A 30A opnet 7 4A5DDDD8 4A5DDDD8 1 lefkada-laptop theoleyr 0 0 none none 0 0 none 0 0 0 0 0 0 0 0 11ab 5                                                                                                                                                                                                                                                                                                                                                                                                    ";
#include <string.h>



/* OPNET system definitions */
#include <opnet.h>



/* Header Block */

/***** Include Files. *****/
#include <stdlib.h>

#include <math.h>
#include <errno.h>


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
#define	BIN		FIN_LOCAL_FIELD(_op_last_line_passed) = __LINE__ - _op_block_origin;
#define	BOUT	BIN
#define	BINIT	FIN_LOCAL_FIELD(_op_last_line_passed) = 0; _op_block_origin = __LINE__;
#else
#define	BINIT
#endif /* #if !defined (VOSD_NO_FIN) */



/* State variable definitions */
typedef struct
	{
	/* Internal state tracking for FSM */
	FSM_SYS_STATE
	/* State Variables */
	Objid	                  		my_objid                                        ;	/* Object identifier of the surrounding module.	 */
	Objid	                  		my_node_objid                                   ;	/* Object identifier of the surrounding node.	 */
	int	                    		instrm_from_mac_                                ;	/* Stream index of the packet stream coming from MAC. */
	int	                    		outstrm_to_mac_                                 ;	/* Stream index of the packet stream going to MAC. */
	OmsT_Aa_Address_Handle	 		oms_aa_handle                                   ;	/* Auto-address assignment handle. Used while      */
	                        		                                                	/* auto-address and destination address selection. */
	int	                    		mac_address                                     ;	/* Element address of the associated MAC. */
	Ici*	                   		wlan_mac_req_iciptr                             ;	/* Interface control information needed to indicate */
	                        		                                                	/* to the MAC of the destination to which packet    */
	                        		                                                	/* needs to be sent.                                */
	int	                    		next_hop                                        ;	/* The next hop (it depends on the routing_type) */
	int	                    		pk_destination                                  ;	/* The destination for all generated packets */
	int	                    		routing_type                                    ;	/* The type of routing (XY, OPT_CORNER, SIDES...) */
	int	                    		range                                           ;	/* The radio range (in multiple of the sensing range, dimension of the grid) */
	int	                    		self_position                                   ;	/* The nodes must be positionned by the process to form a regular grid */
	int	                    		my_stat_id                                      ;	/* Control variable to write stats */
	int	                    		rate_adaptation                                 ;	/* Must the rate be adapted to comput dynamically its limit ? */
	int	                    		beta                                            ;	/* Depends on the parameters :                     */
	                        		                                                	/* -Radio interference range if OPT / OPT2 routing */
	                        		                                                	/* -Parameter of the Alpha-routing                 */
	List*	                  		my_route_to_sink                                ;	/* The route to the sink (hops, source, .... , sink) */
	Boolean	                		is_border_node                                  ;	/* Is this node a border node ? */
	int	                    		mac_backoff_type                                ;	/* The type of backoff which is used by the mac layer */
	List*	                  		id_list                                         ;	/* List of ids */
	int	                    		DEBUG                                           ;	/* debug level */
	int	                    		is_sink                                         ;	/* Is this node a sink ? */
	} wifi_interface_auto_state;

#define my_objid                		op_sv_ptr->my_objid
#define my_node_objid           		op_sv_ptr->my_node_objid
#define instrm_from_mac_        		op_sv_ptr->instrm_from_mac_
#define outstrm_to_mac_         		op_sv_ptr->outstrm_to_mac_
#define oms_aa_handle           		op_sv_ptr->oms_aa_handle
#define mac_address             		op_sv_ptr->mac_address
#define wlan_mac_req_iciptr     		op_sv_ptr->wlan_mac_req_iciptr
#define next_hop                		op_sv_ptr->next_hop
#define pk_destination          		op_sv_ptr->pk_destination
#define routing_type            		op_sv_ptr->routing_type
#define range                   		op_sv_ptr->range
#define self_position           		op_sv_ptr->self_position
#define my_stat_id              		op_sv_ptr->my_stat_id
#define rate_adaptation         		op_sv_ptr->rate_adaptation
#define beta                    		op_sv_ptr->beta
#define my_route_to_sink        		op_sv_ptr->my_route_to_sink
#define is_border_node          		op_sv_ptr->is_border_node
#define mac_backoff_type        		op_sv_ptr->mac_backoff_type
#define id_list                 		op_sv_ptr->id_list
#define DEBUG                   		op_sv_ptr->DEBUG
#define is_sink                 		op_sv_ptr->is_sink

/* These macro definitions will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE
#define FIN_PREAMBLE_DEC	wifi_interface_auto_state *op_sv_ptr;
#define FIN_PREAMBLE_CODE	\
		op_sv_ptr = ((wifi_interface_auto_state *)(OP_SIM_CONTEXT_PTR->_op_mod_state_ptr));


/* Function Block */

#if !defined (VOSD_NO_FIN)
enum { _op_block_origin = __LINE__ + 2};
#endif

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
//					TOOLS
//
//-----------------------------------------------------------

//return the minimum value
double min(double a, double b){
	FIN(min(double a, double b));
	
	if (a<b)
		FRET(a);
	FRET(b);
}



//-----------------------------------------------------------
//
//					DUPLICATA DETECTION
//
//-----------------------------------------------------------


//Deletes the frame_id which became obsolete
void del_timeouted_id(void* tot , int code){
	FIN(del_timeouted_id(void* tot , int code));
	
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
	FOUT;
}


//Is thie frame_id already in the list ?
Boolean is_id_seen(int pk_id){
	FIN(is_id_seen(int pk_id));

	id_struct		*ptr;
	int				i;
	
	for(i=0 ; i < op_prg_list_size(id_list) ; i++){
		ptr = op_prg_list_access(id_list , i);
		
		if (ptr->id == pk_id)
			FRET(OPC_TRUE);
	}
	
	FRET(OPC_FALSE);
}



//adds the frame_id as already seen
void add_id_seen(int pk_id){
	FIN(add_id_seen(int pk_id));
		
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
		
	FOUT;
}









//-----------------------------------------------------------
//
//							TOOLS
//
//-----------------------------------------------------------


//is this int positive or negative ?
short signe(int value){
	FIN(signe(int value));
	
	if (value < 0)
		FRET(-1);
	if (value > 0)
		FRET(1);
	FRET(0);
}


//returns the euclidyan distance
double get_distance(double x1 , double y1 ,double x2 , double y2){
	FIN(get_distance(double x1 , double y1 ,double x2 , double y2));
	FRET(sqrt (  pow(x1 - x2, 2) + pow(y1 - y2, 2)));
}	


//Is the node in the circle of radius r and center x/y ?
short is_in_circle(int address , double x , double y , double radius){
	FIN(is_in_circle(int address , double x , double y , double radius));
	
	int		x_addr , y_addr;

	y_addr = address /  100;
	x_addr = address - y_addr * 100;

	if (get_distance(x_addr , y_addr, x , y) < radius)
		FRET(OPC_TRUE);
	FRET(OPC_FALSE);
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
	FIN(get_next_hop_via_shortest_routing(int range_tmp , int destination_tmp));
	
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
		
	
	FRET(next_hop_tmp);
}





//-----------------------------------------------------------
//				  	OPT-CORNER  -  ROUTING
//-----------------------------------------------------------
//returns the next hop for the optimal routing scheme, via the axis (but avoiding the interferences circle)
int get_next_hop_via_opt_corner_sides_routing(int range_tmp , int destination_tmp , double x_center , double y_center , double radius){
	FIN(get_next_hop_via_opt_corner_sides_routing(int range_tmp , int destination_tmp , double x_center , double y_center , double radius));
	
	int 	next_hop_tmp;
	int		x_dev , y_dev , next_hop_x_dev , next_hop_y_dev;
	int		next_hop_tmp_sav;
//	int		dev = range_tmp / sqrt(2);		// Max mobility via diagonale

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
				next_hop_tmp = next_hop_tmp - 100 * signe (y_dev) * floor(sqrt( pow(range , 2) - pow(x_dev, 2) ));
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
		
	FRET(next_hop_tmp);	
}



 
//-----------------------------------------------------------
//						XY - ROUTING
//-----------------------------------------------------------
//returns the next hop via the XY routing toward the destination (Along Y and then along X)
int get_next_hop_via_xy_routing(int range_tmp , int destination_tmp){
	FIN(get_next_hop_via_xy_routing(int range_tmp , int destination_tmp));
	
	int 	next_hop_tmp;
	int		x_dev , y_dev;
	char	msg[200];
//	int		dev = range_tmp / sqrt(2);		// Max mobility via diagonale
	
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
	FRET(next_hop_tmp);
}




//-----------------------------------------------------------
//						SIDES - ROUTING
//-----------------------------------------------------------
//returns the next hop via a routing via axis centered on the destination
int get_next_hop_via_sides_routing(int range_tmp , int destination_tmp){
	FIN(get_next_hop_via_sides_routing(int range_tmp , int destination_tmp));
	
	int 	next_hop_tmp;
	int		x_dev , y_dev;
//	int		dev = range_tmp / sqrt(2);		// Max mobility via diagonale
	
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

	FRET(next_hop_tmp);
}



//-----------------------------------------------------------
//		SIDES 2 - ROUTING (combination sides + short)
//-----------------------------------------------------------
//returns the next hop via a routing via axis centered on the destination
int get_next_hop_via_sides_short_bis_routing(int range_tmp , int destination_tmp , double x_center , double y_center , double radius){
	FIN(get_next_hop_via_sides_short_bis_routing(int range_tmp , int destination_tmp , double x_center , double y_center , double radius));
	
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

	FRET(next_hop_tmp);
}




//-----------------------------------------------------------
//						OPT2 ROUTING
//-----------------------------------------------------------

//a quite particular route
int get_next_hop_via_opt2_routing(int range_tmp , int destination_tmp){
	FIN(get_next_hop_via_opt2_routing(int range_tmp , int destination_tmp));
	
	int 	next_hop_tmp = -1;
	int		x_dev , y_dev;
	int		i;
	
	//the Sink
	if (destination_tmp == mac_address)
		FRET(mac_address);
	
	
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
	
	FRET(next_hop_tmp);
}














//-----------------------------------------------------------
//
//							STATISTICS
//
//-----------------------------------------------------------

//returns the struct pk_info associated to pk_id 
pk_info* get_pk_info_in_list(List* ll, int pk_id){
	FIN(get_pk_info_in_list(List* ll, int pk_id));
	
	pk_info		*elem;
	int			i;
	
	for(i=0; i<op_prg_list_size(ll); i++){
		elem = op_prg_list_access(ll , i);
		if (elem->pk_id == pk_id){
			FRET(elem);
		}
	}
	FRET(NULL);
}



//returns the delivery ratio for the packets between id1 and id2
double compute_delivery_ratio(List* ll, int id1, int id2){
	FIN(compute_delivery_ratio(List* ll, int id1, int id2));
	
	pk_info		*elem;
	int			i;
	int			nb_pk_sent 		= 0;
	int			nb_pk_received 	= 0;
	
	
	//Error
	if (id2 > op_prg_list_size(ll))
		FRET(0);
	
	for(i=0 ; i < op_prg_list_size(ll) ; i++){
		elem = op_prg_list_access(ll , i);
		
		//nb of sent packets
		if ((elem->pk_id <= id2) && (elem->pk_id >= id1))
			nb_pk_sent ++;
		
		//this packet was received
		if ((elem->pk_id <= id2) && (elem->pk_id >= id1) && (elem->received))
			nb_pk_received++;
	}
	FRET((double)nb_pk_received / nb_pk_sent);
}


//returns the average delay for the packets between id1 and id2
double compute_delay(List* ll, int id1, int id2){
	FIN(compute_delay(List* ll, int id1, int id2));
	
	pk_info		*elem;
	int			i;
	double		delay 			= 0;
	int			nb_pk_sent 		= 0;
	int			nb_pk_received 	= 0;
	
	
	//Error
	if (id2 > op_prg_list_size(ll))
		FRET(0);
	
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
	FRET(delay / nb_pk_received);
}



//returns the max delay for the packets between id1 and id2
double compute_max_delay(List* ll, int id1, int id2){
	FIN(compute_max_delay(List* ll, int id1, int id2));
	
	pk_info		*elem;
	int			i;
	double		max_delay 		= 0;
	int			nb_pk_sent 		= 0;
	int			nb_pk_received 	= 0;
	
	
	//Error
	if (id2 > op_prg_list_size(ll))
		FRET(0);
	
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
	FRET(max_delay);
}





//-----------------------------------------------------------
//
//							DEBUG
//
//-----------------------------------------------------------


//Writes a file containing info about the received/generated packets
void debug_write_pk_info(){
	FIN(debug_write_pk_info());

	int			i;
	FILE* 		pfile;
	char		filename[200];
	char		msg[500];
	//Stats
	pk_info		*elem;

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
	FOUT;
}





//-----------------------------------------------------------
//
//			   INTERMEDIARY	STATS
//
//-----------------------------------------------------------


//Prints parameters, etc ...
void print_headers_stat_file(FILE *pfile){
	FIN(print_headers_stat_file(FILE *pfile));
	
	//Simulation parameters
	int		RTS , BETA;
	int		routing_mac = 0;
	int		routing_up = 0;

	
	
	
	//-------------------------------------------
	//				PARAMETERS
	//-------------------------------------------
	
	op_ima_sim_attr_get(OPC_IMA_INTEGER , 	"RTS" , 			&RTS);
	op_ima_sim_attr_get(OPC_IMA_INTEGER , 	"BETA" , 			&BETA);
	
	if (op_ima_sim_attr_exists("ROUTING_MAC"))
		op_ima_sim_attr_get(OPC_IMA_INTEGER , 	"ROUTING_MAC" , &routing_mac);
	else
		routing_mac = -1;
	if (op_ima_sim_attr_exists("ROUTING_UP"))
		op_ima_sim_attr_get(OPC_IMA_INTEGER , 	"ROUTING_UP" ,	&routing_up);
	else
		routing_up = -1;
	


	fprintf(pfile , " ------------------------------------------------------------------------------------------------------------\n");
	fprintf(pfile , "|                                           SIMULATION PARAMETERS                                            |\n");
	fprintf(pfile , " ------------------------------------------------------------------------------------------------------------\n");

	fprintf(pfile , "------------------------------------------------  GENERAL ---------------------------------------------------\n");
	fprintf(pfile , "Number of nodes							:	%d\n", 			nb_nodes);
	fprintf(pfile , "Radio Range								:	%d\n", 			range);
	fprintf(pfile , "Duration								:	%f\n", 				op_sim_time());
	fprintf(pfile , "RTS									:	%d\n", 				RTS);
	fprintf(pfile , "Inter Packet Time							:	%f\n", 			current_inter_pk_time);
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
	
	FOUT;
}



//Write delivery ratio, delay, max_delay
void write_intermediary_stats(int low_pk_id , int high_pk_id){
	FIN(write_intermediary_stats(int low_pk_id , int high_pk_id));
	
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
	FOUT;
}	

//-----------------------------------------------------------
//
//			   ADAPT THE RATE (inter packet time)
//
//-----------------------------------------------------------


//Updates the parameter value in the source process
void update_packet_arrival_in_source_process(double inter_pk_time , double start_time){
	FIN(update_packet_arrival_in_source_process(double inter_pk_time , double start_time));
	
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
	FOUT;
}

//adapt the inter packet time
void adapt_rate(void *arg , int code){
	FIN(adapt_rate(void *arg , int code));
	
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
		FOUT;
	
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
	printf("NEW RATE : %f (delivery ratio %f)\n", current_inter_pk_time , delivery_ratio);
	
	FOUT;
}


//We compute the required stats -> stop the simulation
void stop_rate(void *arg , int code){
	FIN(stop_rate(void *arg , int code));
	
	//max pk_id to compute our stats
	int		pk_id;
	
	//pk_id_tmp
	pk_id = *(int*)arg;
	op_prg_mem_free(arg);
	
	//Next scheduled
	rate_adapt_scheduled 		= OPC_FALSE;

	//not enough packets
	if (lowest_pk_id_authorized + PK_ID_MODULO / 10 > pk_id)
		FOUT;
	
	//Write all stats in a file
	write_intermediary_stats(lowest_pk_id_authorized , pk_id);

	//Other packets in transit must be dropped
	lowest_pk_id_authorized 	= current_pk_id;

	
	//simulation end !
	op_sim_end("The stats were computed" , "" , "" , "");
	
	FOUT;
}




//-----------------------------------------------------------
//
//			   ROUTES MANAGEMENT
//
//-----------------------------------------------------------


//Prints the content of a route (route_length, source, ..., destination)
void print_route(List* route_tmp){
	FIN(print_route(List* route_tmp));
	int		i;
	int		*int_ptr;

	printf("ROUTE : ");
	for(i=0 ; i < op_prg_list_size(route_tmp) ; i++){
		int_ptr = op_prg_list_access(route_tmp, i);
	
		printf(" %d" , *int_ptr);
		}
	printf("\n");
	FOUT;
}










//-----------------------------------------------------------
//
//			  			 TRANSMISSION
//
//-----------------------------------------------------------

void pk_send_to_mac(Packet * pkptr , int next_hop_tmp){
	FIN(pk_send_to_mac(Packet * pkptr , int next_hop_tmp));
		
	double	power_ratio;
	int		x_dist , y_dist;
	double	dist;

	//Power ratio (compared to the maximum power)
	y_dist = (mac_address - next_hop_tmp) / 100;
	x_dist = mac_address - next_hop_tmp - y_dist * 100;
	dist = sqrt( pow(x_dist , 2) + pow(y_dist , 2));

	//radio propagation model in alpha = 4
	if (routing_type == OPT2_ROUTING)
		power_ratio = pow(dist / range , 4);
	else
		power_ratio = 1;


	//Packet transmission
	op_ici_attr_set (wlan_mac_req_iciptr, "dest_addr", 		next_hop_tmp);
//	op_ici_attr_set (wlan_mac_req_iciptr, "power_ratio", 	power_ratio);
	op_ici_install (wlan_mac_req_iciptr);
	op_pk_send (pkptr, STREAM_TO_MAC);

	FOUT;
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
	void wifi_interface_auto (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Obtype _op_wifi_interface_auto_init (int * init_block_ptr);
	void _op_wifi_interface_auto_diag (OP_SIM_CONTEXT_ARG_OPT);
	void _op_wifi_interface_auto_terminate (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Address _op_wifi_interface_auto_alloc (VosT_Obtype, int);
	void _op_wifi_interface_auto_svar (void *, const char *, void **);


	VosT_Obtype Vos_Define_Object_Prstate (const char * _op_name, size_t _op_size);
	VosT_Address Vos_Alloc_Object (VosT_Obtype _op_ob_hndl);
	VosT_Fun_Status Vos_Poolmem_Dealloc (VosT_Address _op_ob_ptr);
#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
wifi_interface_auto (OP_SIM_CONTEXT_ARG_OPT)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	FIN_MT (wifi_interface_auto ());

		{
		/* Temporary Variables */
		List*				proc_record_handle_list_ptr;
		int					record_handle_list_size;
		/* End of Temporary Variables */


		FSM_ENTER ("wifi_interface_auto")

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (init) enter executives **/
			FSM_STATE_ENTER_UNFORCED_NOLABEL (0, "init", "wifi_interface_auto [init enter execs]")
				FSM_PROFILE_SECTION_IN ("wifi_interface_auto [init enter execs]", state0_enter_exec)
				{
				//Addresses
				char		str[500];
				int			my_address;
				//Control
				int			i;
				//Topology
				int			node_id;
				int			mac_id;
				
				
				
				
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
				op_ima_sim_attr_get(OPC_IMA_INTEGER, 	"RADIO_RANGE" , 					&range);
				op_ima_sim_attr_get(OPC_IMA_INTEGER, 	"BETA" , 							&beta);
				op_ima_sim_attr_get(OPC_IMA_INTEGER, 	"SELF_POSITION",					&self_position);
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
				
				
				
				
				//Sets this value for all the nodes (if self_position)
				if (self_position){
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
				FSM_PROFILE_SECTION_OUT (state0_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (1,"wifi_interface_auto")


			/** state (init) exit executives **/
			FSM_STATE_EXIT_UNFORCED (0, "init", "wifi_interface_auto [init exit execs]")
				FSM_PROFILE_SECTION_IN ("wifi_interface_auto [init exit execs]", state0_exit_exec)
				{
				/* Schedule a self interrupt to wait for lower layer	*/
				/* wlan MAC process to initialize and register itself in*/
				/* the model-wide process registry.						*/
				op_intrpt_schedule_self (op_sim_time (), 0);
				
				}
				FSM_PROFILE_SECTION_OUT (state0_exit_exec)


			/** state (init) transition processing **/
			FSM_TRANSIT_FORCE (5, state5_enter_exec, ;, "default", "", "init", "init2", "wifi_interface_auto [init -> init2 : default / ]")
				/*---------------------------------------------------------*/



			/** state (idle) enter executives **/
			FSM_STATE_ENTER_UNFORCED (1, "idle", state1_enter_exec, "wifi_interface_auto [idle enter execs]")
				FSM_PROFILE_SECTION_IN ("wifi_interface_auto [idle enter execs]", state1_enter_exec)
				{
				
				}
				FSM_PROFILE_SECTION_OUT (state1_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,"wifi_interface_auto")


			/** state (idle) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "idle", "wifi_interface_auto [idle exit execs]")
				FSM_PROFILE_SECTION_IN ("wifi_interface_auto [idle exit execs]", state1_exit_exec)
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
				FSM_PROFILE_SECTION_OUT (state1_exit_exec)


			/** state (idle) transition processing **/
			FSM_PROFILE_SECTION_IN ("wifi_interface_auto [idle trans conditions]", state1_trans_conds)
			FSM_INIT_COND (APPL_LAYER_PKT_ARVL && !ENDSIM)
			FSM_TEST_COND (MAC_LAYER_PKT_ARVL && !ENDSIM)
			FSM_TEST_COND (ENDSIM)
			FSM_TEST_LOGIC ("idle")
			FSM_PROFILE_SECTION_OUT (state1_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 2, state2_enter_exec, ;, "APPL_LAYER_PKT_ARVL && !ENDSIM", "", "idle", "appl layer arrival", "wifi_interface_auto [idle -> appl layer arrival : APPL_LAYER_PKT_ARVL && !ENDSIM / ]")
				FSM_CASE_TRANSIT (1, 3, state3_enter_exec, ;, "MAC_LAYER_PKT_ARVL && !ENDSIM", "", "idle", "mac layer arrival", "wifi_interface_auto [idle -> mac layer arrival : MAC_LAYER_PKT_ARVL && !ENDSIM / ]")
				FSM_CASE_TRANSIT (2, 6, state6_enter_exec, ;, "ENDSIM", "", "idle", "END_SIM", "wifi_interface_auto [idle -> END_SIM : ENDSIM / ]")
				}
				/*---------------------------------------------------------*/



			/** state (appl layer arrival) enter executives **/
			FSM_STATE_ENTER_FORCED (2, "appl layer arrival", state2_enter_exec, "wifi_interface_auto [appl layer arrival enter execs]")
				FSM_PROFILE_SECTION_IN ("wifi_interface_auto [appl layer arrival enter execs]", state2_enter_exec)
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
				FSM_PROFILE_SECTION_OUT (state2_enter_exec)

			/** state (appl layer arrival) exit executives **/
			FSM_STATE_EXIT_FORCED (2, "appl layer arrival", "wifi_interface_auto [appl layer arrival exit execs]")


			/** state (appl layer arrival) transition processing **/
			FSM_PROFILE_SECTION_IN ("wifi_interface_auto [appl layer arrival trans conditions]", state2_trans_conds)
			FSM_INIT_COND (ENDSIM)
			FSM_TEST_COND (!ENDSIM)
			FSM_TEST_LOGIC ("appl layer arrival")
			FSM_PROFILE_SECTION_OUT (state2_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 6, state6_enter_exec, ;, "ENDSIM", "", "appl layer arrival", "END_SIM", "wifi_interface_auto [appl layer arrival -> END_SIM : ENDSIM / ]")
				FSM_CASE_TRANSIT (1, 1, state1_enter_exec, ;, "!ENDSIM", "", "appl layer arrival", "idle", "wifi_interface_auto [appl layer arrival -> idle : !ENDSIM / ]")
				}
				/*---------------------------------------------------------*/



			/** state (mac layer arrival) enter executives **/
			FSM_STATE_ENTER_FORCED (3, "mac layer arrival", state3_enter_exec, "wifi_interface_auto [mac layer arrival enter execs]")
				FSM_PROFILE_SECTION_IN ("wifi_interface_auto [mac layer arrival enter execs]", state3_enter_exec)
				{
				//Src and destination
				int			source , dest;
				//Information about the packet to receive/forward
				pk_info		*info_ptr;
				int			pk_id_tmp;
				//Debug
				char		msg[500];
				//tmp vaue
				int			*int_ptr;
				Packet		*pkptr;
				
				
				
				
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
				FSM_PROFILE_SECTION_OUT (state3_enter_exec)

			/** state (mac layer arrival) exit executives **/
			FSM_STATE_EXIT_FORCED (3, "mac layer arrival", "wifi_interface_auto [mac layer arrival exit execs]")


			/** state (mac layer arrival) transition processing **/
			FSM_PROFILE_SECTION_IN ("wifi_interface_auto [mac layer arrival trans conditions]", state3_trans_conds)
			FSM_INIT_COND (ENDSIM)
			FSM_TEST_COND (!ENDSIM)
			FSM_TEST_LOGIC ("mac layer arrival")
			FSM_PROFILE_SECTION_OUT (state3_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 6, state6_enter_exec, ;, "ENDSIM", "", "mac layer arrival", "END_SIM", "wifi_interface_auto [mac layer arrival -> END_SIM : ENDSIM / ]")
				FSM_CASE_TRANSIT (1, 1, state1_enter_exec, ;, "!ENDSIM", "", "mac layer arrival", "idle", "wifi_interface_auto [mac layer arrival -> idle : !ENDSIM / ]")
				}
				/*---------------------------------------------------------*/



			/** state (wait) enter executives **/
			FSM_STATE_ENTER_UNFORCED (4, "wait", state4_enter_exec, "wifi_interface_auto [wait enter execs]")
				FSM_PROFILE_SECTION_IN ("wifi_interface_auto [wait enter execs]", state4_enter_exec)
				{
				
				}
				FSM_PROFILE_SECTION_OUT (state4_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (9,"wifi_interface_auto")


			/** state (wait) exit executives **/
			FSM_STATE_EXIT_UNFORCED (4, "wait", "wifi_interface_auto [wait exit execs]")
				FSM_PROFILE_SECTION_IN ("wifi_interface_auto [wait exit execs]", state4_exit_exec)
				{
				char	msg[500];
				//Inter pk time
				int		mac_id;
				//Real coordinates
				double	x , y;
				//Grid coordinates
				int		x_int , y_int ;
				int		x_sink , y_sink;
				double	x_center , y_center;
				int		x_dev , y_dev;
				double	radius;
				//Contro//Routes
				List 	*my_route_tmp;
				int		*int_ptr;
				
				
				
				
				
				
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
				
				
				
				/*
				//--------------------------------------------
				//
				//			MAC ADDRESS + STREAMS
				//
				//--------------------------------------------
				
				//Address
				mac_id = op_topo_assoc (op_id_self(), OPC_TOPO_ASSOC_OUT, OPC_OBJTYPE_PROC, STREAM_TO_MAC);
				op_ima_obj_attr_get(mac_id , "Address" , 	&mac_address);
				op_ima_obj_attr_get(mac_id , "Is Sink" , 	&is_sink);
				*/
				
				
				
				
				//--------------------------------------------
				//
				//					POSITION
				//
				//--------------------------------------------
				
				y_int = mac_address /  100;
				x_int = mac_address - y_int * 100;
				
				y_sink = (int)(pk_destination /  100);
				x_sink = (int)(pk_destination - y_sink * 100);
				
				x = (double)x_int * PHYSIC_RADIO_RANGE / (double)range;
				y = (double)y_int * PHYSIC_RADIO_RANGE / (double)range;
				
				if (self_position){
					op_ima_obj_attr_set(op_id_parent(op_id_self()) , "x position" , x);
					op_ima_obj_attr_set(op_id_parent(op_id_self()) , "y position" , y);
				}
				
				
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
						next_hop = get_next_hop_via_xy_routing(range , pk_destination);
					break;
					
					
					
					
					//----------------------------------------
					//		   	OPTIMAL CORNER ROUTING
					//----------------------------------------
					case OPT_ROUTING :
						radius = (double)beta * range / 2;
						x_center = x_sink + beta * range / (2 * sqrt(2));
						y_center = y_sink + beta * range / (2 * sqrt(2));
						
						//The sink MUST be in the corner
						if (pk_destination != MIN_ADDRESS){
							sprintf(msg , "Min address (%d) != destination (%d)" , MIN_ADDRESS , pk_destination);
						}
						
					
						//Shortest path (in the maximum clique)
						if (get_distance(x_center , y_center , (double)x_int , (double)y_int) <= radius)
							next_hop = get_next_hop_via_shortest_routing(range , pk_destination); 
						//SIDES - modified (to avoid the interferences circle)
						else
							next_hop = get_next_hop_via_opt_corner_sides_routing(range , pk_destination , x_center , y_center , radius); 
						
					if (mac_address == pk_destination){
						printf("SINK %d %d\n", x_sink , y_sink);
						printf("CENTER : %f %f\n",x_center , y_center);
					}
					
					break;
				
					
					
					
				   	//------------------------------------------------
					//				OPT_CORNER 2
					//------------------------------------------------
					case OPT2_ROUTING :
						radius = (double)beta * (double)range;
						x_center = x_sink;
						y_center = y_sink;		
				
						//special path (in the beta-center)
						if (get_distance(x_center , y_center , (double)x_int , (double)y_int) <= (double)radius){
							next_hop = get_next_hop_via_opt2_routing(range , pk_destination); 
						}
						//SIDES
						else{
							next_hop = get_next_hop_via_opt_corner_sides_routing(range , pk_destination , x_center , y_center , radius); 
						}
						
					break;
				
					
					
					
					
				   	//----------------------------------------
					//		   	SHORTEST ROUTES
					//----------------------------------------
					case SHORT_ROUTING :
						
						next_hop = get_next_hop_via_shortest_routing(range , pk_destination); 
					
					break;
						
					
					
					
				   	//------------------------------------------------
					//	SHORTEST ROUTES VIA THE SIDES OF THE SQUARE
					//------------------------------------------------
					case SIDES_ROUTING :
						
						next_hop = get_next_hop_via_sides_routing(range , pk_destination); 
					
					break;
					
					
					
				
				   	//------------------------------------------------
					//				'F_alpha' ROUTES
					//------------------------------------------------
					case ALPHA_ROUTING :
						radius = (double)beta * (double)range;
						x_center = x_sink;
						y_center = y_sink;		
				
						//printf("%f %f %d %d %f > %f\n",  x_center , y_center , x_int , y_int , radius , get_distance(x_center , y_center , (double)x_int , (double)y_int));
						
						//Shortest path (in the beta-center)
						if (get_distance(x_center , y_center , (double)x_int , (double)y_int) <= (double)radius){
							//printf("IN  ");
							next_hop = get_next_hop_via_shortest_routing(range , pk_destination); 
						}
						//SIDES
						else{
							//printf("OUT  ");
							next_hop = get_next_hop_via_sides_routing(range , pk_destination); 
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
				
				
				if ((next_hop == mac_address) && (!is_sink) && (routing_type != NO_ROUTING))
					op_sim_end("Bug in the routing algorithm in the mac-interface process" , "" , "" , "");
				
				
				
				
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
				FSM_PROFILE_SECTION_OUT (state4_exit_exec)


			/** state (wait) transition processing **/
			FSM_TRANSIT_FORCE (7, state7_enter_exec, ;, "default", "", "wait", "stats_init", "wifi_interface_auto [wait -> stats_init : default / ]")
				/*---------------------------------------------------------*/



			/** state (init2) enter executives **/
			FSM_STATE_ENTER_UNFORCED (5, "init2", state5_enter_exec, "wifi_interface_auto [init2 enter execs]")

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (11,"wifi_interface_auto")


			/** state (init2) exit executives **/
			FSM_STATE_EXIT_UNFORCED (5, "init2", "wifi_interface_auto [init2 exit execs]")
				FSM_PROFILE_SECTION_IN ("wifi_interface_auto [init2 exit execs]", state5_exit_exec)
				{
				/* Schedule a self interrupt to wait for lower layer	*/
				/* Wlan MAC process to finalize the MAC address			*/
				/* registration and resolution.							*/
				op_intrpt_schedule_self (op_sim_time (), 0);
				}
				FSM_PROFILE_SECTION_OUT (state5_exit_exec)


			/** state (init2) transition processing **/
			FSM_TRANSIT_FORCE (4, state4_enter_exec, ;, "default", "", "init2", "wait", "wifi_interface_auto [init2 -> wait : default / ]")
				/*---------------------------------------------------------*/



			/** state (END_SIM) enter executives **/
			FSM_STATE_ENTER_UNFORCED (6, "END_SIM", state6_enter_exec, "wifi_interface_auto [END_SIM enter execs]")
				FSM_PROFILE_SECTION_IN ("wifi_interface_auto [END_SIM enter execs]", state6_enter_exec)
				{
				FILE* 		pfile;
				char		filename[200];
				char		msg[500];
				int			i;
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
				FSM_PROFILE_SECTION_OUT (state6_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (13,"wifi_interface_auto")


			/** state (END_SIM) exit executives **/
			FSM_STATE_EXIT_UNFORCED (6, "END_SIM", "wifi_interface_auto [END_SIM exit execs]")


			/** state (END_SIM) transition processing **/
			FSM_TRANSIT_MISSING ("END_SIM")
				/*---------------------------------------------------------*/



			/** state (stats_init) enter executives **/
			FSM_STATE_ENTER_UNFORCED (7, "stats_init", state7_enter_exec, "wifi_interface_auto [stats_init enter execs]")

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (15,"wifi_interface_auto")


			/** state (stats_init) exit executives **/
			FSM_STATE_EXIT_UNFORCED (7, "stats_init", "wifi_interface_auto [stats_init exit execs]")
				FSM_PROFILE_SECTION_IN ("wifi_interface_auto [stats_init exit execs]", state7_exit_exec)
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
				FSM_PROFILE_SECTION_OUT (state7_exit_exec)


			/** state (stats_init) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "stats_init", "idle", "wifi_interface_auto [stats_init -> idle : default / ]")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (0,"wifi_interface_auto")
		}
	}




void
_op_wifi_interface_auto_diag (OP_SIM_CONTEXT_ARG_OPT)
	{
	/* No Diagnostic Block */
	}




void
_op_wifi_interface_auto_terminate (OP_SIM_CONTEXT_ARG_OPT)
	{

	FIN_MT (_op_wifi_interface_auto_terminate ())


	/* No Termination Block */

	Vos_Poolmem_Dealloc (op_sv_ptr);

	FOUT
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in _op_wifi_interface_auto_svar function. */
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
#undef range
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

#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE

#define FIN_PREAMBLE_DEC
#define FIN_PREAMBLE_CODE

VosT_Obtype
_op_wifi_interface_auto_init (int * init_block_ptr)
	{
	VosT_Obtype obtype = OPC_NIL;
	FIN_MT (_op_wifi_interface_auto_init (init_block_ptr))

	obtype = Vos_Define_Object_Prstate ("proc state vars (wifi_interface_auto)",
		sizeof (wifi_interface_auto_state));
	*init_block_ptr = 0;

	FRET (obtype)
	}

VosT_Address
_op_wifi_interface_auto_alloc (VosT_Obtype obtype, int init_block)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	wifi_interface_auto_state * ptr;
	FIN_MT (_op_wifi_interface_auto_alloc (obtype))

	ptr = (wifi_interface_auto_state *)Vos_Alloc_Object (obtype);
	if (ptr != OPC_NIL)
		{
		ptr->_op_current_block = init_block;
#if defined (OPD_ALLOW_ODB)
		ptr->_op_current_state = "wifi_interface_auto [init enter execs]";
#endif
		}
	FRET ((VosT_Address)ptr)
	}



void
_op_wifi_interface_auto_svar (void * gen_ptr, const char * var_name, void ** var_p_ptr)
	{
	wifi_interface_auto_state		*prs_ptr;

	FIN_MT (_op_wifi_interface_auto_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (void *)OPC_NIL;
		FOUT
		}
	prs_ptr = (wifi_interface_auto_state *)gen_ptr;

	if (strcmp ("my_objid" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->my_objid);
		FOUT
		}
	if (strcmp ("my_node_objid" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->my_node_objid);
		FOUT
		}
	if (strcmp ("instrm_from_mac_" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->instrm_from_mac_);
		FOUT
		}
	if (strcmp ("outstrm_to_mac_" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->outstrm_to_mac_);
		FOUT
		}
	if (strcmp ("oms_aa_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->oms_aa_handle);
		FOUT
		}
	if (strcmp ("mac_address" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->mac_address);
		FOUT
		}
	if (strcmp ("wlan_mac_req_iciptr" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->wlan_mac_req_iciptr);
		FOUT
		}
	if (strcmp ("next_hop" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->next_hop);
		FOUT
		}
	if (strcmp ("pk_destination" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->pk_destination);
		FOUT
		}
	if (strcmp ("routing_type" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->routing_type);
		FOUT
		}
	if (strcmp ("range" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->range);
		FOUT
		}
	if (strcmp ("self_position" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->self_position);
		FOUT
		}
	if (strcmp ("my_stat_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->my_stat_id);
		FOUT
		}
	if (strcmp ("rate_adaptation" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->rate_adaptation);
		FOUT
		}
	if (strcmp ("beta" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->beta);
		FOUT
		}
	if (strcmp ("my_route_to_sink" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->my_route_to_sink);
		FOUT
		}
	if (strcmp ("is_border_node" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->is_border_node);
		FOUT
		}
	if (strcmp ("mac_backoff_type" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->mac_backoff_type);
		FOUT
		}
	if (strcmp ("id_list" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->id_list);
		FOUT
		}
	if (strcmp ("DEBUG" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->DEBUG);
		FOUT
		}
	if (strcmp ("is_sink" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->is_sink);
		FOUT
		}
	*var_p_ptr = (void *)OPC_NIL;

	FOUT
	}

