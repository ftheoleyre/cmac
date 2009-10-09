/*
 *  cmac_tools.c
 *  
 *  Created by Fabrice Theoleyre on 25/08/09.
 *  Copyright 2009 CNRS / LIG. All rights reserved.
 *
 */


#include	"cmac_process.h"
#include 	"cmac_tools.h"






//-----------------------------------------------------------------------------
//						GRAPH
//-----------------------------------------------------------------------------

//constructs the graph
graph_struct **cmac_tools_graph_construct(graph_struct **graph, Objid *mac_ids){

	FIN(cmac_tools_graph_construct(graph_struct **graph, Objid *mac_ids));
	
	//control
	int				i, j;
	int				nb_nodes = get_nb_nodes();
	//neigh tables
	List			**neigh_table_ptr;
	int				nb_neigh;
	neigh_struct 	*neigh_ptr;
	int				neigh_id;
	//state variables
	sink_tree_struct  *ptr_sink_tree;

	
	//constructs the associated graph
	graph = op_prg_mem_alloc(sizeof(graph_struct*) * nb_nodes);
	for(i=0;i<nb_nodes; i++){
		graph[i] = op_prg_mem_alloc(sizeof(graph_struct) * nb_nodes);
		for(j=0; j<nb_nodes; j++){
			graph[i][j].link 		= GRAPH_LINK_NO;
			graph[i][j].thickness 	= 1;
		}
	}
	
	//walk in the neighborhood of each node
	for(i=0;i<nb_nodes; i++){
		neigh_table_ptr = op_ima_obj_svar_get(mac_ids[i], "my_neighborhood_table");
		ptr_sink_tree 	= op_ima_obj_svar_get(mac_ids[i], "my_sink_tree");
	
		nb_neigh = op_prg_list_size(*neigh_table_ptr);
		for(j=0; j<nb_neigh; j++){
			neigh_ptr = op_prg_list_access(*neigh_table_ptr, j);
						
			neigh_id = addr_to_nodeid(neigh_ptr->address);	
			if (neigh_id == ADDR_INVALID){
				printf("invalid neighid, cannot plot xfig\n");
				FRET(NULL);
			}		
			
			//tree link
			if ((neigh_ptr->address == ptr_sink_tree->parent) && (graph[neigh_id][i].thickness < 2)){
				graph[neigh_id][i].thickness = 2;
				graph[i][neigh_id].thickness = 2;
			}
			//cmac link (i.e. branch)
			if (is_cmac_child_of(neigh_ptr, nodeid_to_addr(i))){
				graph[neigh_id][i].thickness = 4;
				graph[i][neigh_id].thickness = 4;
			}
				
			//color different according to the stability metric value
			graph[i][neigh_id].link 	= GRAPH_LINK_RADIO;
			graph[neigh_id][i].link 	= GRAPH_LINK_RADIO;	
			if(stability_get(neigh_ptr) > STAB_MIN){
				graph[neigh_id][i].color = GRAY;
				graph[i][neigh_id].color = GRAY;
			}
			else{
				graph[i][neigh_id].color = RED;
				graph[neigh_id][i].color = RED;
			}				
		}		
	}
	FRET(graph);
}

//gets some useful variables
void cmac_tools_vars_get(int **states_ptr, Objid **nodes_ids_ptr, Objid **mac_ids_ptr, pos_struct **positions_ptr){
	FIN(cmac_tools_vars_get(int **states_ptr, Objid **nodes_ids_ptr, Objid **mac_ids_ptr, pos_struct **positions_ptr));
	int			i;
	int			nb_nodes = get_nb_nodes();
	//results
	int			*states		= *states_ptr;
	Objid		*node_ids	= *nodes_ids_ptr;
	Objid		*mac_ids	= *mac_ids_ptr;
	pos_struct 	*positions	= *positions_ptr;

	//initialization
	states	 	= op_prg_mem_alloc(sizeof(int) * nb_nodes);
	mac_ids 	= op_prg_mem_alloc(sizeof(Objid) * nb_nodes);
	node_ids 	= op_prg_mem_alloc(sizeof(Objid) * nb_nodes);
	positions 	= op_prg_mem_alloc(sizeof(pos_struct) * nb_nodes);
	for(i=0; i<nb_nodes; i++){
		positions[i].x = 0;
		positions[i].y = 0;
	}

	//reads the topology
	if (op_topo_object_count (OPC_OBJTYPE_NDMOB) > nb_nodes){
		printf("Some mobile nodes are not cmac nodes. I will not be able to plot xfig file\n");
		FOUT;
	}

	for(i=0; i<op_topo_object_count (OPC_OBJTYPE_NDMOB) ; i++){
		node_ids[i] = op_topo_object(OPC_OBJTYPE_NDMOB, i);
		mac_ids[i]	= op_id_from_name(node_ids[i], OPC_OBJTYPE_PROC, "mac"); 
		if (mac_ids[i] == OPC_OBJID_INVALID){
			printf("I am unable to find the id for the MAC layer, I will not be able to plot the xfig file\n");
			FOUT;
		}
		
		//geo positions
		op_ima_obj_attr_get_dbl(node_ids[i], "x position", &(positions[i].x));
		op_ima_obj_attr_get_dbl(node_ids[i], "y position", &(positions[i].y));

		//states
		sink_tree_struct  *ptr_sink_tree;
		ptr_sink_tree = op_ima_obj_svar_get(mac_ids[i], "my_sink_tree");
		if (ptr_sink_tree == OPC_NIL){
			printf("The my_sink_tree state variable is not defined, I break the xfig generation\n");
			FOUT;
		}
		states[i] = ptr_sink_tree->is_in_ktree;
	}
	
	//the correct values for resulting arguments
	*states_ptr		= states;
	*nodes_ids_ptr	= node_ids;
	*mac_ids_ptr	= mac_ids;
	*positions_ptr	= positions;
	
	FOUT;
}

