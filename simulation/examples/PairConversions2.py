#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This script saves e+ or e- from photon conversions into a pair in MCParticles.

<header>
  <contact>dorisykim@ssu.ac.kr</contact>
  <description>
      Saves 100 generic BBbar events with EvtGen + secondary e+ or e- from pair conversions created by Geant4 in MCParticles.
      The detector simulation mixed with background, trigger simulation, and standard reconstruction is done.
  </description>
</header>
"""

from basf2 import *
from simulation import add_simulation
from L1trigger import add_tsim
from reconstruction import add_reconstruction
from background import get_background_files

set_random_seed(12345)


main = create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter', evtNumList=[100], runList=[1], expList=[1])

# generate BBbar events
main.add_module('EvtGenInput')

# detector simulation
add_simulation(main, bkgfiles=get_background_files())

# saving e+ or e- from pair conversions with kinetic energy > 10.0 MeV.
set_module_parameters(main, "FullSim", StorePairConversions=True, PairConversionsEnergyCut=10.0)

# trigger simulation
add_tsim(main)

# reconstruction
add_reconstruction(main)

# output
main.add_module("RootOutput", outputFileName="EvtGenSimRecYesPairConversions.root")


process(main)

# Print call statistics
print(statistics)
