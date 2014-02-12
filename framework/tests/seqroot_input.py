#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Test SeqRootInputModule. Input data is the same as used by chain_input.py
# (both files)

from basf2 import *

import os
import shutil

from ROOT import Belle2


class TestModule(Module):
    """Test to read relations in the input files."""

    def event(self):
        """reimplementation of Module::event().

        prints all PXD true and simhit energy depositions, using relations
        """

        simhits = Belle2.PyStoreArray('PXDSimHits')
        for hit in simhits:
            relations = hit.getRelationsFrom("PXDTrueHits")
            for truehit in relations:
                print 'truehit (edep: %g) => hit(edep: %g)' % \
                           (truehit.getEnergyDep(), hit.getEnergyDep())


# copy input file into current dir to avoid having the full path in .out file
try:
    shutil.copy(os.getenv('BELLE2_LOCAL_DIR')
                + '/framework/tests/seqroot_input.sroot', '.')
except:
    pass  # we're probably in tests/ directory, no copy necessary

# ============================================================================
# Now lets create the necessary modules to perform a simulation

# Create Event information
# Show progress of processing
progress = register_module('Progress')
# Set input filename
input = register_module('SeqRootInput')
input.param('inputFileName', 'seqroot_input.sroot')
input.logging.log_level = LogLevel.WARNING  # ignore read rate

eventinfo = register_module('EventInfoPrinter')
printcollections = register_module('PrintCollections')


main = create_path()

main.add_module(input)
main.add_module(eventinfo)
main.add_module(printcollections)
main.add_module(TestModule())

process(main)
