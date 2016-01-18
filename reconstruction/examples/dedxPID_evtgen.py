#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# This steering file generates several events and
# does a simple dE/dx measurement with path length
# correction. The results are stored in a ROOT file.
#
# Usage: basf2 dedxPID_evtgen.py
#
# Input: None
# Output: dedxPID_evtgen.root
#
# Example steering file - 2011 Belle II Collaboration
#############################################################

import os
import random
from basf2 import *
from simulation import add_simulation
from beamparameters import add_beamparameters

# change to True if you want to use PXD hits (fairly small benefit, if any)
use_pxd = False

# register necessary modules
eventinfosetter = register_module('EventInfoSetter')

# generate one event
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [50])
eventinfoprinter = register_module('EventInfoPrinter')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
geometry.param('excludedComponents', ['EKLM'])

# EvtGen to provide generic BB events
evtgeninput = register_module('EvtGenInput')

# simulation
g4sim = register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

# run the MC track finder
mctrackfinder = register_module('TrackFinderMCTruth')
mctrackfinder.param('UsePXDHits', use_pxd)
mctrackfinder.param('UseSVDHits', True)
mctrackfinder.param('UseCDCHits', True)
mctrackfinder.param('UseClusters', True)

# set up genfit
genfitextrap = register_module('SetupGenfitExtrapolation')
genfit = register_module('GenFitter')
# genfit.param('UseClusters', True)

# set up the CDC dE/dx module
cdcdedx = register_module('CDCDedxPID')
cdcdedx_params = {  # 'pdfFile': 'YourPDFFile.root',
    'useIndividualHits': True,
    'removeLowest': 0.05,
    'removeHighest': 0.25,
    'onlyPrimaryParticles': False,
    'enableDebugOutput': True,
    'ignoreMissingParticles': False,
}
cdcdedx.param(cdcdedx_params)

# set up the VXD dE/dx module
svddedx = register_module('VXDDedxPID')
svddedx_params = {  # 'pdfFile': 'YourPDFFile.root',
    'useIndividualHits': True,
    'removeLowest': 0.05,
    'removeHighest': 0.25,
    'onlyPrimaryParticles': False,
    'usePXD': use_pxd,
    'useSVD': True,
    'trackDistanceThreshold': 4.0,
    'enableDebugOutput': True,
    'ignoreMissingParticles': False,
}
svddedx.param(svddedx_params)

# write the results to file
output = register_module('RootOutput')
output.param('outputFileName', 'dedxPID_evtgen.root')


# create paths
main = create_path()


# add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)

add_beamparameters(main, "Y4S")
main.add_module(evtgeninput)
main.add_module(g4sim)

if use_pxd:
    main.add_module(register_module('PXDDigitizer'))
    main.add_module(register_module('PXDClusterizer'))
main.add_module(register_module('SVDDigitizer'))
main.add_module(register_module('SVDClusterizer'))
main.add_module(register_module('CDCDigitizer'))

main.add_module(mctrackfinder)
main.add_module(genfitextrap)
main.add_module(genfit)

main.add_module(cdcdedx)
main.add_module(svddedx)

main.add_module(output)

# process events and print call statistics
process(main)
print(statistics)
