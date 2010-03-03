#!/bin/bash

nohup nice -n 20  ./cmac_sim.sh grid7x7_load &
nohup nice -n 20 ./cmac_sim.sh grid7x7_ctrhop &
nohup nice -n 20 ./cmac_sim.sh grid7x7_branches &
nohup nice -n 20 ./cmac_sim.sh grid7x7_privtime &
nohup nice -n 20 ./cmac_sim.sh grid7x7_static-dynamic &

