#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This script saves bremsstrahlung photons in MCParticles.

<header>
  <contact>dorisykim@ssu.ac.kr</contact>
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

# as well as the simulation, saving Bremsstrahlung photons with kinetic energy > 10.0 MeV.
main.add_module("FullSim", StoreBremsstrahlungPhotons=True, BremsstrahlungPhotonsEnergyCut=10.0)

# output
main.add_module("RootOutput", outputFileName="EvtGenSimNoBkgYesBrems.root")

# run it
process(main)

# Print call statistics
print(statistics)
