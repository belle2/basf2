#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This script is based on BremsstralungPhotons.py and was developed to test Belle2PhysicsList.

<header>
  <contact>dwright@slac.stanford.edu</contact>
  <description>
      Saves 100 EvtGen events + secondary bremsstrahlung photons created by Geant4 in MCParticles.
      The corresponding secondaryPhysicsProcess ID is 3, which is defined as fBremsstrahlung in G4EmProcessSubType.h.
  </description>
</header>
"""

from basf2 import *

main = create_path()

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
process(main)

# Print call statistics
print(statistics)
