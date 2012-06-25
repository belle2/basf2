#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import random
from basf2 import *

# register necessary modules
evtmetagen = register_module('EvtMetaGen')

# generate one event
evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [20])

evtmetainfo = register_module('EvtMetaInfo')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

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
main.add_module(evtmetagen)
main.add_module(evtmetainfo)

main.add_module(gearbox)
main.add_module(geometry)
main.add_module(evtgeninput)
main.add_module(g4sim)

cdcdigi = register_module('CDCDigi')
main.add_module(cdcdigi)

pxd_digi = register_module('PXDDigitizer')
main.add_module(pxd_digi)

main.add_module(register_module('PXDClustering'))

mctrackfinder = register_module('MCTrackFinder')
mctrackfinder.param('UsePXDHits', True)
mctrackfinder.param('UseSVDHits', True)
mctrackfinder.param('UseCDCHits', True)
main.add_module(mctrackfinder)

genfit = register_module('GenFitter')
main.add_module(genfit)

dedx = register_module('DedxPID')
dedx_params = {
    'UseIndividualHits': True,
    'RemoveLowest': 0.15,
    'RemoveHighest': 0.15,
    'OnlyPrimaryParticles': False,
    'UsePXD': False,
    'UseSVD': True,
    'UseCDC': True,
    'TrackDistanceThreshold': 4.0,
    'EnableDebugOutput': True,
    'PDFFile': os.path.join(basf2datadir, 'analysis/dedxPID_PDFs_r3178.root'),
    'IgnoreMissingParticles': False,
    }
dedx.param(dedx_params)

main.add_module(dedx)

simpleoutput = register_module('SimpleOutput')
simpleoutput.param('outputFileName', 'dedxPID_evtgen.root')
main.add_module(simpleoutput)

process(main)
print statistics
