#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import shutil
from basf2 import *

# copy input file into current dir to avoid having the full path in .out file
try:
    shutil.copy(os.getenv('BELLE2_LOCAL_DIR')
                + '/framework/tests/root_input.root', '.')
except:
    pass  # we're probably in tests/ directory, no copy necessary

input = register_module('RootInput')
evtmetainfo = register_module('EvtMetaInfo')
printcollections = register_module('PrintCollections')

input.param('inputFileName', 'root_input.root')
# load all branches, minus PXDClusters
input.param('branchNames', [
    'EventMetaData',
    'PXDClustersToPXDDigits',
    'PXDClustersToPXDTrueHits',
    'PXDDigits',
    'PXDTrueHits',
    'PXDClusters',
    ])
input.param('excludeBranchNames', ['PXDClusters'])

# also load persistent tree (contains FileMetaData object)
input.param('treeNamePersistent', 'persistent')

main = create_path()

# not used for anything, just checking wether the master module
# can be found if it's not the first module in the path.
main.add_module(register_module('Gearbox'))

main.add_module(input)
main.add_module(evtmetainfo)
main.add_module(printcollections)

# Process events
process(main)
