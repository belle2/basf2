#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
logging.log_level = LogLevel.WARNING
# logging.log_level = LogLevel.INFO

eventinfosetter = register_module('EventInfoSetter')
# Load XML parameters

gearbox = register_module('Gearbox')
# Create Geometry
geobuilder = register_module('Geometry')
geobuilder.param('Components', ['CDC'])
geobuilder.log_level = LogLevel.INFO

pguninput = register_module('ParticleGun')

g4sim = register_module('FullSim')

eventinfosetter.param('evtNumList', [1])

g4sim.param('EnableVisualization', True)
g4sim.param('UICommands', ['/vis/open DAWNFILE', '/vis/drawVolume',
            '/vis/viewer/set/lineSegmentsPerCircle 100',
            '/vis/scene/add/axes 0 0 0 100 mm'])
#            '/vis/scene/add/axes 0 0 0 100 mm',
#            '/vis/scene/add/trajectories smooth',
#            '/vis/modeling/trajectories/create/drawByCharge'])

# Parameters for particle gun
pguninput.param('nTracks', 1)
pguninput.param('pdgCodes', [211])
pguninput.param('momentumParams', [2.0, 5.0])
pguninput.param('thetaParams', [85.0, 95.0])
pguninput.param('phiParams', [-5.0, 5.0])

main = create_path()
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(geobuilder)
# Without Particle Gun there are no tracks
main.add_module(pguninput)
main.add_module(g4sim)
process(main)
