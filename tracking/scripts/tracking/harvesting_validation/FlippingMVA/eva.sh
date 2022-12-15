#!/bin/bash
##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This file meant to be the instruction to retrain/apply/evaluate the MVAs for the flip and refit
# 1st MVA: takes all the `low level` variables (eg: ndf, cdc hits number, pxd hits etc) as the input to selecte the RecoTracks to be flipped
# 2nd MVA: takes the fitResults from original Tracks and flipped Tracks as input to decide wich one to keep


save_path="./" 

# --- for 1st MVA .... 
#step 1: generating the ntuples for training and testing
#     (the flipping is off) 
#     mc variables like charge_turth are also saved for validation 
#bsub -q s -oo ${save_path}train_1mva_0.log basf2 BBbar_FlipAndRefitTest.py  -- -n 5000 --exp 1003 --ranseed 23 --output_file_mva ${save_path}train_1stmva.root --num 1
#bsub -q s -oo ${save_path}test_1mva_0.log basf2 BBbar_FlipAndRefitTest.py -- -n 5000 --exp 1003 --ranseed 24 --output_file_mva ${save_path}test_1stmva.root --num 1

#step 2: training 1st mva, please check the variables list and configuration of the fastBDT before running it
#basf2 tracking/flipAndRefitMVAs/training_1st_flipping_mva.py

#setp 3: evaluation, also please specify the root path  
#basf2_mva_evaluate.py -id localdb/dbstore_Weightfile_rev_0b43a0.root -tree data -train train.root -data test.root -o validation.root

# --- for 2nd MVA ---
#step 1:  generating the ntuples for training and testing
#     (need to comment the FlippedRecoTracksMerger module)
#bsub -q s -oo ${save_path}train_2ndmva_0.log basf2 BBbar_FlipAndRefitTest.py -- -n 5000 --exp 1003 --ranseed 122 --output_file_mva ${save_path}train_2ndmva.root --num 2  --flip_recoTrack True
#bsub -q s -oo ${save_path}test_2ndmva_0.log basf2 BBbar_FlipAndRefitTest.py -- -n 5000 --exp 1003 --ranseed 121 --output_file_mva ${save_path}test_2ndmva.root --num 2  --flip_recoTrack True

#step 2: training 
#basf2 tracking/flipAndRefitMVAs/training_2nd_flipping_mva.py

#setp 3: evaluation using basf2 script: same way as 1st MVA



# using local weight file
# modifying add_flipping_of_recoTracks in tracking/scripts/tracking/path_utils.py 
#   
#       path.add_module("FlipQuality", recoTracksStoreArrayName=reco_tracks,
#                       identifier='TRKTrackFlipAndRefit_MVA1_weightfile',  -> to the local weight file 
#                       indexOfFlippingMVA=1).set_name("FlipQuality_1stMVA")
# note: if new variables added in the mva, make sure the /tracking/trackFitting/trackQualityEstimator/variableExtractors/include/FlipRecoTrackExtractor.h also updated  
