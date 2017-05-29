#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../2610030000.dst.root</input>
  <output>../2610030000.ntup.root</output>
  <contact>Jake Bennett; jvbennett@cmu.edu</contact>
</header>
"""

#######################################################
#
# Reconstruct D* tagged D0 for charm validation.
# Check proper time resolution,
# slow pion efficiency and impact parameters resolution
#
# ccbar -> D*+ anything
#           |
#           +-> D0:kpi pi+
#                |
#                +-> K- pi+
#
#
# Contributors: Giulia Casarosa (April 2017)
#
######################################################

from basf2 import *
from vertex import *
from modularAnalysis import *
from reconstruction import *
from stdFSParticles import *

# load input ROOT file
inputMdst('default', '../2610030000.dst.root')

# --------------------------------------------------
# Create and fill final state ParticleLists
# --------------------------------------------------

fillParticleList('pi+:all', 'chiProb > 0.001 and abs(d0) < 2 and abs(z0) < 4')
# creates "pi+:all" ParticleList (and c.c.)
copyList('pi+:slow', 'pi+:all')
applyCuts('pi+:slow', 'd0<1.5 and -10 < z0 < 10 and useCMSFrame(p) < 0.5 and pt > 0.1 ')
# creates "pi+:loose" ParticleList (and c.c.)
copyList('pi+:pt', 'pi+:all')
applyCuts('pi+:pt', 'pt > 0.1')

fillParticleList('K+:all', 'chiProb > 0.001 and abs(d0) < 2 and abs(z0) < 4')
copyList('K+:pt', 'K+:all')
applyCuts('K+:pt', 'pt > 0.1')

# reconstruct D0:kpi and perform a mass constrained vertex fit
reconstructDecay('D0:kpi -> K-:pt pi+:pt', 'useCMSFrame(p) > 2.4 and 1.81 < M < 1.91')
massVertexRave('D0:kpi', 0.001)

# reconstruct the D*+ from the D0:kpi and pi+:all
reconstructDecay('D*+:sig -> D0:kpi pi+:slow', '0 < Q < 0.02')
vertexRave('D*+:sig', 0.001, '', 'ipprofile')
applyCuts('D*+:sig', '0.0 < Q < 0.02')

# truth matching
matchMCTruth('D*+:sig')


# --------------------------------------------------
# write out useful information to a ROOT file
# --------------------------------------------------

# information to be saved to file
toolsDST = ['EventMetaData', '^D*+']
toolsDST += ['InvMass', '^D*+ -> [D0 -> ^K- pi+] pi+']
toolsDST += ['InvMass[BeforeFit]', 'D*+ -> ^D0 pi+']
toolsDST += ['CustomFloats[Q]', '^D*+ -> D0 pi+']
toolsDST += ['CustomFloats[isSignal]', '^D*+ -> D0 pi+']
toolsDST += ['Kinematics', '^D*+ -> [^D0 -> ^K- ^pi+] ^pi+']
toolsDST += ['CMSKinematics', '^D*+ -> [^D0 -> ^K- ^pi+] ^pi+']
toolsDST += ['Vertex', '^D*+ -> ^D0 pi+']
toolsDST += ['Track', 'D*+ -> [D0 -> ^K- ^pi+] ^pi+']
toolsDST += ['TrackHits', 'D*+ -> [D0 -> ^K- ^pi+] ^pi+']
toolsDST += ['FlightInfo', '^D*+ -> ^D0 pi+']

toolsDST += ['MCKinematics', '^D*+ -> [^D0 -> ^K- ^pi+] ^pi+']
toolsDST += ['MCVertex', '^D*+ -> ^D0 pi+']
toolsDST += ['MCFlightInfo', '^D*+ -> ^D0 pi+']
toolsDST += ['MCTruth', '^D*+ -> [^D0 -> ^K- ^pi+] ^pi+']

# write out the flat ntuple
ntupleFile('../2610030000.ntup.root')
ntupleTree('dst', 'D*+:sig', toolsDST)

summaryOfLists(['D*+:sig', 'D0:kpi', 'pi+:slow'])
# --------------------------------------------------
# Process the events and print call statistics
# --------------------------------------------------

analysis_main.add_module('Progress')

process(analysis_main)

print(statistics)
