#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <output>../1110021010.dst.root</output>
  <contact>Frank Meier; frank.meier@duke.edu</contact>
</header>
"""
# Generates and reconstructs 1000 [B0 -> rho0 gamma] decay events for nightly
# validation. This script is NOT AN OFFICIAL DATA PRODUCTION script. Always
# always refer to the B2P project for the latest official scripts.

import basf2
from generators import add_evtgen_generator
from simulation import add_simulation
from reconstruction import add_reconstruction
from mdst import add_mdst_output

DECAY_MODE_ID = 1110021010
OUTPUT_FILENAME = f"../{DECAY_MODE_ID}.dst.root"

basf2.set_random_seed(12345)  # for reproducibility
main = basf2.Path()
main.add_module('EventInfoSetter', evtNumList=[1000])
add_evtgen_generator(
    main, 'signal',
    basf2.find_file(f'decfiles/dec/{DECAY_MODE_ID}.dec')
)                        # signal event generation
add_simulation(main)     # detector simulation without background overlay
add_reconstruction(main)  # reconstruction
add_mdst_output(main, filename=OUTPUT_FILENAME)  # mdst output
main.add_module('Progress')
basf2.process(main)
print(basf2.statistics)
