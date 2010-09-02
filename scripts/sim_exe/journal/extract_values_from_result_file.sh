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
BEB=`cat $MAC_FILE |  grep "Exponential backoff"  |cut -d ":" -f 2`
PRIVTIME=`cat $MAC_FILE |  grep "Max privileged duration"  |cut -d ":" -f 2`

#MAC parameters
NB_CHANNELS=`cat $MAC_FILE |  grep "Number of channels"  |cut -d ":" -f 2`
NB_BRANCHES=`cat $MAC_FILE |  grep "Number of branches"  |cut -d ":" -f 2`
CTR_HOPS=`cat $MAC_FILE |  grep "CTR hop spacing"  |cut -d ":" -f 2`

#MAC perfs
DRATIO_DATA=`cat $MAC_FILE |  grep "Delivery Ratio (data)"  |cut -d ":" -f 2`
DELAY_AVG=`cat $MAC_FILE |  grep "Average Delay (in s)"  |cut -d ":" -f 2`
DELAY_STDDEV=`cat $MAC_FILE |  grep "Delay Standard Deviation (in s)"  |cut -d ":" -f 2`
DELAY_MAX=`cat $MAC_FILE |  grep "Max Delay (in s)"  |cut -d ":" -f 2`
JAIN_DRATIO=`cat $MAC_FILE |  grep "Delivery Ratio Jain Index"  |cut -d ":" -f 2`
GOODPUT_MBPS=`cat $MAC_FILE |  grep "Goodput (in Mbps)"  |cut -d ":" -f 2`
RLENGTH=`cat $MAC_FILE |  grep "Average route length"  |cut -d ":" -f 2`

#Overhead
OH_BITS_RTS=`cat $MAC_FILE |  grep "RTS"  | grep "bits"	| cut -d ":" -f 2`
OH_BITS_CTS=`cat $MAC_FILE |  grep "CTS"  | grep "bits"	| cut -d ":" -f 2`
OH_BITS_CTR=`cat $MAC_FILE |  grep "CTR"  | grep "bits"	| cut -d ":" -f 2`
OH_BITS_CTR_END=`cat $MAC_FILE |  grep "CT-END"  | grep "bits"	| cut -d ":" -f 2`
OH_BITS_DATA=`cat $MAC_FILE |  grep "DATA-UNI"  | grep "bits"	| cut -d ":" -f 2`
OH_BITS_MULTI=`cat $MAC_FILE |  grep "DATA-MULTI"  | grep "bits"	| cut -d ":" -f 2`
OH_BITS_ACK=`cat $MAC_FILE |  grep "ACK"  | grep "bits"	| cut -d ":" -f 2`
OH_BITS_HELLO=`cat $MAC_FILE |  grep "HELLO"  | grep "bits"	| cut -d ":" -f 2`
OH_BITS_CTR_ACK=`cat $MAC_FILE |  grep "CT-CK"  | grep "bits"	| cut -d ":" -f 2`
OH_PK_RTS=`cat $MAC_FILE |  grep "RTS"  | grep "pkts"	| cut -d ":" -f 2`
OH_PK_CTS=`cat $MAC_FILE |  grep "CTS"  | grep "pkts"	| cut -d ":" -f 2`
OH_PK_CTR=`cat $MAC_FILE |  grep "CTR"  | grep "pkts"	| cut -d ":" -f 2`
OH_PK_CTR_END=`cat $MAC_FILE |  grep "CT-END"  | grep "pkts"	| cut -d ":" -f 2`
OH_PK_DATA=`cat $MAC_FILE |  grep "DATA-UNI"  | grep "pkts"	| cut -d ":" -f 2`
OH_PK_MULTI=`cat $MAC_FILE |  grep "DATA-MULTI"  | grep "pkts"	| cut -d ":" -f 2`
OH_PK_ACK=`cat $MAC_FILE |  grep "ACK"  | grep "pkts"	| cut -d ":" -f 2`
OH_PK_HELLO=`cat $MAC_FILE |  grep "HELLO"  | grep "pkts"	| cut -d ":" -f 2`
OH_PK_CTR_ACK=`cat $MAC_FILE |  grep "CT-CK"  | grep "pkts"	| cut -d ":" -f 2`

