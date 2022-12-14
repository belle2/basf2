#!/bin/bash
# A shell script to execute embedding sequence

# remove tag for data: 
python3 analysis/scripts/udst_purge_list.py --listName "B+:BplusToKplusJpsiMuMu"  --charge 'pos' --fileIn $BELLE2_EXAMPLES_DATA_DIR/embedding_skim_BplusToKplusJpsiMuMu_bucket17.udst.root --fileOut data.root
# keep truth-matched signal for MC
python3 analysis/scripts/udst_purge_list.py --isSignal --listName "B+:BplusToKplusNuNu"  --charge 'pos' --fileIn $BELLE2_EXAMPLES_DATA_DIR/embedding_skim_sig_71_Bplus2Kplus.udst.root --fileOut sigmc.root

# merge them together, adjust kinematics of the signal to match ROE:
python3 analysis/scripts/embedding_merge.py --filePrimary data.root --fileSecondary sigmc.root --fileOut embedded_data.root
