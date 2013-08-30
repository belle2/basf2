#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import shutil
from basf2 import *

from basf2 import Module
from ROOT import Belle2


class NoopModule(Module):
    """Doesn't do anything."""


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
                + '/framework/tests/chaintest_1.root', '.')
    shutil.copy(os.getenv('BELLE2_LOCAL_DIR')
                + '/framework/tests/chaintest_2.root', '.')
except:
    pass  # we're probably in tests/ directory, no copy necessary

input = register_module('RootInput')
evtmetainfo = register_module('EvtMetaInfo')
printcollections = register_module('PrintCollections')

input.param('inputFileNames', ['chaintest_1.root', 'chaintest_2.root'])

main = create_path()

# not used for anything, just checking wether the master module
# can be found if it's not the first module in the path.
main.add_module(NoopModule())

main.add_module(input)
main.add_module(evtmetainfo)
main.add_module(printcollections)

main.add_module(TestModule())

# Process events
process(main)