#Error detection
NB_PKTS=`cat $MAC_FILE |  grep "Nb packets sent"  |cut -d ":" -f 2`
NB_BITS=`cat $MAC_FILE |  grep "Nb bits sent"  |cut -d ":" -f 2`
NB_BITS_RCVD=`cat $MAC_FILE |  grep "Nb bits received"  |cut -d ":" -f 2`

# stat file (discard bad networks with a null reliability)
#if [ $NB_PKTS \> 0 ]
#then
	#file to save results
	FILE_RESULT_AGGREG="$3/nodes=`echo $NB_NODES`_algo=`echo $MAC_LAYER`-`echo $KTREE_ALGO`_nbchannels=`echo $NB_CHANNELS`_beb=`echo $BEB`.txt"
	echo $FILE_RESULT_AGGREG
	
	#all the stats
	echo "$NB_NODES	$SIM_LENGTH	$INTER_PK_T	$PRIVTIME	$DEGREE	$NB_CHANNELS	$NB_BRANCHES	$CTR_HOPS	$DRATIO_DATA	$DELAY_AVG	$DELAY_STDDEV	$DELAY_MAX	$JAIN_DRATIO	$GOODPUT_MBPS	$RLENGTH\
	$SEED	$NB_PKTS	$NB_BITS	$NB_BITS_RCVD\
	$OH_BITS_RTS $OH_BITS_CTS $OH_BITS_CTR $OH_BITS_CTR_END $OH_BITS_DATA $OH_BITS_MULTI $OH_BITS_ACK $OH_BITS_HELLO $OH_BITS_CTR_ACK\
	$OH_PK_RTS $OH_PK_CTS $OH_PK_CTR $OH_PK_CTR_END $OH_PK_DATA $OH_PK_MULTI $OH_PK_ACK $OH_PK_HELLO $OH_PK_CTR_ACK\
	" >> $FILE_RESULT_AGGREG
	
	#debug
	echo "$NB_NODES	$SIM_LENGTH	$INTER_PK_T	$PRIVTIME	$DEGREE	$NB_CHANNELS	$NB_BRANCHES	$CTR_HOPS	$DRATIO_DATA	$DELAY_AVG	$DELAY_STDDEV	$DELAY_MAX	$JAIN_DRATIO	$GOODPUT_MBPS	$RLENGTH\
	$SEED	$NB_PKTS	$NB_BITS	$NB_BITS_RCVD\
	$OH_BITS_RTS $OH_BITS_CTS $OH_BITS_CTR $OH_BITS_CTR_END $OH_BITS_DATA $OH_BITS_MULTI $OH_BITS_ACK $OH_BITS_HELLO $OH_BITS_CTR_ACK\
	$OH_PK_RTS $OH_PK_CTS $OH_PK_CTR $OH_PK_CTR_END $OH_PK_DATA $OH_PK_MULTI $OH_PK_ACK $OH_PK_HELLO $OH_PK_CTR_ACK"
#fi

#1 NB_NODES	
#2 $SIM_LENGTH	
#3 $INTER_PK_T	
#4 $PRIVTIME	
#5 $DEGREE	
#6 $NB_CHANNELS	
#7 $NB_BRANCHES	
#8 $CTR_HOPS	
#9 $DRATIO_DATA	
#10 $DELAY_AVG	
# $DELAY_STDDEV	
# $DELAY_MAX	
# $JAIN_DRATIO	
# $GOODPUT_MBPS	
#15 $RLENGTH	
# $SEED
# NB_PKTS
# NB_BITS
# NB_BITS_RCVD
#20 OH_BITS_RTS 
# OH_BITS_CTS 
# OH_BITS_CTR 
# OH_BITS_CTR_END 
# OH_BITS_DATA 
#25 OH_BITS_MULTI 
# OH_BITS_ACK 
# OH_BITS_HELLO 
# OH_BITS_CTR_ACK
# OH_PK_RTS 
#30 OH_PK_CTS 
# OH_PK_CTR 
# OH_PK_CTR_END 
# OH_PK_DATA 
# OH_PK_MULTI 
#35 OH_PK_ACK 
# OH_PK_HELLO 
# OH_PK_CTR_ACK




# 1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41
# A  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z  aa ab ac ad ae af ag ah ai aj ak al am an ao
