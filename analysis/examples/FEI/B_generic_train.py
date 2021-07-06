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

# create path
path = b2.create_path()

# Load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('mdst14.root', 'validation', False),
             path=path)

# Get FEI default channels.
# Utilise the arguments to toggle on and off certain channels
particles = fei.get_default_channels()

# Set up FEI configuration specifying the FEI prefix
configuration = fei.config.FeiConfiguration(prefix='FEI_TEST', training=True, cache=-1)

# Get FEI path
feistate = fei.get_path(particles, configuration)

# Add FEI path to the path to be processed
path.add_path(feistate.path)

# Add RootOutput to save particles reconstructing during the training stage
path.add_module('RootOutput')

# Process 100 events
b2.process(path, max_event=100)
