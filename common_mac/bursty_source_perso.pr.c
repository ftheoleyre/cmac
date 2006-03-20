/* Process model C form file: bursty_source_perso.pr.c */
/* Portions of this file copyright 1992-2002 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
static const char bursty_source_perso_pr_c [] = "MIL_3_Tfile_Hdr_ 81A 30A modeler 7 44158D59 44158D59 1 ares-theo-1 ftheoley 0 0 none none 0 0 none 0 0 0 0 0 0                                                                                                                                                                                                                                                                                                                                                                                                                 ";
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

/* Include files. */
//#include "oms_dist_support.h"
#include <math.h>

#define		PACKET_TIME_DISTRIBUTION		"constant"
#define		PACKET_TIME_ARG2				0
#define		PACKET_SIZE_DISTRIBUTION		"constant"
#define		PACKET_SIZE_ARG1				128.0
#define		PACKET_SIZE_ARG2				0





/* Function Declarations.	*/
static void			bursty_source_sv_init ();


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
	char	                   		pid_string [64];
	Boolean	                		debug_mode;
	Stathandle	             		pksize_stathandle;
	Distribution *	         		intarrvl_time_dist_handle;
	Distribution *	         		packet_size_dist_handle;
	Stathandle	             		bits_sent_stathandle;
	Stathandle	             		bitssec_sent_stathandle;
	Stathandle	             		pkts_sent_stathandle;
	Stathandle	             		pktssec_sent_stathandle;
	Stathandle	             		bits_sent_gstathandle;
	Stathandle	             		bitssec_sent_gstathandle;
	Stathandle	             		pkts_sent_gstathandle;
	Stathandle	             		pktssec_sent_gstathandle;
	double	                 		inter_pk_time;
	} bursty_source_perso_state;

#define pr_state_ptr            		((bursty_source_perso_state*) SimI_Mod_State_Ptr)
#define pid_string              		pr_state_ptr->pid_string
#define debug_mode              		pr_state_ptr->debug_mode
#define pksize_stathandle       		pr_state_ptr->pksize_stathandle
#define intarrvl_time_dist_handle		pr_state_ptr->intarrvl_time_dist_handle
#define packet_size_dist_handle 		pr_state_ptr->packet_size_dist_handle
#define bits_sent_stathandle    		pr_state_ptr->bits_sent_stathandle
#define bitssec_sent_stathandle 		pr_state_ptr->bitssec_sent_stathandle
#define pkts_sent_stathandle    		pr_state_ptr->pkts_sent_stathandle
#define pktssec_sent_stathandle 		pr_state_ptr->pktssec_sent_stathandle
#define bits_sent_gstathandle   		pr_state_ptr->bits_sent_gstathandle
#define bitssec_sent_gstathandle		pr_state_ptr->bitssec_sent_gstathandle
#define pkts_sent_gstathandle   		pr_state_ptr->pkts_sent_gstathandle
#define pktssec_sent_gstathandle		pr_state_ptr->pktssec_sent_gstathandle
#define inter_pk_time           		pr_state_ptr->inter_pk_time

/* This macro definition will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE
#define FIN_PREAMBLE	bursty_source_perso_state *op_sv_ptr = pr_state_ptr;


/* Function Block */

