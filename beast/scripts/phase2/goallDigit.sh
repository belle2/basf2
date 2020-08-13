#!/bin/bash

# submits jobs to digitize and analyze beast MC
# specify subfolder in /group/belle2/BGcampaigns/g4sim/ and number of jobs to be submitted, each jobs processes 10 input files  
# output histograms are stored into /group/belle2/BGcampaigns/g4sim/"folder"/beast_hist/ 

# == que list ==
# s <6h
# l <48h
# b_a <48h
# b_l <720h

njob=40 # number of files in folder / 10! 10 files are processed in one job 
folder=phase2.1.3_june_Touschek/EM_0

for f in Touschek Coulomb Brems; do
    for g in LER HER; do
	h=0;
	while [ $h -lt $njob ]; do 
	    bsub -q s "basf2 beast/scripts/phase2/Digitize.py $f $g $h $folder "
	    h=$(($h+1))
	done
    done
done




