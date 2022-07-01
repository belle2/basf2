#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#####################################################################
# VXDTF2 Example Scripts - Step 0 - Simulation
#
# This script can be used to produce MC data from particle guns or
# Y(4S) events for the training and validation of the VXDTF 1 and 2.
#
# The number of events which will be simulated can be set via the
# basf2 commandline option -n.
# The name of the root output file can be defined with the option -o.
#
# E.g.: 'basf2 eventSimulation.py -n 1000 -o trainingSample.root'
#
# The settings for the particle gun(s) and EvtGen simulation can be
# adapted in this script. Some convenience functions are outsourced
# to setup_modules.py.
#
# The script takes two optional command line arguments: the first will
# be interpreted as random seed, the second as directory for the output.
# e.g: basf2 'eventSimulation.py 12354 ./datadir/'
# will result in setting the random seed to 12354 and the output will
# be written to './datadir/'
#####################################################################


import basf2 as b2

from SectorMapTrainingUtils import add_event_generation, add_simulation_and_reconstruction_modules, add_rootoutput

main = b2.create_path()

# needs input! adds either BB or ee event generation (no detector simulation)
add_event_generation(path=main, randomSeed=12345, eventType="BBbar", expNumber=0)

# adds simulation and reconstruction as needed for the training
# for now we dont need the PXD
add_simulation_and_reconstruction_modules(path=main, usePXD=False)

# make root output optional, as its a lot!!
if False:
    add_rootoutput(path=main, outputFileName="test.root")

# dump some logging
b2.log_to_file('createSim.log', append=False)


b2.print_path(main)

main.add_module("Progress")

b2.process(main)
print(b2.statistics)
