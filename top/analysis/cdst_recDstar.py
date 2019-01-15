#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Example of Dstar reconstruction where output ntuple includes some additional variables
# from group "TOP Calibration". Input must be a cdst file.
#
# usage: basf2 cdst_recDstar.py -i <cdst_file.root>
# ---------------------------------------------------------------------------------------

import sys
from basf2 import *
from modularAnalysis import *
from variables import variables
from ROOT import gSystem

gSystem.Load('libtop.so')

MC = False
VFit = False

if not MC:
    use_central_database("data_reprocessing_proc7", LogLevel.WARNING)

# Create path
main = create_path()

# Just a dummy input file, use basf2 -i option
inputMdstList('default', 'Input.root', path=main)

# Particle lists
fillParticleList('K-:all', '-2.0 < d0 < 2.0 and -4.0 < z0 < 4.0', path=main)
fillParticleList('pi+:all', '-2.0 < d0 < 2.0 and -4.0 < z0 < 4.0', path=main)

# Reconstruct D0 -> K- pi+ decay
reconstructDecay('D0:kpi -> K-:all pi+:all', '1.8 < M < 1.95', path=main)
if VFit:
    fitVertex('D0:kpi', 0.001, '', 'rave', 'vertex', '', True, path=main)

# Reconstruct D*+ -> D0 pi+ decay
reconstructDecay('D*+ -> D0:kpi pi+:all', '0.0 < Q < 0.020', path=main)
if VFit:
    fitVertex('D*+', 0.001, '', 'rave', 'vertex', 'ipprofile', True, path=main)

# MC matching
if MC:
    matchMCTruth('D*+', path=main)

# Output ntuple
variables.addAlias('isBunch', 'isTopRecBunchReconstructed')
variables.addAlias('bunchOffset', 'topRecBunchCurrentOffset')
variables.addAlias('M_D0', 'daughter(0, M)')
variables.addAlias('p_cms', 'useCMSFrame(p)')

variables.addAlias('p_K', 'daughter(0, daughter(0, p))')
variables.addAlias('flag_K', 'daughter(0, daughter(0, topFlag))')
variables.addAlias('kaonLL_K', 'daughter(0, daughter(0, topKaonLogL))')
variables.addAlias('pionLL_K', 'daughter(0, daughter(0, topPionLogL))')
variables.addAlias('slotID_K', 'daughter(0, daughter(0, topSlotID))')

variables.addAlias('p_pi', 'daughter(0, daughter(1, p))')
variables.addAlias('flag_pi', 'daughter(0, daughter(1, topFlag))')
variables.addAlias('kaonLL_pi', 'daughter(0, daughter(1, topKaonLogL))')
variables.addAlias('pionLL_pi', 'daughter(0, daughter(1, topPionLogL))')
variables.addAlias('slotID_pi', 'daughter(0, daughter(1, topSlotID))')

varlist = ['M_D0', 'Q', 'dQ', 'p_cms', 'isBunch', 'bunchOffset', 'p_K', 'p_pi',
           'kaonLL_K', 'pionLL_K', 'flag_K', 'slotID_K',
           'kaonLL_pi', 'pionLL_pi', 'flag_pi', 'slotID_pi']

variablesToNtuple('D*+', varlist, 'dstar', 'Dstar.root', path=main)

# Process events
process(main)

# Print statistics
print(statistics)
