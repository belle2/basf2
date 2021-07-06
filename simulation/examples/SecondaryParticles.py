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
This script saves all the secondary particles in MCParticles.

Saves 100 EvtGen events + all the secondary particles created by Geant4 in MCParticles.
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

# as well as the simulation, saving secondary particles with kinetic energy > 1.0 MeV.
main.add_module("FullSim", StoreAllSecondaries=True, SecondariesEnergyCut=1.0)

# output
main.add_module("RootOutput", outputFileName="EvtGenSimNoBkgYesSecondaries.root")

# run it
b2.process(main)

# Print call statistics
print(b2.statistics)
