#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Test for the DetailedParticleMatching configuration in FullSim.
"""

import basf2 as b2

# Set log level to avoid too much noise, but show warnings/errors
b2.logging.log_level = b2.LogLevel.WARNING


# Register modules
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [5])  # Run 5 events

gearbox = b2.register_module('Gearbox')
geometry = b2.register_module('Geometry')
pgun = b2.register_module('ParticleGun')
pgun.param('pdgCodes', [11])
pgun.param('momentumParams', [1.0, 2.0])

# Configure FullSim with the new parameters
full_sim = b2.register_module('FullSim')
full_sim.param('DetailedParticleMatching', True)
full_sim.param('RegionRho', 125.0)
full_sim.param('RegionZForward', 196.0)
full_sim.param('RegionZBackward', -102.0)
full_sim.param('KineticEnergyThreshold', 0.5)
full_sim.param('DistanceThreshold', 1.0)
full_sim.param('UseIsEM', True)
full_sim.param('UseIsNuclei', False)
full_sim.param('UseSeenInECL', False)

# Create path
main = b2.create_path()
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pgun)
main.add_module(full_sim)

# Process
b2.process(main)
