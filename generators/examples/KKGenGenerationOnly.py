#!/usr/bin/env python
# -*- coding: utf-8 -*-
from basf2 import *
set_log_level(LogLevel.DEBUG)

# to run the framework the used modules need to be registered
kkgeninput = register_module('KKGenInput')
# kkgeninput.param('tauinputFile','./tau.input.dat')
# kkgeninput.param('KKdefaultFile','./KK2f_defaults.dat')
# kkgeninput.param('taudecaytableFile','./tau_decaytable.dat')

evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('EvtNumList', [10])  # we want to process 100 events
evtmetagen.param('RunList', [1])  # from run number 1
evtmetagen.param('ExpList', [1])  # and experiment number 1
mcparticleprinter = register_module('PrintTauTauMCParticles')
mcparticleprinter.logging.log_level = LogLevel.INFO
mcparticleprinter.param('onlyPrimaries', False)

main = create_path()
main.add_module(evtmetagen)
main.add_module(kkgeninput)
main.add_module(mcparticleprinter)
process(main)
