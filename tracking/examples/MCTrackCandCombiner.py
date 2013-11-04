#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will
import os
from basf2 import *
import sys

set_log_level(LogLevel.ERROR)
set_random_seed(101)

# the usual stuff
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [10])

eventinfoprinter = register_module('EventInfoPrinter')
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)  # MCTrackCandCombiner needs this parameter to be TRUE!!!

# digitizer
pxdDigitizer = register_module('PXDDigitizer')
svdDigitizer = register_module('SVDDigitizer')
pxdClusterizer = register_module('PXDClusterizer')
svdClusterizer = register_module('SVDClusterizer')
cdcDigitizer = register_module('CDCDigitizer')

# the VXD track finder
trackfinderVXD = register_module('VXDTF')
trackfinderVXD.logging.log_level = LogLevel.WARNING
secSetup = ['evtgenHIGH', 'evtgenSTD', 'evtgenLOW']
param_trackfinderVXD = {  # Supports 'SVD' and 'VXD' so far
    'tccMinState': [2],
    'tccMinLayer': [3],
    'detectorType': ['VXD'],
    'sectorSetup': secSetup,
    'GFTrackCandidatesColName': 'vxdcands',
    'TESTERexpandedTestingRoutines': False,
    }
trackfinderVXD.param(param_trackfinderVXD)

# any of the CDC track finder will work. Using CDCLegendreTracking is just an example the other track finder will also work
trackfinderCDC = register_module('CDCLegendreTracking')
param_trackfinderCDC = {'GFTrackCandidatesColName': 'cdccands'}
trackfinderCDC.param(param_trackfinderCDC)
trackfinderCDC.logging.log_level = LogLevel.WARNING

# the MCTrackCandCombiner takes the track candidates from the CDC track finder and the VXD track finder
# and uses simulated thruth info to decide which should be combined with which
# additionally it also makes a cleanup of the TCs, if several TCs where created from one real track
# only the TCs with the most hits from this real track will be put to the output StoreArray
mccombiner = register_module('MCTrackCandCombiner')
mccombiner.param('VXDTrackCandidatesColName', 'vxdcands')
mccombiner.param('CDCTrackCandidatesColName', 'cdccands')
mccombiner.param('HitsRatio', 0.6)
mccombiner.logging.log_level = LogLevel.INFO

trackfitter = register_module('GenFitter')
trackfitter.logging.log_level = LogLevel.WARNING

output = register_module('RootOutput')
output.param('outputFileName', 'fittedTracksWithVXDAndCDCHits.root')
# Create paths
main = create_path()
# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(evtgeninput)
main.add_module(g4sim)
main.add_module(pxdDigitizer)
main.add_module(pxdClusterizer)
main.add_module(svdDigitizer)
main.add_module(svdClusterizer)
main.add_module(cdcDigitizer)
main.add_module(trackfinderVXD)
main.add_module(trackfinderCDC)
main.add_module(mccombiner)
main.add_module(trackfitter)
main.add_module(output)
# Process events
process(main)

print statistics
