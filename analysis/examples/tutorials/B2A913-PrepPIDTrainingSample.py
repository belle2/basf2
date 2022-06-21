#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
# This tutorial shows how to create a data sample for training the PID   #
# calibration weights. After completing the tutorial, you can train the  #
# weights using the pidTrainWeights script, e.g.                         #
# $ python /path/to/release/analysis/scripts/pidTrainWeights.py \  #
#       ./data/slim_dstar ./models/net.pt -n 1000  #
##########################################################################

import basf2 as b2
import root_pandas as rtp
import pidDataUtils as pdu
from os import makedirs

# Read data into DataFrame
df = rtp.read_root(b2.find_file('mc_dstar.root', 'examples', False))
print("ROOT file read into DataFrame.")

# Make slim h5 files for each particle type and merge into one large file
makedirs('data', exist_ok=True)
pdu.make_h5(df, ['DST_D0_pi', 'DST_pi'], 'data/slim_dstar_pion.h5', pdg=211)
print("Slim h5 file made at data/slim_dstar_pion.h5")

pdu.make_h5(df, ['DST_D0_K'], 'data/slim_dstar_kaon.h5', pdg=321)
print("Slim h5 file made at data/slim_dstar_kaon.h5")

pdu.merge_h5s(['data/slim_dstar_pion.h5', 'data/slim_dstar_kaon.h5'], 'data/slim_dstar.h5')
print("H5 files merged, written out to data/slim_dstar.h5")

# Split into train/val/test sets for training
pdu.split_h5('data/slim_dstar.h5', 'data/slim_dstar')
print("Data in data/slim_dstar.h5 split into train/val/test files in directory: data/slim_dstar")

# Now you can train weights using the pidTrainWeights script, e.g.
# $ python3 path/to/pidTrainWeights.py ./data/slim_dstar ./models/net.pt -n 1000
