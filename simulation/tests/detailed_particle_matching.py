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


class CheckExecutionModule(b2.Module):
    """Simple module to verify that events are processed."""

    def __init__(self):
        super().__init__()
        self.count = 0

    def event(self):
        self.count += 1

    def terminate(self):
        if self.count == 0:
            b2.B2FATAL("No events processed!")
        else:
            print(f"Processed {self.count} events successfully.")


# Register modules
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [5])  # Run 5 events

gearbox = b2.register_module('Gearbox')
geometry = b2.register_module('Geometry')
pgun = b2.register_module('ParticleGun')
pgun.param('pdgCodes', [11])
pgun.param('momentumParams', [1.0])

# Configure FullSim with the new parameters
g4sim = b2.register_module('FullSim')
g4sim.param('DetailedParticleMatching', True)
g4sim.param('RegionRho', 125.0)
g4sim.param('RegionZForward', 196.0)
g4sim.param('RegionZBackward', -102.0)
g4sim.param('KineticEnergyThreshold', 0.5)
g4sim.param('DistanceThreshold', 1.0)
g4sim.param('UseIsEM', True)
g4sim.param('UseIsNuclei', False)
g4sim.param('UseSeenInECL', False)  # Test without this requirement to keep it simple

# Create path
main = b2.create_path()
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pgun)
main.add_module(g4sim)
main.add_module(CheckExecutionModule())

# Process
b2.process(main)
