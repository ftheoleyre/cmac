#!/bin/bash

nohup nice -n 20 ./cmac_sim.sh grid7x7_load &
nohup nice -n 20 ./cmac_sim.sh grid7x7_interf &
nohup nice -n 20 ./cmac_sim.sh grid7x7_branches &

