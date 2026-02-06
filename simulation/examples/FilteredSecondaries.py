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
main.add_module("EventInfoSetter", evtNumList=[100])

# Use particle gun
main.add_module("ParticleGun", pdgCodes=[11], nTracks=1, momentumGeneration="fixed", momentumParams=[2.0])

# Load parameters and geometry
main.add_module("Gearbox")
main.add_module("Geometry")

# Run full simulation with secondary filtering enabled
# These parameters were added to filter secondaries based on ECL geometry and particle properties.
# Note: StoreAllSecondaries must likely be True for the filtering to operate on broadly collected secondaries,
# or at least it interacts with how particles are initially collected before filtering.
# Based on the implementation, the filter *re-enables* ignored particles under certain conditions
# or was originally designed to select from ignored ones.
#
# In the original code (TrackingAction.cc):
# if (use_seen_in_ecl) { if (seenInECL) { currParticle.setIgnore(false); } }
# It seems this logic is about *keeping* particles that would otherwise be ignored (default Geant4 behavior for low energy etc).
#
# So we usually want standard simulation, but maybe with these flags to KEEP specific interesting secondaries.

main.add_module("FullSim",
                StoreAllSecondaries=False,  # We might want to selectively store, not store all.
                DetailedParticleMatching=True,     # Enable the custom filtering logic to UN-ignore specific particles
                RegionRho=125.0,            # Radius of the matching region (cm)
                RegionZForward=196.0,       # Forward Z limit of the matching region (cm)
                RegionZBackward=-102.0,     # Backward Z limit of the matching region (cm)
                KineticEnergyThreshold=10.0,  # Kinetic energy threshold (MeV)
                DistanceThreshold=5.0,      # Distance threshold for decay/vertex (cm)
                UseIsEM=True,               # Check if particle is EM
                UseIsNuclei=False,          # Check if particle is Nuclei
                UseSeenInECL=True           # Check if particle was seen in ECL (for particles before region)
                )

# Save output
main.add_module("RootOutput", outputFileName="FilteredSecondaries.root")

# Process the path
b2.process(main)

# Print statistics
print(b2.statistics)
