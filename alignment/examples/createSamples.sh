#!/bin/bash

p=10 # Number of runs to generate for each sample

n1=80000 # Cosmics with no magnetic field
n2=40000 # Cosmics with magnetic field
n3=10000 # MuMu pairs from IP (QED)
n4=10000 # Single muons from BBar events

rm localdb -r -f
mkdir localdb
echo "" > localdb/database.txt

for run in $(seq 1 1 $p)
do
  n=$(( $n1 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleCosmicsB0.py --experiment 1 --run $run -n $n -o DST_cosmicsB0_exp1run$run.root -l ERROR  &
  n=$(( $n2 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleCosmics.py   --experiment 2 --run $run -n $n -o DST_cosmics_exp2run$run.root   -l ERROR  &
  n=$(( $n3 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleMuMu.py      --experiment 3 --run $run -n $n -o DST_mumu_exp3run$run.root   -l ERROR  &
  n=$(( $n4 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleBMuons.py    --experiment 4 --run $run -n $n -o DST_bMuons_exp4run$run.root -l ERROR  &
done

wait

basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/2_misalignVXDSystematics.py

for run in $(seq 1 1 $p)
do
  n=$(( $n1 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/collectCosmicsB0.py -i DST_cosmicsB0_exp1run$run.root -o collect_cosmicsB0_exp1run$run.root -l ERROR  &
  n=$(( $n2 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/collectCosmics.py   -i DST_cosmics_exp2run$run.root   -o collect_cosmics_exp2run$run.root   -l ERROR  &
  n=$(( $n3 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/collectMuMu.py      -i DST_mumu_exp3run$run.root      -o collect_mumu_exp3run$run.root   -l ERROR  &
  n=$(( $n4 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/collectBMuons.py    -i DST_bMuons_exp4run$run.root    -o collect_bMuons_exp4run$run.root -l ERROR  &
done
wait

basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/4_merge.py -i "collect_*.root" -o merged.root

basf2 ${BELLE2_LOCAL_DIR}/alignment/tools/millepede_calibration.py calibrate VXDAlignment -i merged.root
