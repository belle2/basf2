#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import fei
import basf2 as b2
import modularAnalysis as ma
import pickle

# create path
path = b2.create_path()

# Load input ROOT file
ma.inputMdst(filename=b2.find_file('mdst16.root', 'validation', False),
             path=path)

# Get FEI default channels.
# Utilise the arguments to toggle on and off certain channels
particles = fei.get_default_channels()

# Set up FEI configuration specifying the FEI prefix
configuration = fei.config.FeiConfiguration(prefix='FEI_TEST', training=True)

# Get FEI path
feistate = fei.get_path(particles, configuration)

# Add FEI path to the path to be processed
path.add_path(feistate.path)

# Add RootOutput to save particles reconstructing during the training stage
output = b2.register_module('RootOutput')
if pickle.load(open('Summary.pickle', 'rb'))[1].roundMode == 3:  # this is necessary for the retraining mode
    output.param('excludeBranchNames', feistate.excludelists)
path.add_module(output)


# Process 100 events
b2.process(path, max_event=100)
