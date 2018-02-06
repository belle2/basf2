#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This script saves e+ or e- from photon conversions into a pair in MCParticles.

<header>
  <contact>dorisykim@ssu.ac.kr</contact>
  <description>
      Saves 100 EvtGen events + secondary e+ or e- from pair conversions created by Geant4 in MCParticles.
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

# as well as the simulation, saving e+ or e- from pair conversions with kinetic energy > 10.0 MeV.
main.add_module("FullSim", StorePairConversions=True, PairConversionsEnergyCut=10.0)

# output
main.add_module("RootOutput", outputFileName="EvtGenSimNoBkgYesPairConversions.root")

# run it
process(main)

# Print call statistics
print(statistics)
