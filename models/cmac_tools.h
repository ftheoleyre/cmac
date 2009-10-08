/*
 *  cmac_tools.c
 *  
 *  Created by Fabrice Theoleyre on 29/09/09.
 *  Copyright 2009 CNRS / LIG. All rights reserved.
 *
 */



#ifndef _CMAC_TOOLS_
#define _CMAC_TOOLS_

//a node position
typedef struct{
	double	x;
	double	y;
}pos_struct;

//a graph
typedef struct{
	short	link;
	short	color;
	short	thickness;
}graph_struct;

#include "tools_fig.h"


//constants for the graph
#define		GRAPH_LINK_NO				0
#define		GRAPH_LINK_RADIO			1

//log file
#define	FILENAME_LOG_MAX	50



//-------------------------  GRAPH  -----------------------------------



//graph
graph_struct **cmac_tools_graph_construct(graph_struct **graph, Objid *mac_ids);
void 	cmac_tools_vars_get(int **states_ptr, Objid **nodes_ids_ptr, Objid **mac_ids_ptr, pos_struct **positions_ptr);
Boolean cmac_tools_graph_is_connected();



//-------------------------  LOGS  -----------------------------------


//the log directory
char 	*get_log_dir();
void 	set_log_dir(char *name);



#endif
