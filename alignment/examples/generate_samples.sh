#!/bin/bash

# This script generates a sufficient sample for VXD alignment
# with the calibrationFramework.py script.

# If you work in alignment/examples folder,
# you can just run this script (takes some time)
# and then directly calibrationFramework.py.
# Otherwise you have to set properly the folder
# where DST samples are taken from in the
# calibrationFramework.py script.

# Exp=1 for cosmics samples
COSMIC_EXP_NUM=1
# Exp=2 for PGun collison samples
IP_EXP_NUM=2

N_COSMIC_RUNS=10
N_IP_RUNS=10

N_COSMIC_EVENTS=30000
N_IP_EVENTS=10000

for (( run=1; run <= $N_COSMIC_RUNS; ++run ))
do
  basf2 GenDST.py $COSMIC_EXP_NUM $run $N_COSMIC_EVENTS 1
done

for (( run=1; run <= $N_IP_RUNS; ++run ))
do
  basf2 GenDST.py $IP_EXP_NUM $run $N_IP_EVENTS
done

