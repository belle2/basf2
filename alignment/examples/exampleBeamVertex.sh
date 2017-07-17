#!/bin/bash

p=1 # Number of runs to generate for each sample

n1=100 # Num events of Cosmics with no magnetic field
n2=100 # Num events of Cosmics with magnetic field
n3=100 # Num events of MuMu pairs from IP (QED)
n4=100 # Num events of Single muons from BBar events

# rm localdb -r -f
# mkdir localdb
# echo "" > localdb/database.txt

for run in $(seq 1 1 $p)
do
  n=$(( $n1 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleCosmicsB0.py --experiment 1 --run $run -n $n -o DST_cosmicsB0_exp1run$run.root -l ERROR
  n=$(( $n2 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleCosmics.py   --experiment 2 --run $run -n $n -o DST_cosmics_exp2run$run.root   -l ERROR
  n=$(( $n3 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleMuMu.py      --experiment 3 --run $run -n $n -o DST_mumu_exp3run$run.root   -l ERROR
  n=$(( $n4 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleBMuons.py    --experiment 4 --run $run -n $n -o DST_bMuons_exp4run$run.root -l ERROR
done


# Mis-align beam vertex in reconstruction:
# (See the script configuration what is actually done)
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/misalignYing.py

# Or one could misalign VXD as well... see the script configuration what is actually done
# basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/2_misalignVXDSystematics.py

for run in $(seq 1 1 $p)
do
  n=$(( $n1 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/recoCosmicsB0.py -i DST_cosmicsB0_exp1run$run.root -o reco_cosmicsB0_exp1run$run.root -l ERROR
  n=$(( $n2 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/recoCosmics.py   -i DST_cosmics_exp2run$run.root   -o reco_cosmics_exp2run$run.root   -l ERROR
  n=$(( $n3 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/recoMuMu.py      -i DST_mumu_exp3run$run.root      -o reco_mumu_exp3run$run.root   -l ERROR
  n=$(( $n4 / $p ))
  basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/recoBMuons.py    -i DST_bMuons_exp4run$run.root    -o reco_bMuons_exp4run$run.root -l ERROR
done


for run in $(seq 1 1 $p)
do
  n=$(( $n1 / $p ))
  millepede_calibration VXDHalfShells collect --tracks RecoTracks --magnet-off -- -i reco_cosmicsB0_exp1run$run.root -o collect_cosmicsB0_exp1run$run.root -l ERROR
  n=$(( $n2 / $p ))
  millepede_calibration VXDHalfShells collect --tracks RecoTracks -- -i reco_cosmics_exp2run$run.root -o collect_cosmics_exp2run$run.root -l ERROR
  n=$(( $n3 / $p ))
  millepede_calibration VXDHalfShells collect --primary-vertices "Z0:mumu" -- -i reco_mumu_exp3run$run.root -o collect_mumu_exp3run$run.root -l INFO
  n=$(( $n4 / $p ))
  millepede_calibration VXDHalfShells collect -- -i reco_bMuons_exp4run$run.root -o collect_bMuons_exp4run$run.root -l ERROR
done


basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/merge.py -i "collect_*.root" -o merged.root

millepede_calibration VXDHalfShells calibrate -i merged.root 