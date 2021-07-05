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
This script is based on BremsstralungPhotons.py and was developed to test Belle2PhysicsList.

Saves 100 EvtGen events + secondary bremsstrahlung photons created by Geant4 in MCParticles.
The corresponding secondaryPhysicsProcess ID is 3, which is defined as fBremsstrahlung in G4EmProcessSubType.h.
"""

import basf2 as b2

main = b2.create_path()

# create 100 events
main.add_module("EventInfoSetter", evtNumList=[100])

# using standard evtgen
main.add_module("EvtGenInput")

# and parameters
main.add_module("Gearbox")

# and the geometry
main.add_module("Geometry")

# Invoke Belle2 physics list instead of Geant4 reference physics list.
# New options StandardEM, UseHighPrecisionNeutrons have been added.
# Otherwise, same collection of bremsstrahlung photons with kinetic
# energy > 10.0 MeV.
main.add_module("FullSim", PhysicsList="Belle2", RunEventVerbosity=0,
                RegisterOptics=False,
                #                StandardEM=True,
                #                UseHighPrecisionNeutrons=True,
                #                ProductionCut=0.07,
                StoreBremsstrahlungPhotons=True, BremsstrahlungPhotonsEnergyCut=10.0)

# output
main.add_module("RootOutput", outputFileName="EvtGenSimNoBkgYesBrems.root")

# run it
b2.process(main)

# Print call statistics
print(b2.statistics)
