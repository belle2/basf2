#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# This steering file generates MCParticles, performs
# a full Geant4 simulation and saves the result to disk.
#
# The following parameters are used:
#  Number of events:      100
#  Tracks per event:      10
#  Particles:             electrons / positrons
#  Theta:                 17 to 150 degree
#  Phi:                   0 to 360 degree
#  Momentum:              50 MeV to 3 GeV
#
# Example steering file - 2011 Belle II Collaboration
########################################################

import os
import random
from basf2 import *

# Set the log level to show only error and fatal messages
set_log_level(3)

# Register modules
particlegun = register_module('PGunInput')
paramloader = register_module('ParamLoaderXML')
geobuilder = register_module('GeoBuilder')
g4sim = register_module('FullSim')
simpleoutput = register_module('SimpleOutput')

# Set parameters
intseed = random.randint(1, 10000000)

particlegun.param('ntracks', 10)
particlegun.param('PIDcodes', [11, -11])
particlegun.param('p_par1', 0.05)
particlegun.param('p_par2', 3)
particlegun.param('Rseed', intseed)

paramloader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))

simpleoutput.param('outputFileName', 'particleGunSimResult.root')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(particlegun)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(g4sim)
main.add_module(simpleoutput)

# Process 100 events with run number 1
process(main, 100, 1)
