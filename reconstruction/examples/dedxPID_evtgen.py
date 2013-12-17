#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import random
from basf2 import *

#change to True if you want to use PXD hits (fairly small benefit, if any)
use_pxd = False

# register necessary modules
eventinfosetter = register_module('EventInfoSetter')

# generate one event
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [20])
eventinfoprinter = register_module('EventInfoPrinter')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
geometry.param('ExcludedComponents', ['EKLM'])

# EvtGen to provide generic BB events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)

# simulation
g4sim = register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

# create paths
main = create_path()

# add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(evtgeninput)
main.add_module(g4sim)

if use_pxd:
    main.add_module(register_module('PXDDigitizer'))
    main.add_module(register_module('PXDClusterizer'))
main.add_module(register_module('SVDDigitizer'))
main.add_module(register_module('SVDClusterizer'))
main.add_module(register_module('CDCDigitizer'))

mctrackfinder = register_module('TrackFinderMCTruth')
mctrackfinder.param('UsePXDHits', use_pxd)
mctrackfinder.param('UseSVDHits', True)
mctrackfinder.param('UseCDCHits', True)
mctrackfinder.param('UseClusters', True)
main.add_module(mctrackfinder)

genfit = register_module('GenFitter')
genfit.param('UseClusters', True)
main.add_module(genfit)

dedx = register_module('DedxPID')
dedx_params = {
    'useIndividualHits': True,
    'removeLowest': 0.0,
    'removeHighest': 0.8,
    'onlyPrimaryParticles': False,
    'usePXD': use_pxd,
    'useSVD': True,
    'useCDC': True,
    'trackDistanceThreshold': 4.0,
    'enableDebugOutput': True,
    #'pdfFile': 'YourPDFFile.root',
    'ignoreMissingParticles': False,
    }
dedx.param(dedx_params)
main.add_module(dedx)
output = register_module('RootOutput')
output.param('outputFileName', 'dedxPID_evtgen.root')
main.add_module(output)
process(main)
print statistics
