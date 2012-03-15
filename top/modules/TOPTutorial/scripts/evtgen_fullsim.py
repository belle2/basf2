#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# Initialize EvtGen
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
evtgeninput.param('userDECFile', 'B2Kpi.dec')

# Register modules that are necessary for simulation
progress = register_module('Progress')
evtmetagen = register_module('EvtMetaGen')
paramloader = register_module('Gearbox')
geobuilder = register_module('Geometry')
g4sim = register_module('FullSim')
param_g4sim = {'RegisterOptics': 1, 'PhotonFraction': 0.3,
               'TrackingVerbosity': 0}
g4sim.param(param_g4sim)
simpleoutput = register_module('SimpleOutput')
# mcparticleprinter = register_module('PrintMCParticles')
# mcparticleprinter.logging.log_level = LogLevel.ERROR

# Setting the option for all non-hepevt reader modules:
evtmetagen.param('EvtNumList', [5])  # we want to process 100 events
evtmetagen.param('RunList', [1])  # from run number 1
evtmetagen.param('ExpList', [1])  # and experiment number 1

simpleoutput.param('outputFileName', 'EvtGenOutput.root')

# creating the path for the processing
main = create_path()
main.add_module(evtmetagen)
main.add_module(progress)
main.add_module(paramloader)
main.add_module(geobuilder)

# Add hepevtreader module to path:
main.add_module(evtgeninput)
# and print parameters for hepevtreader
# on startup of process
print_params(evtgeninput)
# main.add_module(mcparticleprinter)

# Add all other modules for simple processing to path
main.add_module(g4sim)
main.add_module(simpleoutput)

# Process the events
process(main)
# if there are less events in the input file

# Print call statistics
print statistics

# the processing will be stopped at EOF.
