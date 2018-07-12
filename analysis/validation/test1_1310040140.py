#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<input>../1310040140.dst.root</input>
<output>../1310040140.ntup.root</output>
<contact>Bilas Pal; palbs@ucmail.uc.edu </contact>
</header>
"""

#######################################################
# Analysis Validation : B2Charmless  WG
# reconstruct the B_s0 -> K_S0 K_S0 decay!
#                          |    |
#                          |    |
#              pi+ pi- <---|    |---> pi+ pi-
#
#
# Bilas Pal, April 12 2017
# palbs@ucmail.uc.edu
#
######################################################

from basf2 import *
from modularAnalysis import *
from beamparameters import add_beamparameters
from stdV0s import *
from ROOT import Belle2
from modularAnalysis import inputMdstList

# set the BeamParameters for running at Y(5S)
beamparameters = add_beamparameters(analysis_main, "Y5S")
# print_params(beamparameters)

inputMdst('default', '../1310040140.dst.root')
stdKshorts()
applyCuts('K_S0:all', '0.485611 < M < 0.509611')

# Reconstruct B_s0 from K_S0:mdst & K_S0:mdst
reconstructDecay('B_s0:KSKS -> K_S0:all K_S0:all', 'Mbc > 5.34 and deltaE > -0.2 and deltaE < 0.1')
matchMCTruth('B_s0:KSKS')

# write out useful information to a ROOT file
toolsBs = ['EventMetaData', '^B_s0']
toolsBs += ['InvMass', 'B_s0 -> ^K_S0 ^K_S0']
toolsBs += ['DeltaEMbc', '^B_s0']
toolsBs += ['CustomFloats[isSignal]', '^B_s0 -> ^K_S0 ^K_S0']

ntupleFile('../1310040140.ntup.root')
ntupleTree('Bs', 'B_s0:KSKS', toolsBs)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
