/* Process model C form file: sink_perso.pr.c */
/* Portions of this file copyright 1992-2002 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
static const char sink_perso_pr_c [] = "MIL_3_Tfile_Hdr_ 81A 30A modeler 7 4483EB2B 4483EB2B 1 ares-theo-1 ftheoley 0 0 none none 0 0 none 0 0 0 0 0 0                                                                                                                                                                                                                                                                                                                                                                                                                 ";
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
	Stathandle	             		bits_rcvd_stathandle;
	Stathandle	             		bitssec_rcvd_stathandle;
	Stathandle	             		pkts_rcvd_stathandle;
	Stathandle	             		pktssec_rcvd_stathandle;
	Stathandle	             		ete_delay_stathandle;
	Stathandle	             		bits_rcvd_gstathandle;
	Stathandle	             		bitssec_rcvd_gstathandle;
	Stathandle	             		pkts_rcvd_gstathandle;
	Stathandle	             		pktssec_rcvd_gstathandle;
	Stathandle	             		ete_delay_gstathandle;
	} sink_perso_state;

#define pr_state_ptr            		((sink_perso_state*) SimI_Mod_State_Ptr)
#define bits_rcvd_stathandle    		pr_state_ptr->bits_rcvd_stathandle
#define bitssec_rcvd_stathandle 		pr_state_ptr->bitssec_rcvd_stathandle
#define pkts_rcvd_stathandle    		pr_state_ptr->pkts_rcvd_stathandle
#define pktssec_rcvd_stathandle 		pr_state_ptr->pktssec_rcvd_stathandle
#define ete_delay_stathandle    		pr_state_ptr->ete_delay_stathandle
#define bits_rcvd_gstathandle   		pr_state_ptr->bits_rcvd_gstathandle
#define bitssec_rcvd_gstathandle		pr_state_ptr->bitssec_rcvd_gstathandle
#define pkts_rcvd_gstathandle   		pr_state_ptr->pkts_rcvd_gstathandle
#define pktssec_rcvd_gstathandle		pr_state_ptr->pktssec_rcvd_gstathandle
#define ete_delay_gstathandle   		pr_state_ptr->ete_delay_gstathandle

/* This macro definition will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE
#define FIN_PREAMBLE	sink_perso_state *op_sv_ptr = pr_state_ptr;


/* No Function Block */

enum { _block_origin = __LINE__ };

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
	void sink_perso (void);
	Compcode sink_perso_init (void **);
	void sink_perso_diag (void);
	void sink_perso_terminate (void);
	void sink_perso_svar (void *, const char *, char **);
