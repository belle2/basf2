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
# processing one event at logLevel INFO: basf2 recDstarFullSim.py -n 1 -l INFO
#
# ----------------------------------------------------------------------------------

# check if the required input file exists
import os.path
import sys
if not os.path.isfile('output.root'):
    sys.exit('output.root file does not exist. Please run reconstruction/examples/example.py script first.'
             )

inputMdst('output.root')
loadReconstructedParticles()

selectParticle('K-', -321, ['chiProb 0.001:', 'Kid 0.5:'])
selectParticle('pi+', 211, ['chiProb 0.001:', 'piid 0.5:'])
selectParticle('pi_slow+', 211, ['chiProb 0.001:'])

makeParticle('D0', 421, ['K-', 'pi+'], 1.7, 2.0)
fitVertex('D0', 0.001)
applyCuts('D0', ['M 1.81:1.91'])

makeParticle('D*+', 413, ['D0', 'pi_slow+'], 1.9, 2.1)
fitVertex('D*+', 0.001)
applyCuts('D*+', ['Q :0.02'])
# applyCuts('D*+',['Q :0.02', 'p_CMS 2.5:']) # to select charm (but no EvtGen at the moment)

# uncomment to investigate the content of the list
# printList('D*+',True)

# define tools for flat ntuples
toolsD0 = ['Kinematics', '^D0 -> ^K- ^pi+']
toolsD0 += ['PID', 'D0 -> ^K- ^pi+']
toolsD0 += ['MCTruth', '^D0 -> ^K- ^pi+']

toolsDst = ['Kinematics', '^D*+ -> ^D0 ^pi+']
toolsDst += ['MCTruth', '^D*+ -> ^D0 ^pi+']

# write flat ntuples
ntupleFile('ntuplesDstar.root')
ntupleTree('ntupD0', 'D0', toolsD0)
ntupleTree('ntupDst', 'D*+', toolsDst)

# uncomment to write microDst
# outputMdst('recDstarFull.root')

process(main)
print statistics
