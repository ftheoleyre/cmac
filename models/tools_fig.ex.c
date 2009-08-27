/*
 *  tools.c
 *  
 *  Created by Fabrice Theoleyre on 25/08/09.
 *  Copyright 2009 CNRS / LIG. All rights reserved.
 *
 */

 
#include 	"tools_fig.h"
#include	"cmac.h"



//-----------------------------------------------------------------------------
//						TOOLS FUNCTIONS
//-----------------------------------------------------------------------------


//returns the maximum x-coordinate
double tools_fig_get_x_max(pos_struct *positions){
	FIN(tools_fig_get_x_max(pos_struct *positions));
	int		i;
	double	x_max =0.0;
			
	for(i=0; i<get_nb_nodes() ; i++){
		if (positions[i].x > x_max)
			x_max = positions[i].x;		
	}
	FRET(x_max);
}

//returns the maximum y-coordinate
double tools_fig_get_y_max(pos_struct *positions){
	FIN(tools_fig_get_y_max(pos_struct *positions));
	int		i;
	double	y_max =0.0 ;
			
	for(i=0; i<get_nb_nodes() ; i++){
		if (positions[i].y > y_max)
			y_max = positions[i].y;		
	}
	FRET(y_max);
}

//returns the color (the WHITE is forbidden, I must shift all the values larger than WHITE)
int tools_fig_set_color(int color){
	FIN(tools_fig_set_color(int color));

	if (color < WHITE)
		FRET(color);
	FRET(color+1);
}


//-----------------------------------------------------------------------------
//						.FIG	FILE
//-----------------------------------------------------------------------------

//generates the .fig file
void tools_fig_write_xfig_file(short **graph, pos_struct *positions, int *states){
	FIN(tools_fig_write_xfig_file(short **graph, pos_struct *positions, int *states));

	short		color , color2, thickness, depth, linestyle, degree=0;
	int			radius;
	//Figure file
	FILE*		pfile;
	char		filename[200];
	//tmp
	int			x1 , y1 , x2 , y2;
	//Control
	int			i, j;
	//parameters
	int			GRAPHIC_XMAX , GRAPHIC_YMAX;
	double		MAX_X = tools_fig_get_x_max(positions) , MAX_Y = tools_fig_get_y_max(positions) ;
	int			nb_nodes = get_nb_nodes();

	//Opens the associated file and 
	//Initialization
	snprintf(filename , 200, "%s/topology.fig", LOG_DIR);
	pfile = fopen(filename , "w");
	if (pfile==NULL){
		printf("Error : we cannot create the file %s\n", filename);
		exit(-1);
	}		

	//Horizontal & vertical Scaling
	if (MAX_X > MAX_Y){
		GRAPHIC_XMAX = GRAPHIC_MAX;
		GRAPHIC_YMAX = MAX_Y * GRAPHIC_MAX / MAX_X;
	}
	else{
		GRAPHIC_YMAX = GRAPHIC_MAX;
		GRAPHIC_XMAX = MAX_X * GRAPHIC_MAX / MAX_Y;
	}
					
	//--------
	//Headers
	//--------
	fprintf(pfile,"#FIG 3.2 \n#Snapshot of The Network Topology (CDS) with CDS-CLUSTER \nLandscape \nCenter \nMetric \nA0 \n100.00 \nSingle \n-2 \n1200 2 \n");						
	//fprintf(pfile,"1 3 0 1 0 0 50 -1 15 0.000 1 0.000 0 0 1 1 0 0 0 0\n");
	//gray color
	fprintf(pfile , "#Gray color\n0 %d #696969\n", GRAY);
			
							
	//-----------------------------
	//Nodes Position and addresses
	//-----------------------------
	fprintf(pfile,"#NODE POSITIONS\n");
	for(i=0 ; i < nb_nodes ; i++){	
		//printf("%f %f\n", G_pos[i].x , G_pos[i].y);
		
		//Disks which represent the node position
		//colors and parameters are different for nucleus and electrons
		if (!states[i]){
			color	= tools_fig_set_color(BLACK);
			color2	= tools_fig_set_color(BLACK);
		}
		else{		
			color	= tools_fig_set_color(RED);
			color2	= tools_fig_set_color(RED);
		}
		radius	= GRAPHIC_RADIUS;
		
		//the disk representing the node
		fprintf(pfile ,"1 3 0 1 %d %d %d -1 15 0.000 1 0.000 %d %d %d %d 0 0 0 0\n", color2 , color , 1 , (int)(positions[i].x * GRAPHIC_XMAX / MAX_X) , (int)(positions[i].y * GRAPHIC_YMAX / MAX_Y) , radius , radius);
		
		//Address associated to this node (this must be printed not far from the node itself)
		fprintf(pfile ,"4 0 0 %d -1 0 %d 0.0000 4 135 135 %d %d %d\\001\n", 0 , GRAPHIC_POLICE_SIZE , GRAPHIC_SHIFT_X + (int)(positions[i].x * GRAPHIC_XMAX / MAX_X) , GRAPHIC_SHIFT_Y + (int)(positions[i].y * GRAPHIC_YMAX / MAX_Y) , nodeid_to_addr(i));
	}
			
			
	//------
	//Links
	//------
	fprintf(pfile,"#LINKS\n");
	for(i=0 ; i < nb_nodes ; i++)
		for(j=0 ; j < nb_nodes ; j++)
			if (graph[i][j] >= GRAPH_LINK_NO){
				degree++;

				//Coordinates of the source and destination of link
				x1 = (int)(positions[i].x / MAX_X * GRAPHIC_XMAX);
				y1 = (int)(positions[i].y / MAX_Y * GRAPHIC_YMAX);
				
				x2 = (int)(positions[j].x / MAX_X * GRAPHIC_XMAX);
				y2 = (int)(positions[j].y / MAX_Y * GRAPHIC_YMAX);
				
				depth		= 4;
				color		= GRAY;
				linestyle	= DASHED;
				thickness	= 1;
						
				fprintf(pfile, "#%d-%d (type %d)\n3 2 %d %d %d 0 %d -1 -1 1.0 0 0 0 2\n			%d %d %d %d\n			0.000 0.000\n", i , j , depth , linestyle , thickness,  color , depth , x1 , y1 , x2 , y2);
			}

	
	//--------
	//Footers
	//--------
	fprintf(pfile,"#END OF FIGURE\n");
	fclose(pfile);
	
	FOUT;
}




