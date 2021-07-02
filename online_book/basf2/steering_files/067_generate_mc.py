#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import generators as ge
import simulation as si
import reconstruction as re
import mdst

# Create the steering path
main = b2.Path()

# Define number of events and experiment number
main.add_module('EventInfoSetter', evtNumList=[10], expList=[0])

# Generate B0B0bar events
ge.add_evtgen_generator(
    path=main,
    finalstate='signal',
    signaldecfile=b2.find_file('my_decay_file.dec')
)

# Simulate the detector response and the L1 trigger
si.add_simulation(path=main)

# Reconstruct the objects
re.add_reconstruction(path=main)

# Create the mDST output file
mdst.add_mdst_output(path=main, filename='my_mdst_output.root')

# Process the steering path
b2.process(path=main)

# Finally, print out some statistics about the modules execution
print(b2.statistics)
