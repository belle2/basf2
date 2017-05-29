#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../1213030101.dst.root</input>
  <output>../1213030101.ntup.root</output>
  <contact>Minakshi Nayak; minakshi.nayak@wayne.edu </contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from ROOT import Belle2
from modularAnalysis import inputMdstList
from glob import glob

inputMdstList('default', '../1213030101.dst.root')

# create and fill pi/K ParticleLists
fillParticleList('pi+:all', 'piid > 0.5 and d0 < 0.5 and abs(z0) < 1')
fillParticleList('K+:all', 'Kid > 0.5 and d0 < 0.5 and abs(z0) < 1')

# create Ks -> pi+ pi- list from V0
fillParticleList('K_S0:all', '0.3 < M < 0.7')
vertexKFit('K_S0:all', 0.0)
applyCuts('K_S0:all', '0.45 < M < 0.55')
matchMCTruth('K_S0:all')

# reconstruct D0, tracks originate from the same vertex
reconstructDecay('D0:kspipi -> K_S0:all pi+:all pi-:all', '1.85 < M < 1.88 and 1.8 < useCMSFrame(p) < 2.8')
vertexKFit('D0:kspipi', 0.0)
matchMCTruth('D0:kspipi')


# reconstruct B+
reconstructDecay('B-:dk -> D0:kspipi K-:all', 'Mbc > 5.25 and abs(deltaE) < 0.15')
vertexKFit('B-:dk', 0.0)
matchMCTruth('B-:dk')

# get the rest of the event:
# buildRestOfEvent('B-:dk')

# get tag vertex ('breco' is the type of MC association)
# TagV('B-:dk', 'breco')
# buildContinuumSuppression('B-:dk')


toolsBDk = ['EventMetaData', 'B-']
toolsBDk += ['InvMass', '^B- -> [^D0:kspipi -> ^K_S0:all pi+:all pi-:all] ^K-:all']
toolsBDk += ['DeltaEMbc', '^B-']
# toolsBDk += ['CustomFloats[isSignal]', '^B-']
# toolsBDk += ['CustomFloats[isNotContinuumEvent]', '^B-']
toolsBDk += ['CMSKinematics', '^B- -> [^D0:ch00 -> ^K_S0:all ^pi+:all ^pi-:all] ^K-:all']
toolsBDk += ['PID', '^B- -> [^D0:kspipi -> ^K_S0:all ^pi+:all ^pi-:all] ^K-:all']
toolsBDk += ['Track', '^B- -> [^D0:kspipi -> ^K_S0:all ^pi+:all ^pi-:all] ^K-:all']
toolsBDk += ['Dalitz', 'B- -> [^D0:kspipi -> ^K_S0:all ^pi+:all ^pi-:all] K-:all']
toolsBDk += ['MCTruth', '^B- -> [^D0:kspipi -> ^K_S0:all ^pi+:all ^pi-:all] ^K-:all']

outputFile = "../1213030101.ntup.root"
ntupleFile(outputFile)
ntupleTree('bdk', 'B-:dk', toolsBDk)

set_log_level(LogLevel.INFO)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
