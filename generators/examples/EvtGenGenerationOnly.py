#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
set_log_level(LogLevel.INFO)

# to run the framework the used modules need to be registered
evtgeninput = register_module('EvtGenInput')
# evtgeninput.param('userDECFile',
#                    os.path.join(basf2datadir,'generators/belle/MIX.DEC'))

evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('EvtNumList', [100])  # we want to process 100 events
evtmetagen.param('RunList', [1])  # from run number 1
evtmetagen.param('ExpList', [1])  # and experiment number 1

mcparticleprinter = register_module('PrintMCParticles')
mcparticleprinter.logging.log_level = LogLevel.ERROR

main = create_path()
main.add_module(evtmetagen)
main.add_module(evtgeninput)
main.add_module(mcparticleprinter)
process(main)
