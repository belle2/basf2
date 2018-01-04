#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../2610030000.dst.root</input>
  <output>../PIDKpi.ntup.root</output>
  <contact>Jan Strube jan.strube@desy.de</contact>
</header>
"""

#######################################################
#
# ccbar -> D*+ anything
#           |
#           +-> D0:kpi pi+
#                |
#                +-> K- pi+
#
######################################################

from basf2 import *
from vertex import *
from modularAnalysis import *
from reconstruction import *
from stdFSParticles import *
from variables import variables

set_log_level(LogLevel.ERROR)

variables.addAlias('piExpertPID_ALL', 'pidProbabilityExpert(211, ALL)')
variables.addAlias('muExpertPID_ALL', 'pidProbabilityExpert(13, ALL)')
variables.addAlias('eExpertPID_ALL', 'pidProbabilityExpert(11, ALL)')
variables.addAlias('KExpertPID_ALL', 'pidProbabilityExpert(321, ALL)')
variables.addAlias('pExpertPID_ALL', 'pidProbabilityExpert(2212, ALL)')

# load input ROOT file
inputMdst('default', '../2610030000.dst.root')

# --------------------------------------------------
# Create and fill final state ParticleLists
# --------------------------------------------------

loadStdCharged()
# creates "pi+:all" ParticleList (and c.c.)
copyList('pi+:slow', 'pi+:all')
applyCuts('pi+:slow', 'd0<1.5 and -10 < z0 < 10 and useCMSFrame(p) < 0.5 and pt > 0.1 ')

# reconstruct D0:kpi and perform a mass constrained vertex fit
reconstructDecay('D0:kpi -> K-:all pi+:all', 'useCMSFrame(p) > 2.4 and 1.81 < M < 1.91')
massVertexRave('D0:kpi', 0.001)

# reconstruct the D*+ from the D0:kpi and pi+:all
reconstructDecay('D*+:sig -> D0:kpi pi+:slow', '0 < Q < 0.02')
vertexRave('D*+:sig', 0.001, '', 'ipprofile')
applyCuts('D*+:sig', '0.0 < Q < 0.02')

# --------------------------------------------------
# write out useful information to a ROOT file
# --------------------------------------------------

# information to be saved to file
toolsDST = ['EventMetaData', '^D*+']
toolsDST += ['InvMass', '^D*+ -> [D0 -> K- pi+] pi+']
toolsDST += ['InvMass[BeforeFit]', 'D*+ -> ^D0 pi+']
toolsDST += ['CustomFloats[Q]', '^D*+ -> D0 pi+']
toolsDST += ['Kinematics', '^D*+ -> [^D0 -> ^K- ^pi+] ^pi+']
toolsDST += ['PID', 'D*+ -> [D0 -> ^K- ^pi+] ^pi+']
toolsDST += ['CustomFloats[piExpertPID_ALL:muExpertPID_ALL:eExpertPID_ALL:KExpertPID_ALL:pExpertPID_ALL]',
             'D*+ -> [D0 -> ^K- ^pi+] ^pi+']

# write out the flat ntuple
ntupleFile('../PIDKpi.ntup.root')
ntupleTree('dst', 'D*+:sig', toolsDST)

summaryOfLists(['D*+:sig', 'D0:kpi', 'pi+:slow'])
# --------------------------------------------------
# Process the events and print call statistics
# --------------------------------------------------

process(analysis_main)

print(statistics)
