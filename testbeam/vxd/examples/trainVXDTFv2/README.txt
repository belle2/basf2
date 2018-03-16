this folder contains modified copies of the scripts in the folder:
racking/examples/VXDTF_redesign/
They have been modified to produce sectorMaps which can be in the testbeam geometry 
of the testbeam in Februar 2017


To perform all the training for the testbeam you have to run the chain in FOUR DIFFERENT settings!


Step 1) collect the training data. 
==================================

To run the training do the following: 

basf2 trainingPreparation.py -- --svd-only --magnet-off

to get all training needed you have to use all 4 possible setting: no options; -- --svd-only; -- --svd-only --magnet-off; -- --magnet-off

NOTE: basf2 will quit with an error message if the output file already exists! So move or delete output files from old trainings!


Step 2) train the sector Map:
=============================

there are several root files created in Step 1). The one relevant for the testbeam start with: "testbeamTEST_" (it should be 4 for the 
4 possible settings). For each of these files the training has to be performed seperately by running the following script:

basf2 trainSecMap.py -- --train_sample <trainings sample>

where <trainings sample> is one of the root files created in step 1). The output will be another root file containing
the sectormap. The name starts with "SecMap_" followed by the name of the input root file. This step should 
be repeated for all 4 different possible settings. 



Step 3) copy the sector maps
============================

The VXDTFv2 will assume that sector maps are the folder:

testbeam/vxd/data

so copy the root files with the sector map (see step 2) into this folder AND do "scons". The root files for the sector maps
all start with "SecMap_" (it should be 4)!





