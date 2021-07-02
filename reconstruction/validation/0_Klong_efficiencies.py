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
<header>
  <output>K_long_full_validation_sample.root</output>
  <description>Generates particle gun Klong Events for Kl efficiency validation.\t
   Output is not very meaningful but sufficient to check for regression.</description>
  <contact>benjamin.oberhof@lnf.infn.it</contact>
  <interval>nightly</interval>
</header>
"""

import basf2 as b2
import simulation as sim
import reconstruction as rec

import os
import glob

bg = None
if 'BELLE2_BACKGROUND_DIR' in os.environ:
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
else:
    b2.B2FATAL('The variable BELLE2_BACKGROUND_DIR is not set!')

b2.set_random_seed('L1V0RN0')

main = b2.create_path()

main.add_module('EventInfoSetter',
                expList=0,
                runList=0,
                evtNumList=1000)

main.add_module('ParticleGun',
                nTracks=5,
                momentumGeneration='uniform',
                momentumParams=[0.05, 5.0],
                pdgCodes=[130, 321, 311, 2212, 2112, 211, 13, 11])

sim.add_simulation(path=main,
                   bkgfiles=bg)

rec.add_reconstruction(path=main)

# Run a module to generate histograms for PID performances
main.add_module('KlongValidation',
                outputName='K_long_full_validation_sample.root',
                KlIdCut=0.1)

# add_mdst_output(main, True, 'Klong_validation_check.root')

main.add_module('Progress')

b2.process(main)
print(b2.statistics)
