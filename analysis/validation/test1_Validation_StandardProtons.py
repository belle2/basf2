#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../mdst-xi-lambda.root</input>
  <output>../PIDProtons.ntup.root</output>
  <contact>Jan Strube jan.strube@desy.de</contact>
</header>
"""

#######################################################
#
# ccbar -> Xi- anything
#           |
#           +-> Labmda:ppi pi-
#                |
#                +-> p+ pi-
#
######################################################

from basf2 import *
from vertex import *
from modularAnalysis import *
from reconstruction import *
from stdFSParticles import *
from variables import variables

variables.addAlias('piExpertPID_ALL', 'pidProbabilityExpert(211, ALL)')
variables.addAlias('muExpertPID_ALL', 'pidProbabilityExpert(13, ALL)')
variables.addAlias('eExpertPID_ALL', 'pidProbabilityExpert(11, ALL)')
variables.addAlias('KExpertPID_ALL', 'pidProbabilityExpert(321, ALL)')
variables.addAlias('pExpertPID_ALL', 'pidProbabilityExpert(2212, ALL)')

# load input ROOT file
inputMdst('default', '../mdst-xi-lambda.root')

set_log_level(LogLevel.ERROR)

# --------------------------------------------------
# Create and fill final state ParticleLists
# --------------------------------------------------

loadStdCharged()
# reconstruct Lambda:ppi
reconstructDecay('Lambda0:ppi -> p+:all pi-:all', '1.100 < M < 1.130')
massVertexRave('Lambda0:ppi', 0.001)

# reconstruct the Xi+ from the Lambda:ppi and pi+:all
fillParticleList('pi+:xi', 'chiProb > 0.001 and abs(d0) < 2 and abs(z0) < 4')
reconstructDecay('Xi-:sig -> Lambda0:ppi pi-:xi', '1.300 < M < 1.340')
vertexRave('Xi-:sig', 0.001)


# --------------------------------------------------
# write out useful information to a ROOT file
# --------------------------------------------------

# information to be saved to file
toolsnu = ['EventMetaData', '^Xi-']
toolsnu += ['Kinematics', '^Xi- -> [^Lambda0 -> ^p+:all ^pi-:all] ^pi-:xi']
toolsnu += ['InvMass[BeforeFit]', '^Xi- -> [^Lambda0 -> p+:all pi-:all] pi-:xi']
toolsnu += ['Vertex', '^Xi- -> [^Lambda0 -> p+:all pi-:all] pi-:xi']
toolsnu += ['FlightInfo', '^Xi- -> [^Lambda0 -> p+:all pi-:all] pi-:xi']
toolsnu += ['PID', 'Xi- -> [Lambda0 -> ^p+:all ^pi-:all] ^pi-:xi']
toolsnu += ['CustomFloats[piExpertPID_ALL:muExpertPID_ALL:eExpertPID_ALL:KExpertPID_ALL:pExpertPID_ALL]',
            'Xi- -> [Lambda0 -> ^p+:all ^pi-:all] ^pi-:xi']

# write out the flat ntuple
ntupleFile('../PIDProtons.ntup.root')
ntupleTree('xitree', 'Xi-:sig', toolsnu)


# --------------------------------------------------
# Process the events and print call statistics
# --------------------------------------------------

process(analysis_main)
print(statistics)
