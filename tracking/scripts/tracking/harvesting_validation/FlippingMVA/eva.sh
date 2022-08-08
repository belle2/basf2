##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#generating the ttree for training and testing 
# --- for 1st MVA
basf2 BBbar.py  -- -n 1000 --exp 1003 --ranseed 12 --output_file_mva train_1stmva.root --num 1
basf2 BBbar.py  -- -n 1000 --exp 1003 --ranseed 11 --output_file_mva test_1stmva.root --num 1
# --- for 2nd MVA (need to turn the FlippedRecoTracksMerger off)
basf2 BBbar.py  -- -n 1000 --exp 1003 --ranseed 122 --output_file_mva train_2ndmva.root --num 2
basf2 BBbar.py  -- -n 1000 --exp 1003 --ranseed 121 --output_file_mva test_2ndmva.root --num 2

#training
basf2 training_1st_flipping_mva.py
basf2 training_2nd_flipping_mva.py

#evaluation  
#basf2_mva_evaluate.py -id localdb/dbstore_Weightfile_rev_0b43a0.root -tree data -train train.root -data test.root -o validation.root
