#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import basf2
import modularAnalysis as ma
import b2test_utils

main = basf2.create_path()

ma.inputMdst("default", b2test_utils.require_file('analysis/tests/mdst.root'), path=main)

kaons = ('K-', 'kaonID > 0.05')
pions = ('pi+', 'pionID > 0.05')
photons = ('gamma', '')
ma.fillParticleLists([kaons, pions, photons], path=main)

ma.reconstructDecay('pi0 -> gamma gamma', '0.11 < M < 0.15', 0, path=main)
ma.reconstructDecay('D0 -> K- pi+ pi0', '1.7 < M < 1.9', 0, path=main)

output = basf2.register_module('RootOutput')
output.param('outputFileName', 'removeparticlesnotinlists_full.root')
main.add_module(output)

ma.summaryOfLists(['D0'], path=main)

ma.removeParticlesNotInLists(['D0'], path=main)

ma.matchMCTruth('D0', path=main)
ma.matchMCTruth('pi0', path=main)

# ensures that things in D0 list are ok
ntupler = basf2.register_module('VariablesToNtuple')
ntupler.param('fileName', 'removeparticlesnotinlists_D0ntuple.root')
ntupler.param('variables', ['M', 'daughter(0, M)', 'daughter(2, M)', 'mcPDG'])
ntupler.param('particleList', 'D0')
main.add_module(ntupler)

# pi0 list should also have been fixed
ntupler = basf2.register_module('VariablesToNtuple')
ntupler.param('fileName', 'removeparticlesnotinlists_pi0ntuple.root')
ntupler.param('variables', ['M', 'daughter(0, M)', 'mcPDG'])
ntupler.param('particleList', 'pi0')
main.add_module(ntupler)

output = basf2.register_module('RootOutput')
output.param('outputFileName', 'removeparticlesnotinlists_reduced.root')
main.add_module(output)

with b2test_utils.clean_working_directory():
    b2test_utils.safe_process(main)

    statfull = os.stat('removeparticlesnotinlists_full.root').st_size
    statreduced = os.stat('removeparticlesnotinlists_reduced.root').st_size
    basf2.B2RESULT("original size (kB): " + str(statfull / 1024))
    basf2.B2RESULT("reduced size (kB):  " + str(statreduced / 1024))
    if statfull <= statreduced:
        basf2.B2FATAL("Reduced file is not smaller than original")