#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
sink_perso (void)
	{
	int _block_origin = 0;
	FIN (sink_perso ());
	if (1)
		{
		Packet*		pkptr;
		double		pk_size;
		double		ete_delay;


		FSM_ENTER (sink_perso)

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (DISCARD) enter executives **/
			FSM_STATE_ENTER_UNFORCED (0, state0_enter_exec, "DISCARD", "sink_perso () [DISCARD enter execs]")
				{
				}


			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (1,sink_perso)


			/** state (DISCARD) exit executives **/
			FSM_STATE_EXIT_UNFORCED (0, "DISCARD", "sink_perso () [DISCARD exit execs]")
				{
				/* Obtain the incoming packet.	*/
				pkptr = op_pk_get (op_intrpt_strm ());
				
				/* Caclulate metrics to be updated.		*/
				pk_size = (double) op_pk_total_size_get (pkptr);
				ete_delay = op_sim_time () - op_pk_creation_time_get (pkptr);
				
				/* Update local statistics.				*/
				op_stat_write (bits_rcvd_stathandle, 		pk_size);
				op_stat_write (pkts_rcvd_stathandle, 		1.0);
				op_stat_write (ete_delay_stathandle, 		ete_delay);
				
				op_stat_write (bitssec_rcvd_stathandle, 	pk_size);
				op_stat_write (bitssec_rcvd_stathandle, 	0.0);
				op_stat_write (pktssec_rcvd_stathandle, 	1.0);
				op_stat_write (pktssec_rcvd_stathandle, 	0.0);
				
				/* Update global statistics.	*/
				op_stat_write (bits_rcvd_gstathandle, 		pk_size);
				op_stat_write (pkts_rcvd_gstathandle, 		1.0);
				op_stat_write (ete_delay_gstathandle, 		ete_delay);
				
				op_stat_write (bitssec_rcvd_gstathandle, 	pk_size);
				op_stat_write (bitssec_rcvd_gstathandle, 	0.0);
				op_stat_write (pktssec_rcvd_gstathandle, 	1.0);
				op_stat_write (pktssec_rcvd_gstathandle, 	0.0);
				
				/* Destroy the received packet.	*/
				op_pk_destroy (pkptr);
				
				
				}


			/** state (DISCARD) transition processing **/
			FSM_TRANSIT_FORCE (0, state0_enter_exec, ;, "default", "", "DISCARD", "DISCARD")
				/*---------------------------------------------------------*/



			/** state (INIT) enter executives **/
			FSM_STATE_ENTER_FORCED_NOLABEL (1, "INIT", "sink_perso () [INIT enter execs]")
				{
				/* Initilaize the statistic handles to keep	*/
				/* track of traffic sinked by this process.	*/
				bits_rcvd_stathandle 		= op_stat_reg ("Traffic Sink.Traffic Received (bits)",			OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
				bitssec_rcvd_stathandle 	= op_stat_reg ("Traffic Sink.Traffic Received (bits/sec)",		OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
				pkts_rcvd_stathandle 		= op_stat_reg ("Traffic Sink.Traffic Received (packets)",		OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
				pktssec_rcvd_stathandle 	= op_stat_reg ("Traffic Sink.Traffic Received (packets/sec)",	OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
				ete_delay_stathandle		= op_stat_reg ("Traffic Sink.End-to-End Delay (seconds)",		OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
				
				bits_rcvd_gstathandle 		= op_stat_reg ("Traffic Sink.Traffic Received (bits)",			OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				bitssec_rcvd_gstathandle 	= op_stat_reg ("Traffic Sink.Traffic Received (bits/sec)",		OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				pkts_rcvd_gstathandle 		= op_stat_reg ("Traffic Sink.Traffic Received (packets)",		OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				pktssec_rcvd_gstathandle 	= op_stat_reg ("Traffic Sink.Traffic Received (packets/sec)",	OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				ete_delay_gstathandle		= op_stat_reg ("Traffic Sink.End-to-End Delay (seconds)",		OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				}


			/** state (INIT) exit executives **/
			FSM_STATE_EXIT_FORCED (1, "INIT", "sink_perso () [INIT exit execs]")
				{
				}


			/** state (INIT) transition processing **/
			FSM_TRANSIT_FORCE (0, state0_enter_exec, ;, "default", "", "INIT", "DISCARD")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (1,sink_perso)
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
sink_perso_init (void ** gen_state_pptr)
	{
	int _block_origin = 0;
	static VosT_Address	obtype = OPC_NIL;

	FIN (sink_perso_init (gen_state_pptr))

	if (obtype == OPC_NIL)
		{
		/* Initialize memory management */
		if (Vos_Catmem_Register ("proc state vars (sink_perso)",
			sizeof (sink_perso_state), Vos_Vnop, &obtype) == VOSC_FAILURE)
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
		((sink_perso_state *)(*gen_state_pptr))->current_block = 2;

		FRET (OPC_COMPCODE_SUCCESS)
		}
	}



void
sink_perso_diag (void)
	{
	/* No Diagnostic Block */
	}




void
sink_perso_terminate (void)
	{
	int _block_origin = __LINE__;

	FIN (sink_perso_terminate (void))

	Vos_Catmem_Dealloc (pr_state_ptr);

	FOUT;
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in sink_perso_svar function. */
#undef bits_rcvd_stathandle
#undef bitssec_rcvd_stathandle
#undef pkts_rcvd_stathandle
#undef pktssec_rcvd_stathandle
#undef ete_delay_stathandle
#undef bits_rcvd_gstathandle
#undef bitssec_rcvd_gstathandle
#undef pkts_rcvd_gstathandle
#undef pktssec_rcvd_gstathandle
#undef ete_delay_gstathandle



void
sink_perso_svar (void * gen_ptr, const char * var_name, char ** var_p_ptr)
	{
	sink_perso_state		*prs_ptr;

	FIN (sink_perso_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (char *)OPC_NIL;
		FOUT;
		}
	prs_ptr = (sink_perso_state *)gen_ptr;

	if (strcmp ("bits_rcvd_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->bits_rcvd_stathandle);
		FOUT;
		}
	if (strcmp ("bitssec_rcvd_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->bitssec_rcvd_stathandle);
		FOUT;
		}
	if (strcmp ("pkts_rcvd_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pkts_rcvd_stathandle);
		FOUT;
		}
	if (strcmp ("pktssec_rcvd_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pktssec_rcvd_stathandle);
		FOUT;
		}
	if (strcmp ("ete_delay_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->ete_delay_stathandle);
		FOUT;
		}
	if (strcmp ("bits_rcvd_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->bits_rcvd_gstathandle);
		FOUT;
		}
	if (strcmp ("bitssec_rcvd_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->bitssec_rcvd_gstathandle);
		FOUT;
		}
	if (strcmp ("pkts_rcvd_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pkts_rcvd_gstathandle);
		FOUT;
		}
	if (strcmp ("pktssec_rcvd_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->pktssec_rcvd_gstathandle);
		FOUT;
		}
	if (strcmp ("ete_delay_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (char *) (&prs_ptr->ete_delay_gstathandle);
		FOUT;
		}
	*var_p_ptr = (char *)OPC_NIL;

	FOUT;
	}

