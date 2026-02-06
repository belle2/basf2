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
This script demonstrates how to filter secondary particles in the simulation
using the specialized ECL and kinetic energy parameters.
"""

import basf2 as b2

# Create the main path
main = b2.create_path()

# Generate 100 events
main.add_module("EventInfoSetter", evtNumList=[10])

# Use particle gun
main.add_module("ParticleGun", pdgCodes=[130], nTracks=1, momentumGeneration="fixed", momentumParams=[2.0])

# Load parameters and geometry
main.add_module("Gearbox")
main.add_module("Geometry")

# Run full simulation with ParticleMatching enabled

main.add_module("FullSim",
                StoreAllSecondaries=False,
                DetailedParticleMatching=True,
                RegionRho=114.0,
                RegionZForward=184.0,
                RegionZBackward=-92.0,
                KineticEnergyThreshold=0.02,
                DistanceThreshold=40,
                UseIsEM=True,
                UseIsNuclei=False,
                UseSeenInECL=True
                )

# Save output
main.add_module("RootOutput", outputFileName="FilteredSecondaries.root")

# Process the path
b2.process(main)

# Print statistics
print(b2.statistics)
