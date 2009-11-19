#!/bin/ksh

#verify the nb of parameters
if [ $# -ne 1 ]
then
	echo "usage $0 topology_name"
	exit
fi

#topology name
TOPO=$1
FILE_PARAMS="params_`echo $TOPO`.txt"



#--------------------------------------
#		ERRORS
#--------------------------------------

if [ -e $FILE_PARAMS ]
then
	echo "Parameters are extracted from the file $FILE_PARAMS"
else
	echo "$FILE_PARAMS does not exist: you MUST specify parameters values"
	exit
fi



#--------------------------------------
#		PARAMETERS
#--------------------------------------
DURATION=90

# traffic
INTER_PK_TIME_LIST=`cat $FILE_PARAMS | grep "^[^#]" | grep Packet_Interarrival_Time| cut -d '=' -f 2`
echo "INTER PK TIMES" $INTER_PK_TIME_LIST
SINK_ADDR=`cat $FILE_PARAMS | grep "^[^#]" | grep sink_address | cut -d '=' -f 2`
echo "Sink Address" $SINK_ADDR

#MAC layer
MAC_LAYER_LIST=`cat $FILE_PARAMS | grep "^[^#]" | grep Mac_layer | cut -d '=' -f 2`
echo "MAC_LAYER" $MAC_LAYER_LIST

#Cmac-parameters
KTREE_ALGO_LIST=`cat $FILE_PARAMS | grep "^[^#]" | grep Ktree_algo | cut -d '=' -f 2`
echo "KTREE ALGO IDS" $KTREE_ALGO_LIST
NB_CHANNELS_LIST=`cat $FILE_PARAMS | grep "^[^#]" | grep Nb_channels | cut -d '=' -f 2`
echo "NB_CHANNELS" $NB_CHANNELS_LIST
NB_BRANCHES_LIST=`cat $FILE_PARAMS | grep "^[^#]" | grep Nb_branches | cut -d '=' -f 2`
echo "NB_BRANCHES" $NB_BRANCHES_LIST
MAX_PRIV_DURATION_LIST=`cat $FILE_PARAMS | grep "^[^#]" | grep Max_priv_duration | cut -d '=' -f 2`
echo "MAX_PRIV_DURATION" $MAX_PRIV_DURATION_LIST
CTR_HOP_SPACING_LIST=`cat $FILE_PARAMS | grep "^[^#]" | grep CTR_hop_spacing | cut -d '=' -f 2`
echo "CTR_HOP_SPACING" $CTR_HOP_SPACING_LIST

#Positions
X_MAX_LIST=`cat $FILE_PARAMS | grep "^[^#]" | grep X_MAX | cut -d '=' -f 2`
echo "XMAX" $X_MAX_LIST
POS_METHOD=`cat $FILE_PARAMS | grep "^[^#]" | grep Positions_Method | cut -d '=' -f 2`

#name for the file
SIMULATION_NAME=$TOPO
echo "SIMULATION NAME" $SIMULATION_NAME
NET_NAME=`cat $FILE_PARAMS | grep "^[^#]" | grep Net_name | cut -d '=' -f 2`
echo "SIMULATION NAME" $NET_NAME

#ARGS
ARGS="-net_name cmac-$NET_NAME -ef cmac_generic.ef -noprompt"
ARGS="$ARGS -duration $DURATION"
ARGS="$ARGS -Positions_Method $POS_METHOD"



#--------------------------------------
#			SIMULATION EXECUTION
#--------------------------------------

#repeats these simulations with 20 different seeds
i=1
while [ $i -le "20" ]
do
	for MAX_PRIV_DURATION in $MAX_PRIV_DURATION_LIST;
	do
		for INTER_PK_TIME in $INTER_PK_TIME_LIST;
		do
			for KTREE_ALGO in $KTREE_ALGO_LIST;
			do
				for X_MAX in $X_MAX_LIST;
				do
					for MAC_LAYER in $MAC_LAYER_LIST;
					do
					
						#only one branch and one channel for IEEE 802.11
						if [ $MAC_LAYER -eq 2 ]
						then
							NB_CHANNELS_LIST_TMP="1"
							NB_BRANCHES_LIST_TMP="1"
							CTR_HOP_SPACING_LIST_TMP="1"
						else
							NB_CHANNELS_LIST_TMP="$NB_CHANNELS_LIST"
							NB_BRANCHES_LIST_TMP="$NB_BRANCHES_LIST"
							CTR_HOP_SPACING_LIST_TMP="$CTR_HOP_SPACING_LIST"
						fi						
						
						for NB_CHANNELS in $NB_CHANNELS_LIST_TMP;
						do
							for NB_BRANCHES in $NB_BRANCHES_LIST_TMP;
							do
								for CTR_HOP_SPACING in $CTR_HOP_SPACING_LIST_TMP;
								do
							
							
										#one random seed
										SEED=`hexdump -n4 -e\"%u\" /dev/random`
										RES_TMP_DIR="/tmp/debug/$TOPO/$SEED"
										mkdir -p $RES_TMP_DIR

										#simulation arguments for this run
										CMD="op_runsim -mac_layer $MAC_LAYER -seed $SEED -Result_Directory $RES_TMP_DIR"
										CMD="$CMD -interpacket_time $INTER_PK_TIME"
										CMD="$CMD -ktree_algo $KTREE_ALGO"
										CMD="$CMD -max_priv_duration $MAX_PRIV_DURATION"
										CMD="$CMD -nb_branches $NB_BRANCHES -nb_channels $NB_CHANNELS"
										CMD="$CMD -CTR_hop_spacing $CTR_HOP_SPACING"
										CMD="$CMD -X_MAX $X_MAX"
										CMD="$CMD -sink_address $SINK_ADDR"
										CMD="$CMD $ARGS"				
										echo $CMD " > " $RES_TMP_DIR
										$CMD
			
										#I must move the results in the correct place
										RES_DIR="../results/$SIMULATION_NAME/$MAC_LAYER/"
										echo $RES_DIR
										if [ ! -d $RES_DIR ]
										then
											mkdir -p $RES_DIR
										fi
						
										#copy the resulting files
										mv $RES_TMP_DIR $RES_DIR
						
										#extract the correct stats
										MAC_FILE="`ls $RES_DIR/$SEED/*stats-nodes*`"
										CMD="./extract_values_from_result_file.sh $MAC_FILE $SEED $RES_DIR"
										echo $CMD; $CMD
										#rm -rf $RES_TMP_DIR
										
										
								done
							done
						done
					done
				done
			done
		done
	done
done
		
