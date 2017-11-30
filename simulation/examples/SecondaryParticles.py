#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This script saves all the secondary particles in MCParticles.

<header>
  <contact>dorisykim@ssu.ac.kr</contact>
  <description>
      Saves 100 EvtGen events + all the secondary particles created by Geant4 in MCParticles.
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

# as well as the simulation, saving secondary particles with kinetic energy > 1.0 MeV.
main.add_module("FullSim", StoreAllSecondaries=True, SecondariesEnergyCut=1.0)

# output
main.add_module("RootOutput", outputFileName="EvtGenSimNoBkgYesSecondaries.root")

# run it
process(main)

# Print call statistics
print(statistics)
