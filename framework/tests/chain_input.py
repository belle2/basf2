#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
from b2test_utils import configure_logging_for_tests, skip_test_if_light
from ROOT import Belle2

skip_test_if_light()  # light builds don't know about PXD hits
configure_logging_for_tests()
basf2.set_random_seed("something important")


class NoopModule(basf2.Module):
    """Doesn't do anything."""


class TestModule(basf2.Module):
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
            basf2.B2FATAL("FileMetaData from file 1 not loaded!")
        elif self.iEvent >= 12 and not nevents == 15:
            basf2.B2FATAL("FileMetaData from file 2 not loaded!")

        simhits = Belle2.PyStoreArray('PXDSimHits')
        for hit in simhits:
            relations = hit.getRelationsFrom("PXDTrueHits")
            for truehit in relations:
                print('truehit %d => hit %d' % (truehit.getArrayIndex(), hit.getArrayIndex()))

        self.iEvent += 1


inputfiles = [
    basf2.find_file('framework/tests/chaintest_1.root'),
    basf2.find_file('framework/tests/chaintest_2.root')
]

basf2.conditions.disable_globaltag_replay()
main = basf2.Path()

# not used for anything, just checking wether the master module
# can be found if it's not the first module in the path.
main.add_module(NoopModule())

main.add_module('RootInput', logLevel=basf2.LogLevel.WARNING, inputFileNames=inputfiles)
main.add_module('EventInfoPrinter')
main.add_module('PrintCollections', printForEvent=0)

main.add_module(TestModule())

# Process events
basf2.process(main)
