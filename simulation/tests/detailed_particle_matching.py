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
This script checks, that the Detailed Particle Matching actually works, not if its behaviour changed!
"""

import basf2 as b2

# Set log level to avoid too much noise, but show warnings/errors
b2.logging.log_level = b2.LogLevel.WARNING


# Create path
main = b2.create_path()

# Add modules
main.add_module('EventInfoSetter', evtNumList=[5])  # Run 5 events
main.add_module('Gearbox')
main.add_module('Geometry')
main.add_module('ParticleGun', pdgCodes=[11], momentumParams=[1.0, 2.0])

# Configure FullSim with the new parameters
main.add_module('FullSim',
                DetailedParticleMatching=True,
                RegionRho=125.0,
                RegionZForward=196.0,
                RegionZBackward=-102.0,
                KineticEnergyThreshold=0.5,
                DistanceThreshold=1.0,
                UseIsEM=True,
                UseIsNuclei=False,
                UseSeenInECL=False)

# Process
b2.process(main)
