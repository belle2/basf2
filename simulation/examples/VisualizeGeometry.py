#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import basf2 as b2
b2.logging.log_level = b2.LogLevel.WARNING

# Set the environment variable to automatically start the VRML viewer
# for example http://freewrl.sourceforge.net/
os.environ['G4VRMLFILE_VIEWER'] = 'freewrl'

# create path
main = b2.create_path()
# process one event
main.add_module('EventInfoSetter', evtNumList=[1])
# Load XML parameters
main.add_module('Gearbox')
# Create Geometry and show info messages for that
main.add_module('Geometry', logLevel=b2.LogLevel.INFO)
particlegun = main.add_module('ParticleGun')
simulation = main.add_module('FullSim')

simulation.param('EnableVisualization', True)
simulation.param('UICommandsAtIdle', [
    # Use VRML2 backend
    '/vis/open VRML2FILE',
    # Draw the geometry
    '/vis/drawVolume',
    # Draw coordinate axes at the origin with a length of 100mm in each direction
    '/vis/scene/add/axes 0 0 0 100 mm',
    # Draw simulated tracks
    '/vis/scene/add/trajectories smooth',
    '/vis/modeling/trajectories/create/drawByCharge'
    # Uncomment the following two lines to have yellow dots at each step boundary
    # along the trajectory
    # '/vis/modeling/trajectories/drawByCharge-0/default/setDrawStepPts true',
    # '/vis/modeling/trajectories/drawByCharge-0/default/setStepPtsSize 0.5',
])

# Parameters for particle gun
particlegun.param('nTracks', 1)
particlegun.param('pdgCodes', [211])
particlegun.param('momentumParams', [2.0, 5.0])
particlegun.param('thetaParams', [85.0, 95.0])
particlegun.param('phiParams', [-5.0, 5.0])

b2.process(main)
