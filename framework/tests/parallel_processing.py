#!/usr/bin/env python3

# Test the parallel processing functionality by simulating a few events
# using 2 processes (so nothing too taxing)

import os
import basf2
from ROOT import TFile, Belle2
from b2test_utils import skip_test_if_light, clean_working_directory
skip_test_if_light()  # cannot simulate events in a light release


class CheckEventNumbers(basf2.Module):
    """Class to check that we see all events we expect exactly once and nothing else"""

    def __init__(self, evtNumList):
        """Remember number of events to process"""
        super().__init__()
        #: the number of events so we expect the event numbers 0..(evtNumList-1)
        self.__evtNumList = evtNumList
        #: event numbers we actually saw
        self.__evtNumbers = []

    def event(self):
        """Accumulate all event numbers we see"""
        evtNr = Belle2.PyStoreObj("EventMetaData").obj().getEvent()
        self.__evtNumbers.append(evtNr)

    def terminate(self):
        """Check if event numbers are as they should be"""
        seen = self.__evtNumbers
        should = list(range(1, self.__evtNumList + 1))
        all_numbers = sorted(set(seen) | set(should))
        all_ok = True
        for evtNr in all_numbers:
            c = seen.count(evtNr)
            if not (evtNr in should and c == 1):
                basf2.B2ERROR("event number %d seen %d times" % (evtNr, c))
                all_ok = False
        if not all_ok:
            basf2.B2FATAL("Missing/extra events")


main = basf2.Path()
# init path
main.add_module("EventInfoSetter", evtNumList=[5])
particlegun = main.add_module("ParticleGun", pdgCodes=[211, -211, 321, -321],
                              momentumGeneration="fixed", momentumParams=[3])

# event path
main.add_module("Gearbox")
main.add_module("Geometry", components=['MagneticField', 'BeamPipe', 'PXD'], logLevel=basf2.LogLevel.ERROR)
simulation = main.add_module("FullSim", logLevel=basf2.LogLevel.ERROR)

# output path
main.add_module("RootOutput", outputFileName='parallel_processing_test.root')
main.add_module("Progress")
main.add_module(CheckEventNumbers(5))

# test wether flags are what we expect
if particlegun.has_properties(basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED):
    basf2.B2FATAL("ParticleGun has pp flag?")
if not simulation.has_properties(basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED):
    basf2.B2FATAL("Simulation doesn't have pp flag?")

# Process events in one more process than we have events to make sure at least
# one of them doesn't get an event
basf2.set_nprocesses(5)
with clean_working_directory():
    basf2.process(main)

    print(basf2.statistics)
    print(basf2.statistics(basf2.statistics.TOTAL))
    assert basf2.statistics.get(simulation).calls(basf2.statistics.EVENT) == 5
    # +1 because of extra call to master module
    assert basf2.statistics.get_global().calls(basf2.statistics.EVENT) == 6

    # check wether output file contains correct number of events
    file = TFile('parallel_processing_test.root')
    tree = file.Get('tree')
    if tree.GetEntries() != 5:
        basf2.B2FATAL('Created output file contains wrong number of events! (' + str(tree.GetEntries()) + ')')

    nummcparticles = tree.Project("", "MCParticles.m_pdg")
    if nummcparticles < 5:
        basf2.B2FATAL('Output file should contain at least five MCParticles!')

    numhits = tree.Project("", "PXDSimHits.getArrayIndex()")
    if numhits < 5:  # usually much more, existence is most important thing here
        basf2.B2FATAL('Output file should contain at least 5 hits!')
