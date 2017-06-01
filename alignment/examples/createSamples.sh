#!/bin/bash

# Number of processes (runs) per sample (single core):
p=${1:-0}

# Total event size of the samples:
N1=${2:-400000}   # Cosmics with no magnetic field
N2=${3:-200000}   # Cosmics with magnetic field
N3=${4:-100000}   # MuMu pairs from IP (QED)
N4=${5:-100000}   # Single muons from BBar events

# Run number to start with:
first_run=${6:-1}
last_run=$(( $p + $first_run - 1 ))

# f1=1
# f2=1
# f3=9
# f4=3


echo ""
echo " Generation script for alignment data samples. (mainly for KEKCC, DESY ...)"
echo ""
echo " Samples' configuration:"
echo ""
echo "  exp  |   sample  | #events  "
echo " -----------------------------"
echo "   1   | CosmicsB0 | $N1      "
echo "   2   |   Cosmics | $N2      "
echo "   3   |      MuMu | $N3      "
echo "   4   |     Muons | $N4      "
echo " -----------------------------"
echo ""

if test ! $p -gt 0; then
  echo " Usage:"
  echo ""
  echo " $ ./createSamples.sh num_runs [N_CosmicsB0 [N_Cosmics [N_MuMu [N_Muons [first_run=1]]]]]"
  echo ""
  echo " - Each sample for generation (disable with N_SAMPLE=0) will start num_runs processes"
  echo "   with N_SAMPLE/num_runs events."
  echo " - All jobs will start at once. Default settings should provide 'optimal' sample."
  echo " - Running without arguments or num_runs=0 will display default settings and this help."  
  echo " - Use first_run to set the starting number for the run list. Otherwise it will start with run 1."
  echo ""
  echo " Please specify number of runs > 0 to generate. Exiting."
  return
fi  

n1=$(( $N1 / $p ))
n2=$(( $N2 / $p ))
n3=$(( $N3 / $p ))
n4=$(( $N4 / $p ))

N=$(( ($n1 + $n2 + $n3 + $n4) * $p ))

echo " Number of runs to generate per sample: $p"
echo " Run numbers from $first_run to $last_run"

if test $N -gt 0; then
  echo " Total number of events for generation: $N"
  echo ""  
else
  echo " Please lower number of runs or increase number of events in some sample to have at least one event per run. Exiting."
  return
fi  

for run in $(seq $first_run 1 $last_run)
do  
  if test $n1 -gt 0; then
    echo ${BELLE2_LOCAL_DIR}/alignment/examples/sampleCosmicsB0.py --experiment 1 --run $run -n $n1 -o DST_cosmicsB0_exp1run$run.root -l ERROR &
  fi
  
  if test $n2 -gt 0; then
    echo ${BELLE2_LOCAL_DIR}/alignment/examples/sampleCosmics.py   --experiment 2 --run $run -n $n2 -o DST_cosmics_exp2run$run.root   -l ERROR &
  fi
  
  if test $n3 -gt 0; then
    echo ${BELLE2_LOCAL_DIR}/alignment/examples/sampleMuMu.py      --experiment 3 --run $run -n $n3 -o DST_mumu_exp3run$run.root      -l ERROR &
  fi
  
  if test $n4 -gt 0; then
    echo ${BELLE2_LOCAL_DIR}/alignment/examples/sampleBMuons.py    --experiment 4 --run $run -n $n4 -o DST_bMuons_exp4run$run.root    -l ERROR &
  fi
  
done

wait

echo ""
echo " Finished sample generation."
echo ""
