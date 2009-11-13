MOLECULAR / MMAC / 802.11 simulation

To execute one set of simulations, you just have to execute ./TOPO_sim.sh to run 20 different seeds for TOPO

For instance, ./3pairs_sim.sh runs all the scenarios associated to the 3 pairs


To create a new scenario, change parameter values, you have to know the correct organization:
1/ params_$TOPO.sh: contains all the values of the different parameters. If you want to add a parameter type, you have to also modify extract_value.sh and topo_sim.sh
2/ topo_sim.sh: create the correct calls to op_runsim, with all arguments, and copy the results in the correct location
3/ extract_values_from_file_result.sh: from the result file created by opnet, extracts all the stats to be put in a common file (tabular)
Thus, creating a new scenario just requires to create the files containing the parameters (according to 1/) and of course the associated opnet scenario, etc (with the compliant pattern)