//generates a xfig figure
void tools_fig_generate(){
	FIN(tools_fig_generate());

	//infos
	int			*states;
	pos_struct	*positions;
	Objid		*mac_ids;
	Objid		*node_ids;
	short		**graph;
	//control
	int			i, j;
	int			nb_nodes = get_nb_nodes();
	int			*ptr_int;
	//neigh tables
	List		**neigh_table_ptr;
	int			nb_neigh;
	neigh_struct *neigh_ptr;
	int			neigh_id;
	
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
		ptr_int = op_ima_obj_svar_get(mac_ids[i], "is_in_ktree");
		if (ptr_int == OPC_NIL){
			printf("The is_in_ktree state variable is not defined, I break the xfig generation\n");
			FOUT;
		}
		states[i] = *ptr_int;
	}
	
	
	//constructs the associated graph
	graph = op_prg_mem_alloc(sizeof(short*) * nb_nodes);
	for(i=0;i<nb_nodes; i++){
		graph[i] = op_prg_mem_alloc(sizeof(short) * nb_nodes);
		for(j=0; j<nb_nodes; j++)
			graph[i][j] = GRAPH_LINK_NO;
	}
	
	//walk in the neighborhood of each node
	for(i=0;i<nb_nodes; i++){
		neigh_table_ptr = op_ima_obj_svar_get(mac_ids[i], "my_neighborhood_table");
		
		nb_neigh = op_prg_list_size(*neigh_table_ptr);
		for(j=0; j<nb_neigh; j++){
			neigh_ptr = op_prg_list_access(*neigh_table_ptr, j);
						
			neigh_id = addr_to_nodeid(neigh_ptr->address);	
			if (neigh_id == ADDR_INVALID){
				printf("invalid neighid, cannot plot xfig\n");
				FOUT;
			}
			
			graph[i][neigh_id] = GRAPH_LINK_RADIO;
			graph[neigh_id][i] = GRAPH_LINK_RADIO;		
		}		
	}
	
	//now,plot it!
	tools_fig_write_xfig_file(graph, positions, states);
	
	//memory release
	op_prg_mem_free(mac_ids);
	op_prg_mem_free(states);
	op_prg_mem_free(node_ids);
	op_prg_mem_free(positions);

	FOUT;
}
