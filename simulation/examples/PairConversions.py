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
This script saves e+ or e- from photon conversions into a pair in MCParticles.

<header>
  <contact>dorisykim@ssu.ac.kr</contact>
  <description>
      Saves 100 EvtGen events + secondary e+ or e- from pair conversions created by Geant4 in MCParticles.
      The corresponding secondaryPhysicsProcess ID is 14, which is defined as fGammaConversion in G4EmProcessSubType.h.
  </description>
</header>
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

# as well as the simulation, saving e+ or e- from pair conversions with kinetic energy > 10.0 MeV.
main.add_module("FullSim", StorePairConversions=True, PairConversionsEnergyCut=10.0)

# output
main.add_module("RootOutput", outputFileName="EvtGenSimNoBkgYesPairConversions.root")

# run it
b2.process(main)

# Print call statistics
print(b2.statistics)
