#!/bin/bash

p=2 # Number of processes to use for event-looping, also number of single process runs for MuMu and bMuons

n1=100 # Cosmics with no magnetic field
n2=100 # Cosmics with magnetic field
n3=50  # MuMu pairs from IP (QED)
n4=20  # Single muons from BBar events

basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleCosmicsB0.py --experiment 1 --run 1 -n $n1 -o DST_cosmicsB0_exp1run1.root -p $p -l ERROR  &
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleCosmics.py --experiment 2 --run 1 -n $n2 -o DST_cosmics_exp2run1.root -p $p -l ERROR  &

# problems in parallel processing -> each run with one process for bMuons and MuMu

for run in $(seq 1 1 $p)
do
  n=$(( $n3 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleMuMu.py --experiment 3 --run $run -n $n -o DST_mumu_exp3run$run.root -l ERROR  &
  n=$(( $n4 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleBMuons.py --experiment 4 --run $run -n $n -o DST_bMuons_exp4run$run.root -l ERROR  &
done

wait

basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/2_misalignVXDSystematics.py

basf2 ${BELLE2_LOCAL_DIR}/alignment/tools/millepede_calibration.py collect VXDAlignment -i "DST_cosmicsB0_*.root" -o cosmicsB0.root -p $p -l ERROR  -- --magnet-off  &
basf2 ${BELLE2_LOCAL_DIR}/alignment/tools/millepede_calibration.py collect VXDAlignment -i "DST_cosmics_*.root" -o cosmics.root -p $p -l ERROR   &
basf2 ${BELLE2_LOCAL_DIR}/alignment/tools/millepede_calibration.py collect VXDAlignment -i "DST_mumu_*.root" -o mumu.root -p $p -l ERROR   &
basf2 ${BELLE2_LOCAL_DIR}/alignment/tools/millepede_calibration.py collect VXDAlignment -i "DST_bMuons_*.root" -o bMuons.root -p $p -l ERROR  &

wait

basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/4_merge.py -i cosmicsB0.root -i cosmics.root -i mumu.root -i bMuons.root -o merged.root

basf2 ${BELLE2_LOCAL_DIR}/alignment/tools/millepede_calibration.py calibrate VXDAlignment -i merged.root