#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

# Register modules
eventnumbers = register_module('EventNumbers')
eventinfo = register_module('EventInfo')
output = register_module('RootOutput')

# Set module parameters
params_eventnumbers = {'expList': [1, 1], 'runList': [1, 2],
                    'evtNumList': [100, 100]}
eventnumbers.param(params_eventnumbers)

output.param('outputFileName', 'root_output_test.root')
output.param('updateFileCatalog', False)

# Create paths
main = create_path()

# Add modules to paths
main.add_module(eventnumbers)
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
