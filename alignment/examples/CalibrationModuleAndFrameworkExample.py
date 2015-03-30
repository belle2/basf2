#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import sys
from basf2 import *
from ROOT import Belle2
set_random_seed(100)
set_log_level(LogLevel.INFO)

main = create_path()

main.add_module('EventInfoSetter', evtNumList=[200])
main.add_module('Cosmics')

main.add_module('Gearbox')
main.add_module('Geometry', components=['PXD', 'SVD', 'CDC'])
"""
These will really run in parralel if you set so:
"""
main.add_module('FullSim')
main.add_module('CDCDigitizer', UseSimpleDigitization=True, DoSmearing=True, AddInWirePropagationDelay=False, AddTimeOfFlight=False)
main.add_module('TrackFinderMCTruth', UseClusters=False)
main.add_module(
    'GBLfit',
    logLevel=LogLevel.WARNING,
    PDGCodes=[13],
    EstimateSeedTime=False,
    UseTrackTime=False,
    seedFromDAF=False,
    RealisticCDCGeoTranslator=True,
    UseClusters=False,
    enableIntermediateScatterer=False
)
main.add_module('MillepedeCalibration', steering="steer.txt")

"""
Back to single processing
"""

main.add_module('Progress')

process(main)
print statistics
