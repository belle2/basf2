#!/bin/bash

n=100000
p=20

basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleCosmicsB0.py --experiment 1 --run 1 -n $n -o DST_cosmicsB0_exp1run1.root -p $p -l ERROR &
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleCosmics.py --experiment 2 --run 1 -n $n -o DST_cosmics_exp2run1.root -p $p -l ERROR &
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleMuMu.py --experiment 3 --run 1 -n $n -o DST_mumu_exp3run1.root -p $p -l ERROR &
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleBMuons.py --experiment 4 --run 1 -n $n -o DST_bMuons_exp4run1.root -p $p -l ERROR

wait

basf2 ${BELLE2_LOCAL_DIR}/alignment/tools/millepede_calibration.py collect VXDAlignment -i "DST_cosmicsB0_*.root" -o cosmicsB0.root -p $p -l ERROR -- --magnet-off &
basf2 ${BELLE2_LOCAL_DIR}/alignment/tools/millepede_calibration.py collect VXDAlignment -i "DST_cosmics_*.root" -o cosmics.root -p $p -l ERROR &
basf2 ${BELLE2_LOCAL_DIR}/alignment/tools/millepede_calibration.py collect VXDAlignment -i "DST_mumu_*.root" -o mumu.root -p $p -l ERROR &
basf2 ${BELLE2_LOCAL_DIR}/alignment/tools/millepede_calibration.py collect VXDAlignment -i "DST_bMuons_*.root" -o bMuons.root -p $p -l ERROR

wait

basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/4_merge.py -i cosmicsB0.root -i cosmics.root -i mumu.root -i bMuons.root -o merged.root

basf2 ${BELLE2_LOCAL_DIR}/alignment/tools/millepede_calibration.py calibrate VXDAlignment -i merged.root