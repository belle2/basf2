#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to apply the TMVA expert
# and store the (transformed) network output for
# Bd->KsPi0 decays prepared with:
# B2A702-ContinuumSuppression_TMVATraining_Bd_KsPi0.py
#
# The expert needs to read in the files produced in the
# previous tutorial (written out to '/training/').
#
# This tutorial stores both sets of KSFW moments, but the
# user can change this in the ContinuumSuppression tool (L113/114)
#
# Note: This example uses a privately produced skim of
# the continuum and generic MC sample created with no beam background in
# MC campaign 3.5, therefore it can be run only on KEKCC computers.
#
# Usage:
#  ./B2A703-ContinuumSuppression_TMVAExpert_Bd_KsPi0.py signal
#  ./B2A703-ContinuumSuppression_TMVAExpert_Bd_KsPi0.py continuum
#  ./B2A703-ContinuumSuppression_TMVAExpert_Bd_KsPi0.py generig

# Contributors: P. Goldenzweig (January 2015)
#
######################################################

import sys

if len(sys.argv) != 2:
    sys.exit('Specify sample to process. Either `signal`, `continuum`, or `generic`. \nUsage: ./expert_Bd_KsPi0.py sample'
             )

import os.path
from basf2 import *
from modularAnalysis import *
from stdFSParticles import stdPi0
from stdV0s import stdKshorts

sample = str(sys.argv[1])
outFile = ''

## Bd -> Ks Pi0 signal MC.
# (Signal MC privately produced with NO beam background under v00-04-01)
filelistSIG = \
    ['/gpfs/fs02/belle/users/pablog/mcGeneration_belle2/Bd_KsPi0_noBeamBkgd/root/*'
     ]

## Bd -> Ks Pi0 skim of continuum MC.
# (Continuum MC produced in mcprod1405 campaign with NO beam background)
filelistCC = \
    ['/gpfs/fs02/belle/users/pablog/analysis/rec_Bd_K0Pi0_noBeamBkgd/Bd_KsPi0/mdst/ccbar/*'
     ]
filelistSS = \
    ['/gpfs/fs02/belle/users/pablog/analysis/rec_Bd_K0Pi0_noBeamBkgd/Bd_KsPi0/mdst/ssbar/*'
     ]
filelistDD = \
    ['/gpfs/fs02/belle/users/pablog/analysis/rec_Bd_K0Pi0_noBeamBkgd/Bd_KsPi0/mdst/ddbar/*'
     ]
filelistUU = \
    ['/gpfs/fs02/belle/users/pablog/analysis/rec_Bd_K0Pi0_noBeamBkgd/Bd_KsPi0/mdst/uubar/*'
     ]

## Bd -> Ks Pi0 skim of generic MC.
# (Continuum MC produced in mcprod1405 campaign with NO beam background)
filelistCH = \
    ['/gpfs/fs02/belle/users/pablog/analysis/rec_Bd_K0Pi0_noBeamBkgd/Bd_KsPi0/mdst/charged/*'
     ]
filelistMX = \
    ['/gpfs/fs02/belle/users/pablog/analysis/rec_Bd_K0Pi0_noBeamBkgd/Bd_KsPi0/mdst/mixed/*'
     ]

if sample == 'signal':
    inputMdstList(filelistSIG)
    outFile = 'Bd_KsPi0_reconstructedSignalMC.root'
elif sample == 'continuum':
    inputMdstList(filelistCC + filelistSS + filelistDD + filelistUU)
    outFile = 'Bd_KsPi0_reconstructedContinuumMC.root'
elif sample == 'generic':
    inputMdstList(filelistCH + filelistMX)
    outFile = 'Bd_KsPi0_reconstructedGenericMC.root'
else:
    sys.exit('Input sample does not match any of the availible samples: `signal`, `continuum`, or `generic`'
             )

# Do the analysis
fillParticleList('gamma:all', '')
fillParticleList('pi+:good',
                 'chiProb > 0.001 and piid > 0.5 and abs(dz) < 4.0 and abs(dr) < 0.2'
                 )
fillParticleList('pi-:good',
                 'chiProb > 0.001 and piid > 0.5 and abs(dz) < 4.0 and abs(dr) < 0.2'
                 )

reconstructDecay('K_S0 -> pi+:good pi-:good', '0.480<=M<=0.516', 1)
reconstructDecay('pi0  -> gamma:all gamma:all', '0.115<=M<=0.152', 1)
reconstructDecay('B0   -> K_S0 pi0', '5.2 < Mbc < 5.3 and -0.3 < deltaE < 0.2')

matchMCTruth('B0')
buildRestOfEvent('B0')
buildContinuumSuppression('B0')

# Run expert
methods = ['FastBDT', 'NeuroBayes']

for method in methods:
    expert = register_module('TMVAExpert')
    expert.param('prefix', 'Bd_KsPi0_TMVA')
    expert.param('method', method)
    expert.param('listNames', ['B0'])
    expert.param('expertOutputName', method + '_Probability')
    expert.param('workingDirectory', 'training')
    analysis_main.add_module(expert)

# Network output
networkOutput = ['extraInfo({method}_Probability)'.format(method=method)
                 for method in methods]
transformedNetworkOutputNB = \
    ['transformedNetworkOutput(NeuroBayes_Probability,-0.9,1.0)']
transformedNetworkOutputFBDT = \
    ['transformedNetworkOutput(FastBDT_Probability,0.1,1.0)']

# NTupleTools including (transformed) network output in tree.
toolsB = ['EventMetaData', '^B0']
toolsB += ['RecoStats', '^B0']
toolsB += ['DeltaEMbc', '^B0']
toolsB += ['PID',
           ' B0 -> [  K_S0 -> ^pi+:good ^pi-:good ]    pi0                           '
           ]
toolsB += ['Cluster',
           ' B0 ->    K_S0                          [  pi0 -> ^gamma:all ^gamma:all ]'
           ]
toolsB += ['Kinematics',
           '^B0 -> [ ^K_S0 -> ^pi+:good ^pi-:good ] [ ^pi0 -> ^gamma:all ^gamma:all ]'
           ]
toolsB += ['MCTruth',
           '^B0 -> [ ^K_S0 -> ^pi+:good ^pi-:good ] [ ^pi0 -> ^gamma:all ^gamma:all ]'
           ]
toolsB += ['MCHierarchy',
           '^B0 -> [ ^K_S0 -> ^pi+:good ^pi-:good ] [ ^pi0 -> ^gamma:all ^gamma:all ]'
           ]
toolsB += ['ContinuumSuppression', '^B0']
toolsB += ['ContinuumSuppression[FS1]', '^B0']
toolsB += ['CustomFloats[' + ':'.join(networkOutput) + ']', '^B0']
toolsB += ['CustomFloats[' + ':'.join(transformedNetworkOutputFBDT) + ']',
           '^B0']
toolsB += ['CustomFloats[' + ':'.join(transformedNetworkOutputNB) + ']', '^B0']

# Write flat ntuples
ntupleFile(outFile)
ntupleTree('ntupB0', 'B0', toolsB)

process(analysis_main)
print statistics

