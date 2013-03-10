#!/usr/bin/env python
# -*- coding: utf-8 -*-
from basf2 import *

# suppress messages and warnings during processing:
set_log_level(LogLevel.INFO)

# to run the framework the used modules need to be registered
evtgeninput = register_module('EvtGenInput')
# evtgeninput.param('userDECFile',
# os.path.join(basf2datadir,'generators/belle/MIX.DEC'))
evtgeninput.param('boost2LAB', True)
evtmetagen = register_module('EvtMetaGen')
paramloader = register_module('Gearbox')
geobuilder = register_module('Geometry')

evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('EvtNumList', [100])  # we want to process 100 events
evtmetagen.param('RunList', [1])  # from run number 1
evtmetagen.param('ExpList', [1])  # and experiment number 1
mcparticleprinter = register_module('PrintMCParticles')
mcparticleprinter.logging.log_level = LogLevel.ERROR

# creating the path for the processing
main = create_path()
main.add_module(evtmetagen)
main.add_module(paramloader)
main.add_module(geobuilder)

## Add hepevtreader module to path:
# main.add_module(evtgeninput)
## and print parameters for hepevtreader on startup of process
# print_params(evtgeninput)

main.add_module(evtgeninput)
main.add_module(mcparticleprinter)

# Process the events
process(main)
