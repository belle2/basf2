#! /bin/bash

# Train TMVA classification
basf2_mva_teacher --datafiles train.root --treename tree --identifier TMVA_BDT.xml --variables p pz 'daughter(0, kaonID)' --target_variable isSignal --method TMVAClassification --tmva_method BDT --tmva_type BDT --tmva_config '!H:!V:CreateMVAPdfs:NTrees=100' --tmva_prepare '!V:SplitMode=block:NormMode=None' --tmva_working_directory .
mv TMVA.root TMVA_BDT.root

basf2_mva_teacher --datafiles train.root --treename tree --identifier TMVA_Fisher.xml --variables p pz 'daughter(0, kaonID)' --target_variable isSignal --method TMVAClassification --tmva_method Fisher --tmva_type Fisher --tmva_config '!H:!V:CreateMVAPdfs' --tmva_prepare '!V:SplitMode=block:NormMode=None' --tmva_working_directory .
mv TMVA.root TMVA_Fisher.root

# Add TMVA rootfiles
hadd TMVA.root TMVA_BDT.root TMVA_Fisher.root

# Open TMVA Gui
TMVASYS="$ROOTSYS" root -e "TMVA::TMVAGui(\"TMVA.root\")"
