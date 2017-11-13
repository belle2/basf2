#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../1213012101.dst.root</input>
  <output>../1213012101.ntup.root</output>
  <contact>Minakshi Nayak; minakshi.nayak@wayne.edu </contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from stdV0s import *
from ROOT import Belle2
from modularAnalysis import inputMdstList
from glob import glob
from stdPi0s import *

inputMdstList('default', '../1213012101.dst.root')

stdPi0s('looseFit')

fillParticleList('K+:all', 'kaonID > 0.5 and abs(d0) < 0.5 and abs(z0) < 1')

# create Ks -> pi+ pi- list from V0
stdKshorts()
applyCuts('K_S0:all', '')
matchMCTruth('K_S0:all')

# reconstruct D0, tracks originate from the same vertex
reconstructDecay('D0:kspi0 -> K_S0:all pi0:looseFit', '1.75 < M < 1.95 and 1.8 < useCMSFrame(p) < 2.8')
# vertexKFit('D0:kspi0', 0.0)
matchMCTruth('D0:kspi0')

# reconstruct B+
reconstructDecay('B-:dk -> D0:kspi0 K-:all', 'Mbc > 5.25 and abs(deltaE) < 0.15')
# vertexKFit('B-:dk', 0.0)
matchMCTruth('B-:dk')


toolsBDk = ['EventMetaData', 'B-']
toolsBDk += ['InvMass', '^B- -> [^D0:kspi0 -> ^K_S0:all pi0:all] ^K-:all']
toolsBDk += ['InvMass[BeforeFit]', 'B- -> [D0:kspi0 -> K_S0:all ^pi0:all] K-:all']
toolsBDk += ['DeltaEMbc', '^B-']
# toolsBDk += ['CustomFloats[isSignal]', '^B-']

outputFile = "../1213012101.ntup.root"
ntupleFile(outputFile)
ntupleTree('bdk', 'B-:dk', toolsBDk)

set_log_level(LogLevel.INFO)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
