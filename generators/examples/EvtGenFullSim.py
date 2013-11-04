#!/usr/bin/env python
# -*- coding: utf-8 -*-
from basf2 import *

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# to run the framework the used modules need to be registered
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
eventinfosetter = register_module('EventInfoSetter')
paramloader = register_module('Gearbox')
geobuilder = register_module('Geometry')
g4sim = register_module('FullSim')
simpleoutput = register_module('RootOutput')
mcparticleprinter = register_module('PrintMCParticles')

# Setting the option for all non-hepevt reader modules:
eventinfosetter.param('evtNumList', [3])  # we want to process 100 events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1
simpleoutput.param('outputFileName', 'EvtGenOutput.root')
mcparticleprinter.set_log_level(0)

# creating the path for the processing
main = create_path()
main.add_module(eventinfosetter)
main.add_module(paramloader)
main.add_module(geobuilder)

# Add hepevtreader module to path:
main.add_module(evtgeninput)
# and print parameters for hepevtreader on startup of process
print_params(evtgeninput)

# Add all other modules for simple processing to path
main.add_module(mcparticleprinter)
main.add_module(g4sim)
main.add_module(simpleoutput)

# Process the events
process(main)
# if there are less events in the input file the processing will be stopped at
# EOF.
