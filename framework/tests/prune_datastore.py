#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from b2test_utils import clean_working_directory


class TestModule(Module):
    """Test if the DataStore contains the expected content."""
    def __init__(self, is_inverted):
        """Create a new instance. If is_inverted is True we check of absence of content"""
        super().__init__()
        #: variable to remember if we test for existence or absence
        self._is_inverted = is_inverted

    def event(self):
        """reimplementation of Module::event().

        Checks for the amount of PXD Clusters and PXD Digits after
        the prune module was run
        """

        PXDClusters = Belle2.PyStoreArray('PXDClusters')
        PXDDigits = Belle2.PyStoreArray('PXDDigits')

        # PXDClusters are in our keep list, should still be there
        if self._is_inverted:
            assert PXDClusters.getEntries() == 0
            # while the PXDDigits should be empty
            assert PXDDigits.getEntries() > 0
        else:
            assert PXDClusters.getEntries() > 0
            # while the PXDDigits should be empty
            assert PXDDigits.getEntries() == 0

        # ensure the eventmetadata has been kept, which is implicitly done by
        # PruneDataStore
        evtmetadata = Belle2.PyStoreObj('EventMetaData')
        assert evtmetadata


set_random_seed("something important")
# make sure FATAL messages don't have the function signature as this makes
# problems with clang printing namespaces differently
logging.set_info(LogLevel.FATAL, logging.get_info(LogLevel.ERROR))
# find file to read
input_file = Belle2.FileSystem.findFile('framework/tests/root_input.root')

with clean_working_directory():
    main = create_path()

    main.add_module('RootInput', inputFileName=input_file)
    main.add_module('EventInfoPrinter')
    main.add_module('PrintCollections')
    main.add_module('PruneDataStore', matchEntries=['PXDClusters.*'])
    main.add_module('PrintCollections')
    main.add_module(TestModule(False))

    # ensure the pruned datastore is still write-able to disk
    main.add_module('RootOutput', outputFileName='prune_datastore_output_test.root', updateFileCatalog=False)

    # Process events
    process(main)

    # now test if the negated logic works, too
    main = create_path()

    main.add_module('RootInput', inputFileName=input_file)
    main.add_module('EventInfoPrinter')
    main.add_module('PrintCollections')
    main.add_module('PruneDataStore', matchEntries=['PXDClusters.*'], keepMatchedEntries=False)
    main.add_module('PrintCollections')
    main.add_module(TestModule(True))

    # ensure the pruned datastore is still write-able to disk
    main.add_module('RootOutput', outputFileName='prune_datastore_output_test.root', updateFileCatalog=False)

    # Process events
    process(main)
