#!/usr/bin/env python
# -*- coding: utf-8 -*-
from basf2 import *

# suppress messages and warnings during processing:
set_log_level(LogLevel.INFO)
#set_random_seed(75628607)

evtnumbers = register_module('EventInfoSetter')
evtnumbers.param('evtNumList', [100])  # we want to process 100 events
evtnumbers.param('runList', [1])  # from run number 1
evtnumbers.param('expList', [1])  # and experiment number 1

paramloader = register_module('Gearbox')

geobuilder = register_module('Geometry')

# to run the framework the used modules need to be registered
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('ParentParticle', 'vpho')
evtgeninput.param('userDECFile', os.environ['BELLE2_LOCAL_DIR']
                  + '/generators/evtgen/decayfiles/ccbar+Dst.dec')
# evtgeninput.param('userDECFile', os.environ['BELLE2_LOCAL_DIR']
#                   + '/generators/evtgen/decayfiles/uubar.dec')
evtgeninput.param('boost2LAB', True)

mcparticleprinter = register_module('PrintMCParticles')
mcparticleprinter.logging.log_level = LogLevel.INFO

# creating the path for the processing
main = create_path()
main.add_module(evtnumbers)
main.add_module(paramloader)
main.add_module(geobuilder)

main.add_module(evtgeninput)
main.add_module(mcparticleprinter)

# Process the events
process(main)
