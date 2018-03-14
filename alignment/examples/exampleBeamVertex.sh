#!/bin/bash
# This script demonstrates some features of Millepede calibration
# in basf2 and how to run it (also with CAF). It
#
# - tries to create an empty local database
# - generates 2 simulated samples for alignment: BBar decays and e+e- -> mu+mu- (with ip+vertex constraint)
# - misaligns one PXD half shell in a local database
# - runs simultaneous alignment of VXD half shells and primary vertex in 3 ways:
#    1) with CAF using the predefined scenario
#    2) with CAF with more control but more code to set up things
#    3) with command line tool "millepede_calibration" based on predefined scenarios, without CAF
#    
# Please note that 3) will actually update the local DB. 1) and 2) have all outputs in own folders

n1=100 # Num events of MuMu pairs from IP (QED)
n2=100 # Num events of Single muons from BBar events

# rm localdb -r -f
mkdir localdb
echo "" > localdb/database.txt

basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleMuMu.py   --experiment 1 --run 1 -n $n1 -o sampleMuMu.root   -l ERROR
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/sampleBMuons.py --experiment 1 --run 2 -n $n2 -o sampleBMuons.root -l ERROR

# Mis-align one PXD half-shell (for example)
# (See the script configuration what is actually done)
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/misalignYing.py


# 1) quick CAF setup with scenarios
basf2 ${BELLE2_LOCAL_DIR}/calibration/examples/caf_vxd_alignment.py sampleMuMu.root sampleBMuons.root


# 2) CAF setup without scenarios
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/caf_vxdbeam.py -i sampleMuMu.root -i sampleBMuons.root


# 3) command line tool (separate collection+calibration)
millepede_calibration VXDHalfShells collect   --components VXDAlignment BeamParameters --tracks --reco --ana -- -i sampleMuMu.root -i sampleBMuons.root -l ERROR
# Output file name name of previous line is used by default in the following (RootOutput.root, but produced by HistoManager)
millepede_calibration VXDHalfShells calibrate --components VXDAlignment BeamParameters
