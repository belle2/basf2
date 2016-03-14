#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
set_log_level(LogLevel.INFO)

# to run the framework the used modules need to be registered
kkgeninput = register_module('KKGenInput')
kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/mu.input.dat'))
kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
kkgeninput.param('taudecaytableFile', '')

eventinfosetter = register_module('EventInfoSetter')
paramloader = register_module('Gearbox')
geobuilder = register_module('Geometry')

eventinfosetter.param('evtNumList', [10])  # we want to process 100 events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1
mcparticleprinter = register_module('PrintTauTauMCParticles')
mcparticleprinter.logging.log_level = LogLevel.DEBUG
main = create_path()
main.add_module(eventinfosetter)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(kkgeninput)
main.add_module(mcparticleprinter)
process(main)
