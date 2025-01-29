#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
This script saves Bremsstrahlung photons in MCParticles.

Saves 100 generic BBbar events with EvtGen + Bremsstrahlungi photons created by Geant4 in MCParticles.
The corresponding secondaryPhysicsProcess ID is 3, which is defined as fBremsstrahlung in G4EmProcessSubType.h.
The detector simulation mixed with background, trigger simulation, and standard reconstruction are done.
"""

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction
from background import get_background_files

b2.set_random_seed(12345)


main = b2.create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter', evtNumList=[100], runList=[1], expList=[1])

# generate BBbar events
main.add_module('EvtGenInput')

# detector and L1 trigger simulation
add_simulation(main, bkgfiles=get_background_files())

# saving Bremsstrahlung photons with kinetic energy > 10.0 MeV.
b2.set_module_parameters(main, "FullSim", StoreBremsstrahlungPhotons=True, BremsstrahlungPhotonsEnergyCut=10.0)

# reconstruction
add_reconstruction(main)

# output
main.add_module("RootOutput", outputFileName="EvtGenSimRecYesBrems.root")


b2.process(main)

# Print call statistics
print(b2.statistics)
