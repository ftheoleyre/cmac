/*
 *  tools.h
 *  
 *  Created by Fabrice Theoleyre on 22/10/08.
 *  Copyright 2008 CNRS / LIG. All rights reserved.
 *
 */

#ifndef _TOOLS_FIG_H_
#define _TOOLS_FIG_H_

#include <stdio.h>
#include <opnet.h>

#include "cmac.h"



//-------------------------  FIGURES  -----------------------------------

//To generate a .fig 
void tools_fig_generate();




//------------------------------------
//		FIGURES
//------------------------------------

//constants for the figures
#define		GRAPHIC_MAX					10000
#define		GRAPHIC_RADIUS				40
#define		GRAPHIC_POLICE_SIZE			10
#define		GRAPHIC_SHIFT_X				60
#define		GRAPHIC_SHIFT_Y				0

#define		BLACK						0
#define		WHITE						7
#define		GRAY						32
#define		RED							4

#define		SOLID						0
#define		DASHED						1

//constants for the graph
#define		GRAPH_LINK_NO				0
#define		GRAPH_LINK_RADIO			1


#endif
