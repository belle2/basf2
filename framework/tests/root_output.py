#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

# Register modules
eventinfosetter = register_module('EventInfoSetter')
eventinfo = register_module('EventInfoPrinter')
output = register_module('RootOutput')

# Set module parameters
params_eventinfosetter = {'expList': [1, 1], 'runList': [1, 2],
                    'evtNumList': [100, 100]}
eventinfosetter.param(params_eventinfosetter)

output.param('outputFileName', 'root_output_test.root')
output.param('updateFileCatalog', False)

# Create paths
main = create_path()

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfo)
main.add_module(output)

# Process events
process(main)

# check wether output file contains correct number of events
from ROOT import TFile
from ROOT import TTree
file = TFile('root_output_test.root')
tree = file.Get('tree')
if tree.GetEntries() != 200:
    B2FATAL('Created output file contains wrong number of events!')

os.remove('root_output_test.root')
