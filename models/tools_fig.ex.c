/*
 *  tools_fig.c
 *  
 *  Created by Fabrice Theoleyre on 25/08/09.
 *  Copyright 2009 CNRS / LIG. All rights reserved.
 *
 */

 
#include 	"tools_fig.h"
#include	"cmac_process.h"
#include	"cmac_tools.h"



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
void tools_fig_write_xfig_file(graph_struct **graph, pos_struct *positions, int *states){
	FIN(tools_fig_write_xfig_file(graph_struct **graph, pos_struct *positions, int *states));

	short		color , color2, thickness, depth, linestyle, degree=0;
	int			radius;
	//Figure file
	FILE*		pfile;
	char		filename[FILENAME_LOG_MAX];
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
	snprintf(filename , FILENAME_LOG_MAX, "%s/topology.fig", get_log_dir());
	pfile = fopen(filename , "w");
	if (pfile==NULL){
		printf("Error : we cannot create the file %s\n", filename);
		printf("%s\n", strerror(errno));
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
			if (graph[i][j].link> GRAPH_LINK_NO){
				degree++;

				//Coordinates of the source and destination of link
				x1 = (int)(positions[i].x / MAX_X * GRAPHIC_XMAX);
				y1 = (int)(positions[i].y / MAX_Y * GRAPHIC_YMAX);
				
				x2 = (int)(positions[j].x / MAX_X * GRAPHIC_XMAX);
				y2 = (int)(positions[j].y / MAX_Y * GRAPHIC_YMAX);
				
				depth		= 4;
				thickness	= graph[i][j].thickness;
				if (thickness >= 2)
					linestyle = SOLID;
				else
					linestyle = DASHED;
						
				fprintf(pfile, "#%d-%d (type %d)\n3 2 %d %d %d 0 %d -1 -1 1.0 0 0 0 2\n			%d %d %d %d\n			0.000 0.000\n", i , j , depth , linestyle , thickness,  graph[i][j].color , depth + graph[i][j].color , x1 , y1 , x2 , y2);
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

	int				i;
	int				nb_nodes = get_nb_nodes();
	//infos
	int				*states;
	pos_struct		*positions;
	graph_struct 	**graph;
	//object identification
	Objid			*node_ids;
	Objid			*mac_ids;
	
	//initialization
	cmac_tools_vars_get(&states, &node_ids, &mac_ids, &positions);	
	
	//graph
	graph = cmac_tools_graph_construct(graph, mac_ids);
	
	//now,plot it!
	tools_fig_write_xfig_file(graph, positions, states);
	
	//memory release
	op_prg_mem_free(mac_ids);
	op_prg_mem_free(states);
	op_prg_mem_free(node_ids);
	op_prg_mem_free(positions);
	for(i=0; i<nb_nodes; i++)
		op_prg_mem_free(graph[i]);
	op_prg_mem_free(graph);

	FOUT;
}
