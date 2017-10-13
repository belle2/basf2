#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

set_random_seed("something important")


class NoopModule(Module):
    """Doesn't do anything."""


class TestModule(Module):
    """Test to read relations in the input files."""

    #: event counter
    iEvent = 0

    def event(self):
        """reimplementation of Module::event().

        prints PXD true and simhit indices, using relations
        """
        filemetadata = Belle2.PyStoreObj('FileMetaData', 1)
        nevents = filemetadata.obj().getNEvents()
        if self.iEvent < 12 and not nevents == 12:
            B2FATAL("FileMetaData from file 1 not loaded!")
        elif self.iEvent >= 12 and not nevents == 15:
            B2FATAL("FileMetaData from file 2 not loaded!")

        simhits = Belle2.PyStoreArray('PXDSimHits')
        for hit in simhits:
            relations = hit.getRelationsFrom("PXDTrueHits")
            for truehit in relations:
                print('truehit %d => hit %d' % (truehit.getArrayIndex(), hit.getArrayIndex()))

        self.iEvent += 1


inputfiles = [
    Belle2.FileSystem.findFile('framework/tests/chaintest_1.root'),
    Belle2.FileSystem.findFile('framework/tests/chaintest_2.root')
]

main = create_path()

# not used for anything, just checking wether the master module
# can be found if it's not the first module in the path.
main.add_module(NoopModule())

main.add_module('RootInput', logLevel=LogLevel.WARNING, inputFileNames=inputfiles)
main.add_module('EventInfoPrinter')
main.add_module('PrintCollections', printForEvent=0)

main.add_module(TestModule())

# Process events
process(main)
