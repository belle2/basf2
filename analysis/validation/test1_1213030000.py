#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../1213030000.dst.root</input>
  <output>../1213030000.ntup.root</output>
  <contact>Minakshi Nayak; minakshi.nayak@wayne.edu </contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from ROOT import Belle2
from modularAnalysis import inputMdstList
from glob import glob

inputMdstList('default', '../1213030000.dst.root')

fillParticleList('K+:all', 'chiProb > 0.001 and abs(d0) < 2 and abs(z0) < 4')
fillParticleList('pi+:all', 'chiProb > 0.001 and abs(d0) < 2 and abs(z0) < 4')


# reconstruct D0
reconstructDecay('D0:kpi -> K-:all pi+:all', '1.83 < M < 1.90')
matchMCTruth('D0:kpi')

# reconstruct B+
reconstructDecay('B-:dpi -> D0:kpi pi-:all', 'Mbc > 5.2 and abs(deltaE) < 0.3')
matchMCTruth('B-:dpi')


toolsBDpi = ['EventMetaData', 'B-']
toolsBDpi += ['InvMass', '^B- -> [^D0:kpi -> ^K-:all ^pi+:all] ^pi-:all']
toolsBDpi += ['DeltaEMbc', '^B-']
toolsBDpi += ['CMSKinematics', '^B- -> [^D0:kpi -> ^K-:all ^pi+:all] ^pi-:all']
toolsBDpi += ['PID', '^B- -> [^D0:kpi -> ^K-:all ^pi+:all] ^pi-:all']
toolsBDpi += ['Track', '^B- -> [^D0:kpi -> ^K-:all ^pi+:all] ^pi-:all']
toolsBDpi += ['MCTruth', '^B- -> [^D0:kpi -> ^K-:all ^pi+:all] ^pi-:all']

outputFile = "../1213030000.ntup.root"
ntupleFile(outputFile)
ntupleTree('bdpi', 'B-:dpi', toolsBDpi)

set_log_level(LogLevel.INFO)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
