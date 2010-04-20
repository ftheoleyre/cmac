#!/bin/bash

#classical grid
nohup nice -n 20  ./cmac_sim.sh conf_grid7x7_load &
nohup nice -n 20  ./cmac_sim.sh conf_grid10x10_load &
nohup nice -n 20  ./cmac_sim.sh conf_grid13x13_load &

#tuning parameter's values
nohup nice -n 20 ./cmac_sim.sh conf_grid7x7_privtime &

# multichannel feature
nohup nice -n 20 ./cmac_sim.sh conf_grid7x7_load_channels &
nohup nice -n 20 ./cmac_sim.sh conf_grid13x13_load_channels &

