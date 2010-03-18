#!/bin/bash

#grid 7x7
nohup nice -n 20  ./cmac_sim.sh grid7x7_load &

#tuning parameter's values
nohup nice -n 20 ./cmac_sim.sh grid7x7_ctrhop &
nohup nice -n 20 ./cmac_sim.sh grid7x7_branches &
nohup nice -n 20 ./cmac_sim.sh grid7x7_privtime &

#proof of correctness for the k-tree algo
nohup nice -n 20 ./cmac_sim.sh grid7x7_static-dynamic &

# multichannel feature
nohup nice -n 20 ./cmac_sim.sh grid7x7_load_channels &

#random disks
nohup nice -n 20 ./cmac_sim.sh random80_load &
nohup nice -n 20 ./cmac_sim.sh random60_load &
nohup nice -n 20 ./cmac_sim.sh random60_density &
