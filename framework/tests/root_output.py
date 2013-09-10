#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

# Register modules
evtmetagen = register_module('EvtMetaGen')
evtmetainfo = register_module('EvtMetaInfo')
output = register_module('RootOutput')

# Set module parameters
param_evtmetagen = {'expList': [1, 1], 'runList': [1, 2], 'evtNumList': [100,
                    100]}
evtmetagen.param(param_evtmetagen)

output.param('outputFileName', 'root_output_test.root')
output.param('updateFileCatalog', False)

# Create paths
main = create_path()

# Add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
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
