#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
logging.log_level = LogLevel.WARNING

# Set the environment variable to automatically start the VRML viewer
# for example http://freewrl.sourceforge.net/
os.environ['G4VRMLFILE_VIEWER'] = 'freewrl'

eventinfosetter = register_module('EventInfoSetter')
# Load XML parameters
paramloader = register_module('Gearbox')
# Create Geometry
geobuilder = register_module('Geometry')
geobuilder.log_level = LogLevel.INFO
pguninput = register_module('ParticleGun')
g4sim = register_module('FullSim')

eventinfosetter.param('evtNumList', [1])

g4sim.param('EnableVisualization', True)
g4sim.param('UICommands', ['/vis/open VRML2FILE', '/vis/drawVolume',
            '/vis/scene/add/axes 0 0 0 1000 mm',
            '/vis/scene/add/trajectories smooth',
            '/vis/modeling/trajectories/create/drawByCharge'])  # Use VRML2 backend
                                                                # Draw the geometry
                                                                # Draw coordinate axes at the origin with a length of 100mm in each direction
                                                                # Draw simulated tracks
# Uncomment the following two lines to have yellow dots at each step boundary
# along the trajectory
# '/vis/modeling/trajectories/drawByCharge-0/default/setDrawStepPts true',
# '/vis/modeling/trajectories/drawByCharge-0/default/setStepPtsSize 0.5',

# Parameters for particle gun
pguninput.param('nTracks', 1)
pguninput.param('pdgCodes', [211])
pguninput.param('momentumParams', [2.0, 5.0])
pguninput.param('thetaParams', [85.0, 95.0])
pguninput.param('phiParams', [-5.0, 5.0])

main = create_path()

main.add_module(eventinfosetter)
main.add_module(paramloader)
main.add_module(geobuilder)
# Without Particle Gun there are no tracks
main.add_module(pguninput)
main.add_module(g4sim)

process(main)
