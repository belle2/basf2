#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../mdst-dstars.root</input>
  <output>../ana-dstars.root</output>
  <contact>Jake Bennett; jvbennett@cmu.edu</contact>
</header>
"""

#######################################################
#
# Obtain K and pi samples for PID performance validation
# from D* decays, using the slow pion to tag the K or pi
#
# ccbar -> D*+ anything
#           |
#           +-> D0:kpi pi+
#                |
#                +-> K- pi+
#
#
# Contributors: Jake Bennett (June 2016)
#
######################################################

from basf2 import *
from vertex import *
from modularAnalysis import *
from reconstruction import *
from stdFSParticles import *

# load input ROOT file
inputMdst('default', '../mdst-dstars.root')

# --------------------------------------------------
# Create and fill final state ParticleLists
# --------------------------------------------------

fillParticleList('pi+:all', 'chiProb > 0.001 and abs(d0) < 2 and abs(z0) < 4')
fillParticleList('K+:all', 'chiProb > 0.001 and abs(d0) < 2 and abs(z0) < 4')


# reconstruct D0:kpi and perform a mass constrained vertex fit
reconstructDecay('D0:kpi -> K-:all pi+:all', '1.835 < M < 1.895')
vertexRave('D0:kpi', 0.001)

# reconstruct the D*+ from the D0:kpi and pi+:all
reconstructDecay('D*+:sig -> D0:kpi pi+:all', '1.7 < M < 2.3')


# --------------------------------------------------
# write out useful information to a ROOT file
# --------------------------------------------------

# information to be saved to file
toolsnu = ['EventMetaData', '^D*+:sig']
toolsnu += ['Kinematics', '^D*+:sig -> [^D0:kpi -> ^K-:all ^pi+:all] ^pi+:all']
toolsnu += ['InvMass[BeforeFit]', '^D*+:sig -> [^D0:kpi -> K-:all pi+:all] pi+:all']
toolsnu += ['PID', 'D*+:sig -> [D0:kpi -> ^K-:all ^pi+:all] ^pi+:all']
toolsnu += ['Charge', 'D*+:sig -> [D0:kpi -> ^K-:all ^pi+:all] ^pi+:all']

# write out the flat ntuple
ntupleFile('../ana-dstars.root')
ntupleTree('dsttree', 'D*+:sig', toolsnu)


# --------------------------------------------------
# Process the events and print call statistics
# --------------------------------------------------

process(analysis_main)
print(statistics)
