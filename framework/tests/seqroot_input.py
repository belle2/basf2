#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Test SeqRootInputModule. Input data is the same as used by chain_input.py
# (both files)

from basf2 import *

from ROOT import Belle2


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


# ============================================================================
# Now lets create the necessary modules to perform a simulation

# Create Event information
# Show progress of processing
progress = register_module('Progress')
# Set input filename
input = register_module('SeqRootInput')
input.param('inputFileName', Belle2.FileSystem.findFile('framework/tests/seqroot_input.sroot'))
input.logging.log_level = LogLevel.WARNING  # ignore read rate

eventinfo = register_module('EventInfoPrinter')
printcollections = register_module('PrintCollections')


main = create_path()

main.add_module(input)
main.add_module(eventinfo)
main.add_module(printcollections)
main.add_module(TestModule())

process(main)