enum { _block_origin = __LINE__ };
static void
bursty_source_sv_init ()
	{
//	Prohandle			my_prohandle;
//	int					my_pro_id;
//	Objid				my_id;
//	Objid				traf_gen_comp_attr_objid, traf_conf_objid;
//	Objid				pkt_gen_comp_attr_objid, pkt_gen_args_objid;
//	char				on_state_string [128], off_state_string [128];
//	char				intarrvl_rate_string [128], packet_size_string [128];
//	char				start_time_string [128];
//	OmsT_Dist_Handle	start_time_dist_handle;
//	char				distrib_pk_time[400];
//	double				arg1_pk_time , arg2_pk_time;

	/**	Initializes state variables associated with		**/
	/**	this process model.								**/
	FIN (bursty_source_sv_init ());

	/*	Determine the prohandle of this process as well as	*/
	/*	the object IDs of the containing module and node.*/
//	my_prohandle = op_pro_self ();
//	my_pro_id = op_pro_id (my_prohandle);
//	my_id = op_id_self ();

	/*	Determine the process ID display string.	*/
//	sprintf (pid_string, "bursty_source PID (%d)", my_pro_id);

	/*	Determine whether or not the simulation is in debug	*/
	/*	mode.  Trace statement are only enabled when the	*/
	/*	simulation is in debug mode.						*/
	debug_mode = op_sim_debug ();

	//Inter-arrival pk distribution
//	op_ima_sim_attr_get (OPC_IMA_STRING, 	"Packet Interarrival Distribution" , 	distrib_pk_time);
//	op_ima_obj_attr_get (op_id_self(), 		"Packet Interarrival Arg1", 			&arg1_pk_time);
//	op_ima_sim_attr_get (OPC_IMA_DOUBLE, 	"Packet Interarrival Arg2", 			&arg2_pk_time);
//	intarrvl_time_dist_handle = op_dist_load (distrib_pk_time , arg1_pk_time , arg2_pk_time);
//	inter_pk_time = arg1_pk_time;

	//Inter-arrival pk distribution
	packet_size_dist_handle = op_dist_load (PACKET_SIZE_DISTRIBUTION , PACKET_SIZE_ARG1 , PACKET_SIZE_ARG2);

	
	/*	Initilaize the packet generation status statistic	*/
	/*	to indicate that currently there are no packets 	*/
	/*	being generated.									*/
	pksize_stathandle = op_stat_reg ("Traffic Source.Packet Generation Status", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	op_stat_write (pksize_stathandle, (double) OPC_FALSE);
		
	/* Initilaize the statistic handles to keep	*/
	/* track of traffic Sourceed by this process.	*/
	bits_sent_stathandle 		= op_stat_reg ("Traffic Source.Traffic Sent (bits)",		OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	bitssec_sent_stathandle 	= op_stat_reg ("Traffic Source.Traffic Sent (bits/sec)",	OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	pkts_sent_stathandle 		= op_stat_reg ("Traffic Source.Traffic Sent (packets)",		OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	pktssec_sent_stathandle 	= op_stat_reg ("Traffic Source.Traffic Sent (packets/sec)",	OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	bits_sent_gstathandle 		= op_stat_reg ("Traffic Source.Traffic Sent (bits)",		OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	bitssec_sent_gstathandle 	= op_stat_reg ("Traffic Source.Traffic Sent (bits/sec)",	OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	pkts_sent_gstathandle 		= op_stat_reg ("Traffic Source.Traffic Sent (packets)",		OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	pktssec_sent_gstathandle 	= op_stat_reg ("Traffic Source.Traffic Sent (packets/sec)",	OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
		
	
	
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
	void bursty_source_perso (void);
	Compcode bursty_source_perso_init (void **);
	void bursty_source_perso_diag (void);
	void bursty_source_perso_terminate (void);
	void bursty_source_perso_svar (void *, const char *, char **);
#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
bursty_source_perso (void)
	{
	int _block_origin = 0;
	FIN (bursty_source_perso ());
	if (1)
		{
		int					intrpt_type;
		int					intrpt_code;
		
		Packet*				pkptr;
		SimT_Pk_Size		pksize;
		double				dval0 = 0.0;
		double				dval1 = 0.0;
		char				pdf_name [256];
		char				pdf_args [256];
		double				on_period;
		double				off_period;
		double				next_packet_arrival_time;


		FSM_ENTER (bursty_source_perso)

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (init) enter executives **/
			FSM_STATE_ENTER_UNFORCED_NOLABEL (0, "init", "bursty_source_perso () [init enter execs]")
				{
				/* Initialize the traffic generation parameters.	*/
				bursty_source_sv_init ();
				
				/* Schedule the first OFF-period scheduling by setting	*/
				/* a self-interrupt for the start time. If the start	*/
				/* time is set to "Infinity", then there is no need to	*/
				/* to schedule an interrupt as this node has been set	*/
				/* will not generate any traffic.					*/
				//The mac_interface or mac processes are badly conceived, they require a synchronization
				
				op_ima_obj_attr_get (op_id_self(), 	"Packet Interarrival Arg1", &inter_pk_time);
				op_intrpt_schedule_self (op_sim_time () + 0.01 + op_dist_uniform(inter_pk_time) , 0);
				
				
				
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (1,bursty_source_perso)


			/** state (init) exit executives **/
			FSM_STATE_EXIT_UNFORCED (0, "init", "bursty_source_perso () [init exit execs]")
				{
				}


			/** state (init) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "init", "on")
				/*---------------------------------------------------------*/



			/** state (on) enter executives **/
			FSM_STATE_ENTER_UNFORCED (1, state1_enter_exec, "on", "bursty_source_perso () [on enter execs]")
				{
				//----------- Traffic generation  -----------------
				double				arg1_pk_time;
				double				time_start;
				
				//Parameters
				op_ima_obj_attr_get (op_id_self(), 		"Packet Interarrival Arg1", 		&arg1_pk_time);
				intarrvl_time_dist_handle = op_dist_load (PACKET_TIME_DISTRIBUTION , arg1_pk_time , PACKET_TIME_ARG2);
				op_ima_obj_attr_get (op_id_self(), 		"Start Time Packet Generation",			&time_start);
				
				
				
				
				//next packet
				next_packet_arrival_time = op_sim_time () + op_dist_outcome (intarrvl_time_dist_handle);
				
				
				//only if start is over-passed (One interarrival pk has surely changed)
				if (time_start < op_sim_time()){
					//PK creation
					pksize = floor ((SimT_Pk_Size) op_dist_outcome (packet_size_dist_handle));
					pksize *= 8;
					pkptr  = op_pk_create (pksize);
				
					//A pk was generated
					op_stat_write (pksize_stathandle, (double) OPC_TRUE);
				
					// Local stats
					op_stat_write (bits_sent_stathandle, 		pksize);
					op_stat_write (pkts_sent_stathandle, 		1.0);
				
					op_stat_write (bitssec_sent_stathandle, 	pksize);
					op_stat_write (bitssec_sent_stathandle, 	0.0);
					op_stat_write (pktssec_sent_stathandle, 	1.0);
					op_stat_write (pktssec_sent_stathandle, 	0.0);
					
					// Gobal stats
					op_stat_write (bits_sent_gstathandle, 		pksize);
					op_stat_write (pkts_sent_gstathandle, 		1.0);
				
					op_stat_write (bitssec_sent_gstathandle, 	pksize);
					op_stat_write (bitssec_sent_gstathandle, 	0.0);
					op_stat_write (pktssec_sent_gstathandle, 	1.0);
					op_stat_write (pktssec_sent_gstathandle, 	0.0);
					
					// Pk transmission
					op_pk_send (pkptr, 0);
				}
				
				//	Schedule the next packet arrival
				if ((next_packet_arrival_time) && (time_start != OPC_DBL_INFINITY))
					op_intrpt_schedule_self (next_packet_arrival_time, 0);
				  
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,bursty_source_perso)


			/** state (on) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "on", "bursty_source_perso () [on exit execs]")
				{
				/* Determine the type of interrupt.		*/
				intrpt_type = op_intrpt_type ();
				intrpt_code = op_intrpt_code ();
				
				}


			/** state (on) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "on", "on")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (0,bursty_source_perso)
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
bursty_source_perso_init (void ** gen_state_pptr)
	{
	int _block_origin = 0;
	static VosT_Address	obtype = OPC_NIL;

	FIN (bursty_source_perso_init (gen_state_pptr))

	if (obtype == OPC_NIL)
		{
		/* Initialize memory management */
		if (Vos_Catmem_Register ("proc state vars (bursty_source_perso)",
			sizeof (bursty_source_perso_state), Vos_Vnop, &obtype) == VOSC_FAILURE)
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
		((bursty_source_perso_state *)(*gen_state_pptr))->current_block = 0;

		FRET (OPC_COMPCODE_SUCCESS)
		}
	}



void
bursty_source_perso_diag (void)
	{
	/* No Diagnostic Block */
	}




void
bursty_source_perso_terminate (void)
	{
	int _block_origin = __LINE__;

	FIN (bursty_source_perso_terminate (void))

	Vos_Catmem_Dealloc (pr_state_ptr);

	FOUT;
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in bursty_source_perso_svar function. */
#undef pid_string
#undef debug_mode
#undef pksize_stathandle
#undef intarrvl_time_dist_handle
#undef packet_size_dist_handle
#undef bits_sent_stathandle
#undef bitssec_sent_stathandle
#undef pkts_sent_stathandle
#undef pktssec_sent_stathandle
#undef bits_sent_gstathandle
#undef bitssec_sent_gstathandle
#undef pkts_sent_gstathandle
#undef pktssec_sent_gstathandle
#undef inter_pk_time



void
bursty_source_perso_svar (void * gen_ptr, const char * var_name, char ** var_p_ptr)
	{
	bursty_source_perso_state		*prs_ptr;

	FIN (bursty_source_perso_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (char *)OPC_NIL;
		FOUT;
		}
	prs_ptr = (bursty_source_perso_state *)gen_ptr;

	if (strcmp ("pid_string" , var_name) == 0)
		{
		*var_p_ptr = (char *) (prs_ptr->pid_string);
		FOUT;
		}
	if (strcmp ("debug_mode" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->debug_mode);
		FOUT;
		}
	if (strcmp ("pksize_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pksize_stathandle);
		FOUT;
		}
	if (strcmp ("intarrvl_time_dist_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->intarrvl_time_dist_handle);
		FOUT;
		}
	if (strcmp ("packet_size_dist_handle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->packet_size_dist_handle);
		FOUT;
		}
	if (strcmp ("bits_sent_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->bits_sent_stathandle);
		FOUT;
		}
	if (strcmp ("bitssec_sent_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->bitssec_sent_stathandle);
		FOUT;
		}
	if (strcmp ("pkts_sent_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pkts_sent_stathandle);
		FOUT;
		}
	if (strcmp ("pktssec_sent_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pktssec_sent_stathandle);
		FOUT;
		}
	if (strcmp ("bits_sent_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->bits_sent_gstathandle);
		FOUT;
		}
	if (strcmp ("bitssec_sent_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->bitssec_sent_gstathandle);
		FOUT;
		}
	if (strcmp ("pkts_sent_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pkts_sent_gstathandle);
		FOUT;
		}
	if (strcmp ("pktssec_sent_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pktssec_sent_gstathandle);
		FOUT;
		}
	if (strcmp ("inter_pk_time" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->inter_pk_time);
		FOUT;
		}
	*var_p_ptr = (char *)OPC_NIL;

	FOUT;
	}

