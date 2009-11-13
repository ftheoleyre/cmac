#!/bin/bash

#verify the nb of parameters
if [ $# -ne 4 ]
then
	echo "usage $0 mac_file seed_stat dir_destination"
	echo "currently, $# arguments";
	exit
fi

#the result file
MAC_FILE=$1
WCDS_FILE=$2
SEED=$3

cat $MAC_FILE
cat $WCDS_FILE

#parameters
NB_NODES=`cat $MAC_FILE |  grep "Nb nodes"  |cut -d ":" -f 2`
SIM_LENGTH=`cat $MAC_FILE |  grep "X Max"  |cut -d ":" -f 2`
INTER_PK_T=`cat $MAC_FILE |  grep "Interarrival"  |cut -d ":" -f 2`
DEGREE=`cat $MAC_FILE |  grep "Average degree"  |cut -d ":" -f 2`
NB_CHANNELS=`cat $MAC_FILE |  grep "Number of channels"  |cut -d ":" -f 2`
NB_BRANCHES`cat $MAC_FILE |  grep "Number of branches"  |cut -d ":" -f 2`


#MAC
DRATIO=`cat $MAC_FILE |  grep "Delivery ratio"  |cut -d ":" -f 2`
DELAY_AVG=`cat $MAC_FILE |  grep "Average Delay (in s)"  |cut -d ":" -f 2`
DELAY_STDDEV=`cat $MAC_FILE |  grep "Delay Standard Deviation (in s)"  |cut -d ":" -f 2`
JAIN_DRATIO=`cat $MAC_FILE |  grep "Delivery Ratio Jain Index"  |cut -d ":" -f 2`
GOODPUT_MBPS=`cat $MAC_FILE |  grep "Goodput (in Mbps)"  |cut -d ":" -f 2`

# stat file (discard bad networks with a null reliability)
if [ $NB_NODES ]
then
	#file to save results
	FILE_RESULT_AGGREG="$4/nodes=`echo $NB_NODES`_length=`echo $SIM_LENGTH`_ktree=`echo $KTREE_ALGO`_nbchannels=`echo $NB_CHANNELS`.txt"
	echo $FILE_RESULT_AGGREG
	
	#all the stats
	echo "$NB_NODES	$SIM_LENGTH	$INTER_PK_T	$DEGREE	$NB_CHANNELS	$NB_BRANCHES	$DEGREE	$DRATIO	$DELAY_AVG	$DELAY_STDDEV	$JAIN_DRATIO	$GOODPUT_MBPS	$SEED" >> $FILE_RESULT_AGGREG
fi

#delete unused temporary files
#rm $FILE_OUT

# Parameters list: 
#NB_NODES=1`
#SEED=2
