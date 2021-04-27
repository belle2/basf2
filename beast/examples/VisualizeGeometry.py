#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import basf2 as b2
b2.logging.log_level = b2.LogLevel.WARNING

# Set the environment variable to automatically start the VRML viewer
# for example http://freewrl.sourceforge.net/
os.environ['G4VRMLFILE_VIEWER'] = 'freewrl'

eventinfosetter = b2.register_module('EventInfoSetter')
# Load XML parameters
paramloader = b2.register_module('Gearbox')
# paramloader.param('fileName', '/geometry/Beast2_phase2.xml')
paramloader.param('fileName', '/geometry/Beast2_phase1.xml')
# paramloader.param('fileName', '/home/igal/src/belle2/release/beast/examples/Beast2_phase1.xml')
# paramloader.param('fileName', '/beast/Beast2_phase1.xml')
# Create Geometry
geobuilder = b2.register_module('Geometry')
geobuilder.log_level = b2.LogLevel.INFO
pguninput = b2.register_module('ParticleGun')
g4sim = b2.register_module('FullSim')

eventinfosetter.param('evtNumList', [1])

g4sim.param('EnableVisualization', True)
g4sim.param('UICommandsAtIdle', ['/vis/open VRML2FILE', '/vis/drawVolume',
                                 '/vis/scene/add/axes 0 0 0 31 mm',
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

main = b2.create_path()

main.add_module(eventinfosetter)
main.add_module(paramloader)
main.add_module(geobuilder)
# Without Particle Gun there are no tracks
main.add_module(pguninput)
main.add_module(g4sim)

b2.process(main)
