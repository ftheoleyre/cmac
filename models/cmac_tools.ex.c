/*
 *  cmac_tools.c
 *  
 *  Created by Fabrice Theoleyre on 25/08/09.
 *  Copyright 2009 CNRS / LIG. All rights reserved.
 *
 */


#include	"cmac_process.h"
#include 	"cmac_tools.h"


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
