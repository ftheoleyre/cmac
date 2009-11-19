#!/bin/bash

#verify the nb of parameters
if [ $# -ne 3 ]
then
	echo "usage $0 mac_file seed_stat dir_destination"
	echo "currently, $# arguments";
	exit
fi

#the result file
MAC_FILE=$1
SEED=$2

cat $MAC_FILE

#Algos
MAC_LAYER=`cat $MAC_FILE |  grep "Mac layer id"  |cut -d ":" -f 2`
KTREE_ALGO=`cat $MAC_FILE |  grep "Ktree algorithm"  |cut -d ":" -f 2`


#environment
NB_NODES=`cat $MAC_FILE |  grep "Number of nodes"  |cut -d ":" -f 2`
SIM_LENGTH=`cat $MAC_FILE |  grep "X_MAX"  |cut -d ":" -f 2`
INTER_PK_T=`cat $MAC_FILE |  grep "Inter Packet Time"  |cut -d ":" -f 2`
DEGREE=`cat $MAC_FILE |  grep "Average degree"  |cut -d ":" -f 2`

#MAC parameters
NB_CHANNELS=`cat $MAC_FILE |  grep "Number of channels"  |cut -d ":" -f 2`
NB_BRANCHES=`cat $MAC_FILE |  grep "Number of branches"  |cut -d ":" -f 2`
CTR_HOPS=`cat $MAC_FILE |  grep "CTR hop spacing"  |cut -d ":" -f 2`

#MAC perfs
DRATIO_DATA=`cat $MAC_FILE |  grep "Delivery Ratio (data)"  |cut -d ":" -f 2`
DELAY_AVG=`cat $MAC_FILE |  grep "Average Delay (in s)"  |cut -d ":" -f 2`
DELAY_STDDEV=`cat $MAC_FILE |  grep "Delay Standard Deviation (in s)"  |cut -d ":" -f 2`
JAIN_DRATIO=`cat $MAC_FILE |  grep "Delivery Ratio Jain Index"  |cut -d ":" -f 2`
GOODPUT_MBPS=`cat $MAC_FILE |  grep "Goodput (in Mbps)"  |cut -d ":" -f 2`

#Error detection
NB_PKTS=`cat $MAC_FILE |  grep "Nb packets sent"  |cut -d ":" -f 2`

# stat file (discard bad networks with a null reliability)
if [ $NB_PKTS \> 0 ]
then
	#file to save results
	FILE_RESULT_AGGREG="nodes=`echo $NB_NODES`_length=`echo $SIM_LENGTH`_algo=`echo $MAC_LAYER`-`echo $KTREE_ALGO`_nbchannels=`echo $NB_CHANNELS`_nbbranches=`echo $NB_BRANCHES`_CTRhops=`echo $CTR_HOPS`.txt"
	echo $FILE_RESULT_AGGREG
	
	#all the stats
	echo "$NB_NODES	$SIM_LENGTH	$INTER_PK_T	$DEGREE	$NB_CHANNELS	$NB_BRANCHES	$DRATIO_DATA	$DELAY_AVG	$DELAY_STDDEV	$JAIN_DRATIO	$GOODPUT_MBPS	$SEED" >> $FILE_RESULT_AGGREG
fi


#delete unused temporary files
#rm $FILE_OUT

# Parameters list: 
#NB_NODES=1`
#SEED=2
