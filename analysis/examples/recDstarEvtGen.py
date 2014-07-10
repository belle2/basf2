#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

# ----------------------------------------------------------------------------------
# Example of reconstruction D*+ -> D0 pi+, D0 -> K- pi+ pi0 using EvtGen output file
#
# To run this script you need first to prepare a root file evtgen-Dstar.root
# using for example analysis/examples/ContinuumGenOnly.py
#
# Charge conjugate combinations are always done.
#
# ----------------------------------------------------------------------------------

# check if the required input file exists
import os.path
import sys
if not os.path.isfile('evtgen-Dstar.root'):
    sys.exit('evtgen-Dstar.root file does not exist. Please run analysis/examples/ContinuumGenOnly.py script first.')

inputMdst('evtgen-Dstar.root')
loadMCParticles()

selectParticle('K-', {})
selectParticle('pi+', {})
selectParticle('gamma', {})

makeParticle('pi0 -> gamma gamma', '0.110 < M < 0.150')

applyCuts('pi0', '0.1 < daughter0(p) and 0.1 < daughter1(p)')

makeParticle('D0 -> K- pi+ pi0', '1.700 < M < 2.000')
makeParticle('D*+ -> D0 pi+', '1.900 < M < 2.100')

applyCuts('D*+', '1.81 < daughter0(M) < 1.91 and Q < 0.02 and 2.0 < p_CMS')
matchMCTruth('D*+')

# define tools for flat ntuples
toolsDst = ['Kinematics', '^D*+ -> ^D0 ^pi+']
toolsDst += ['MCTruth', '^D*+ -> ^D0 ^pi+']
toolsDst += ['MCHierarchy', 'D*+ -> [D0 -> K- pi+ ^pi0] pi+']
toolsDst += ['MCKinematics', 'D*+ -> [D0 -> ^K- ^pi+ ^pi0] ^pi+']
toolsDst += ['MCVertex', '^D*+ -> ^D0 pi+']

# write flat ntuples
ntupleFile('ntupleDstar.root')
ntupleTree('ntupDst', 'D*+', toolsDst)

process(analysis_main)
print statistics
