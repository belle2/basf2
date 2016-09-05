#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
import multiprocessing


class TestModule(Module):

    """Test if the DataStore contains the expected content."""

    def event(self):
        """reimplementation of Module::event().

        Checks for the amount of PXD Clusters and PXD Digits after
        the prune module was run
        """

        PXDClusters = Belle2.PyStoreArray('PXDClusters')
        PXDDigits = Belle2.PyStoreArray('PXDDigits')

        # PXDClusters are in our keep list, should still be there
        assert PXDClusters.getEntries() > 0
        # while the PXDDigits should be empty
        assert PXDDigits.getEntries() == 0

set_random_seed("something important")
# make sure FATAL messages don't have the function signature as this makes
# problems with clang printing namespaces differently
logging.set_info(LogLevel.FATAL, logging.get_info(LogLevel.ERROR))

input = register_module('RootInput')
eventinfo = register_module('EventInfoPrinter')
printcollections = register_module('PrintCollections')

input.param('inputFileName', Belle2.FileSystem.findFile('framework/tests/root_input.root'))
main = create_path()

main.add_module(input)
main.add_module(eventinfo)
main.add_module(printcollections)

prune = register_module("PruneDatastore")
prune.param('keepEntries', ['PXDClusters.*', 'EventMetaData'])
main.add_module(prune)

main.add_module(register_module('PrintCollections'))
main.add_module(TestModule())

# Process events
process(main)
