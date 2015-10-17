#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################

# Example steering file - 2011 Belle II Collaboration
########################################################

import os
import random
from basf2 import *

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.DEBUG)

# EventInfoSetter - generate event meta data
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [200])

# Root file output
# output = register_module('RootOutput')
# output.param('outputFileName', 'muForBKLM.root')

# Particle gun

particlegun = register_module('ParticleGun')
particlegun.param('nTracks', 1)
particlegun.param('pdgCodes', [13, -13])
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [20, 30])

particlegun.param('thetaGeneration', 'uniform')
# was 17 to 150
particlegun.param('thetaParams', [0, 180])

particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])

gearbox = register_module('Gearbox')
# print('Gearbox registered ')
# gearbox.param('backends', ['file:./geometry/data/', 'file:./XMLdata/', 'file:'])
# print('backend set ')
# gearbox.param('fileName', 'Belle2.xml')
# gearbox.param('InputFileXML','Belle2.xml')

# Geometry builder
geobuilder = register_module('Geometry')
# geobuilder.log_level = LogLevel.INFO
geobuilder.param('components', ['BKLM'])

g4sim = register_module('FullSim')

bklmdigi = register_module('BKLMDigitizer')
bklmPack = register_module('BKLMRawPacker')
bklmUnpack = register_module('BKLMUnpacker')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geobuilder)
main.add_module(g4sim)

main.add_module(bklmdigi)
main.add_module(bklmPack)
main.add_module(bklmUnpack)
# main.add_module(output)

process(main)
