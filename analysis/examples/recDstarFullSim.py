#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

# ----------------------------------------------------------------------------------
# Example of reconstruction D*+ -> D0 pi+, D0 -> K- pi+ using full simulation and
# reconstruction output file
#
# To run this script you need first to prepare a root file output.root
# using for example reconstruction/examples/example.py
#
# Charge conjugate combinations are always done.
#
# ----------------------------------------------------------------------------------

# check if the required input file exists
import os.path
import sys
if not os.path.isfile('output.root'):
    sys.exit('output.root file does not exist. Please run reconstruction/examples/example.py script first.')

inputMdst('output.root')
loadReconstructedParticles()

selectParticle('K-', 'chiProb > 0.001 and Kid > 0.5')
selectParticle('pi+', 'chiProb > 0.001 and piid > 0.5')
selectParticle('pi+:slow', 'chiProb > 0.001')

makeParticle('D0 -> K- pi+', '1.7 < M < 2.0')
fitVertex('D0', 0.001)
applyCuts('D0', '1.81 < M < 1.91')
matchMCTruth('D0')

makeParticle('D*+ -> D0 pi+:slow', '1.9 < M < 2.1')
fitVertex('D*+', 0.001)
applyCuts('D*+', 'Q < 0.02')
matchMCTruth('D*+')

# uncomment to investigate the content of the list
# printList('D*+',True)

# define tools for flat ntuples
toolsD0 = ['Kinematics', '^D0 -> ^K- ^pi+']
toolsD0 += ['PID', 'D0 -> ^K- ^pi+']
toolsD0 += ['MCTruth', '^D0 -> ^K- ^pi+']

toolsDst = ['Kinematics', '^D*+ -> ^D0 ^pi+:slow']
toolsDst += ['MCTruth', '^D*+ -> ^D0 ^pi+:slow']

# write flat ntuples
ntupleFile('ntuplesDstar.root')
ntupleTree('ntupD0', 'D0', toolsD0)
ntupleTree('ntupDst', 'D*+', toolsDst)

# uncomment to write microDst
# outputMdst('recDstarFull.root')

process(analysis_main)
print statistics