//walk in the graph, marking neighbors 1 by 1
void cmac_tools_graph_is_connected_walk(Objid *mac_ids, int id, Boolean *connectivity){
	FIN(cmac_tools_graph_is_connected_walk(Objid *mac_ids, int id, Boolean *connectivity));
	//neigh tables
	List			**neigh_table_ptr;
	int				nb_neigh;
	neigh_struct 	*neigh_ptr;
	//control
	int				i;
	
	//get the neighborhood table
	neigh_table_ptr = op_ima_obj_svar_get(mac_ids[id], "my_neighborhood_table");
	
	//walk in this neighborhood table
	nb_neigh = op_prg_list_size(*neigh_table_ptr);
	for(i=0; i<nb_neigh; i++){
		neigh_ptr = op_prg_list_access(*neigh_table_ptr, i);
		
		//this node is part of the largest connected component
		if ((!connectivity[addr_to_nodeid(neigh_ptr->address)]) && (stability_get(neigh_ptr) > STAB_MIN)){
			connectivity[addr_to_nodeid(neigh_ptr->address)] = OPC_TRUE;
			cmac_tools_graph_is_connected_walk(mac_ids, addr_to_nodeid(neigh_ptr->address), connectivity);
		}
	}

	FOUT;
}


//is the graph connected?
Boolean cmac_tools_graph_is_connected(){
	FIN(cmac_tools_graph_is_connected());

	int			nb_nodes = get_nb_nodes();
	int			i;
	//result
	Boolean		*connectivity;
	Boolean		res;
	//object identification
	Objid		*node_ids;
	Objid		*mac_ids;
	int			*states;
	pos_struct	*positions;

	//initialization
	cmac_tools_vars_get(&states, &node_ids, &mac_ids, &positions);	
	
	//initialization
	connectivity = op_prg_mem_alloc(sizeof(Boolean) * nb_nodes);
	for(i=1; i<nb_nodes; i++)
		connectivity[i] = OPC_FALSE;
	connectivity[0] = OPC_TRUE;
		
	//walk in the graph using radio links
	cmac_tools_graph_is_connected_walk(mac_ids, 0, connectivity);
		
	//none can be disconnected
	res = OPC_TRUE;
	for(i=1; (i<nb_nodes) && (res); i++)
		if (!connectivity[i])
			res = OPC_FALSE;
	
	//memory release
	op_prg_mem_free(mac_ids);
	op_prg_mem_free(states);
	op_prg_mem_free(node_ids);
	op_prg_mem_free(positions);

	FRET(res);
}

	
	
	
	
//-----------------------------------------------------------------------------
//						LOG
//-----------------------------------------------------------------------------

	

//The directory (for logs and results)
char log_dir[FILENAME_LOG_MAX];

//get the name
char *get_log_dir(){
	FIN(get_log_dir());
	FRET(log_dir);
}

//fix the name
void set_log_dir(char *name){
	FIN(set_log_dir(char *name));
	char	msg[200];
	
	if (strlen(name) >= FILENAME_LOG_MAX){
		snprintf(msg, 200, "The name %s is too long", name);
		op_sim_end(msg, "", "", "");
	}
	strncpy(log_dir, name, FILENAME_LOG_MAX);	
	FOUT;
}
