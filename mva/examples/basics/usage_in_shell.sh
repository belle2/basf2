#! /bin/bash

# Help for General and Meta options
basf2_mva_teacher --help

# Help for a specific method
basf2_mva_teacher --method FastBDT --help

# Train a MVA method and directly upload it to the database
basf2_mva_teacher --datafiles train.root --treename tree --identifier MVADatabaseIdentifier --variables p pz 'daughter(0, kaonID)' --target_variable isSignal --method FastBDT

# Download the weightfile from the database and store it on disk in a root file
basf2_mva_download --identifier weightfile.root --db_identifier MVADatabaseIdentifier

# Train a MVA method and store the weightfile on disk in a root file
basf2_mva_teacher --datafiles train.root --treename tree --identifier weightfile2.root --variables p pz 'daughter(0, kaonID)' --target_variable isSignal --method FastBDT

# Upload the weightfile on disk to the database
basf2_mva_upload --identifier weightfile2.root --db_identifier MVADatabaseIdentifier2 

# Apply the trained methods on data
basf2_mva_expert --identifiers weightfile.root weightfile2.root MVADatabaseIdentifier MVADatabaseIdentifier2 --datafiles train.root --treename tree --outputfile expert.root
