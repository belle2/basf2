#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../2210022100.dst.root</input>
  <output>../2210022100.ntup.root</output>
  <contact>Jake Bennett; jvbennett@cmu.edu</contact>
</header>
"""

#######################################################
#
# D0 sample for charm validation.
#
# ccbar -> D0 -> Ks pi0
#
#
# Contributors: Giulia Casarosa (April 2017)
#
######################################################

from basf2 import *
from vertex import *
from modularAnalysis import *
from reconstruction import *
from stdPi0s import *
from stdV0s import *

# load input ROOT file
inputMdst('default', '../2210022100.dst.root')

# --------------------------------------------------
# Create and fill final state ParticleLists
# --------------------------------------------------

loadStdKS()
stdPi0s('veryLoose')

# reconstruct D0:kpi and perform a mass constrained vertex fit
reconstructDecay('D0:sig -> K_S0:all pi0:veryLoose', 'useCMSFrame(p) > 2.4 and 1.81 < M < 1.91')
massVertexRave('D0:sig', 0.001)

# truth matching
matchMCTruth('D0:sig')

# --------------------------------------------------
# write out useful information to a ROOT file
# --------------------------------------------------

# information to be saved to file
toolsDZ = ['EventMetaData', '^D*+']
toolsDZ += ['InvMass', '^D0 -> K_S0 ^pi0']
toolsDZ += ['InvMass[BeforeFit]', '^D0 -> ^K_S0 ^pi0']
toolsDZ += ['CustomFloats[isSignal]', '^D0 -> K_S0 pi0']
toolsDZ += ['Kinematics', '^D0 -> ^K_S0 ^pi0']
toolsDZ += ['CMSKinematics', '^D0 -> ^K_S0 ^pi0']
toolsDZ += ['Vertex', '^D0 -> K_S0 pi0']
toolsDZ += ['FlightInfo', '^D0 -> ^K_S0 pi0']

toolsDZ += ['MCKinematics', '^D0 -> ^K_S0 ^pi0']
toolsDZ += ['MCVertex', '^D0 -> K_S0 pi0']
toolsDZ += ['MCFlightInfo', '^D0 -> ^K_S0 pi0']
toolsDZ += ['MCTruth', '^D0 -> K_S0 pi0']

# write out the flat ntuple
ntupleFile('../2210022100.ntup.root')
ntupleTree('dz', 'D0:sig', toolsDZ)

summaryOfLists(['D0:sig', 'K_S0:all', 'pi0:veryLoose'])
# --------------------------------------------------
# Process the events and print call statistics
# --------------------------------------------------

analysis_main.add_module('Progress')

process(analysis_main)

print(statistics)
