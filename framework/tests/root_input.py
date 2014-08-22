#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

from basf2 import Module
from ROOT import Belle2


class NoopModule(Module):
    """Doesn't do anything."""


input = register_module('RootInput')
eventinfo = register_module('EventInfoPrinter')
printcollections = register_module('PrintCollections')

input.param('inputFileName', Belle2.FileSystem.findFile('framework/tests/root_input.root'))
# load all branches, minus PXDClusters
input.param('branchNames', [
    'EventMetaData',
    'PXDClustersToPXDDigits',
    'PXDClustersToPXDTrueHits',
    'PXDDigits',
    'PXDTrueHits',
    'PXDClusters', ])
input.param('excludeBranchNames', ['PXDClusters'])
input.logging.log_level = LogLevel.WARNING

main = create_path()

# not used for anything, just checking wether the master module
# can be found if it's not the first module in the path.
main.add_module(NoopModule())

main.add_module(input)
main.add_module(eventinfo)
main.add_module(printcollections)

# Process events
process(main)
