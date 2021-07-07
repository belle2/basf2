#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
from basf2 import create_path, process, B2FATAL, set_random_seed
from ROOT import TFile

set_random_seed("something important")

# Create paths
main = create_path()
# Add modules to paths
main.add_module("EventInfoSetter", expList=[1, 1], runList=[1, 2], evtNumList=[100, 100])
main.add_module("EventInfoPrinter")
main.add_module("RootOutput", outputFileName='root_output_test.root', updateFileCatalog=False)

# Process events
process(main)

# check wether output file contains correct number of events
tfile = TFile('root_output_test.root')
tree = tfile.Get('tree')
if tree.GetEntries() != 200:
    B2FATAL('Created output file contains wrong number of events!')

os.remove('root_output_test.root')
