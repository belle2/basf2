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
# Selection criteria in selectParticle and applyCuts are specified using ranges:
#
# 'M 1.8:1.9'       means 1.8 <= M <= 1.9
# 'M 1.8:'          means M >= 1.8
# 'M :1.9'          means M <= 1.9
# 'dM +-0.1'        means abs(dM) <= 0.1
# 'dM +-(0.1:0.2)'  means 0.1 <= abs(dM) <= 0.2 (e.g. sidebands in M)
#
# More than one range can be specified for a single variable:
# 'dM +-0.1 +-(0.15:0.25)' to select events in signal and sideband region
#
# Cut on variable of daughter particle:
# M.1.2 means 2nd daugher of 1st daugher (e.g. K+ from D0 -> K- K+ in D*+ decay chain)
# Note: daugher numbering starts from 1
#
# selectParticle and applyCuts can take a list of cuts:
# applyCuts('D*+',['dM.1 +-0.1 +-(0.15:0.25)','dQ +-0.001'])
# meaning: (abs(dM.1) <= 0.1 or 0.15 <= abs(dM.1) <= 0.25) and abs(dQ) <= 0.001
#
# Available selection variables are printed at the beginning of execution
#
# Selection criteria converted to the usual expressions can be printed out by
# processing one event at logLevel INFO: basf2 recDstarEvtGen.py -n 1 -l INFO
#
# ----------------------------------------------------------------------------------

# check if the required input file exists
import os.path
import sys
if not os.path.isfile('evtgen-Dstar.root'):
    sys.exit('evtgen-Dstar.root file does not exist. Please run analysis/examples/ContinuumGenOnly.py script first.'
             )

inputMdst('evtgen-Dstar.root')
loadMCParticles()

selectParticle('K-', -321, [])
selectParticle('pi+', 211, [])
selectParticle('g', 22, [])

makeParticle('pi0', 111, ['g', 'g'], 0.110, 0.150)

applyCuts('pi0', ['p.1 0.1:', 'p.2 0.1:'])

makeParticle('D0', 421, ['K-', 'pi+', 'pi0'], 1.700, 2.000)
makeParticle('D*+', 413, ['D0', 'pi+'], 1.900, 2.100)

applyCuts('D*+', ['M.1 1.81:1.91', 'Q :0.02', 'p_CMS 2.0:'])

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

process(main)
print statistics

