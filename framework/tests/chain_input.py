#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

from basf2 import Module
from ROOT import Belle2


class NoopModule(Module):
    """Doesn't do anything."""


class TestModule(Module):
    """Test to read relations in the input files."""

    def event(self):
        """reimplementation of Module::event().

        prints PXD true and simhit indices, using relations
        """

        simhits = Belle2.PyStoreArray('PXDSimHits')
        for hit in simhits:
            relations = hit.getRelationsFrom("PXDTrueHits")
            for truehit in relations:
                print 'truehit %d => hit %d' % (truehit.getArrayIndex(), hit.getArrayIndex())


input = register_module('RootInput')
eventinfo = register_module('EventInfoPrinter')
printcollections = register_module('PrintCollections')

inputfiles = [
    Belle2.FileSystem.findFile('framework/tests/chaintest_1.root'),
    Belle2.FileSystem.findFile('framework/tests/chaintest_2.root')
]
input.param('inputFileNames', inputfiles)
input.logging.log_level = LogLevel.WARNING

main = create_path()

# not used for anything, just checking wether the master module
# can be found if it's not the first module in the path.
main.add_module(NoopModule())

main.add_module(input)
main.add_module(eventinfo)
main.add_module(printcollections)

main.add_module(TestModule())

# Process events
process(main)
