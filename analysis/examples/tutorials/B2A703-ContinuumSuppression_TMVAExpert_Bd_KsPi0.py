#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################################
#
# This tutorial demonstrates how to apply the TMVA expert
# and store the (transformed) network output for
# Bd->KsPi0 decays prepared with:
# B2A702-ContinuumSuppression_TMVATraining_Bd_KsPi0.py
#
# The expert needs to read in the files produced in the
# previous tutorial (written out to /training_qq_<test,full>).
#
# Note: This example uses a privately produced skim of
# the continuum and generic MC sample created with no beam
# background (BGx0) in MC campaign 3.5, therefore it can
# only be run on KEKCC computers.
#
# Usage:
#  ./B2A703-ContinuumSuppression_TMVAExpert_Bd_KsPi0.py <signal,continuum,generic> <test,full>
#
# Contributors: P. Goldenzweig (January 2015)
#
################################################################################

import sys

# Command-line usage
if len(sys.argv) != 3:
    sys.exit('Specify sample to process. Either `signal`, `continuum`, or `generic`. \n'
             'Specify sampleSize to process. Either `test` or `full`. \n'
             'Usage: ./expert_Bd_KsPi0.py sample sampleSize')

sample = str(sys.argv[1])
sampleSize = str(sys.argv[2])

if sample not in ('signal', 'continuum', 'generic'):
    sys.exit('Input sample does not match any of the available samples: `signal`, `continuum`, or `generic`')

if sampleSize not in ('test', 'full'):
    sys.exit('Input sampleSize does not match any of the available samples: `test` or `full`')

import os.path
from basf2 import *
from modularAnalysis import *
from FlavorTagger import *
from stdFSParticles import stdPi0
from stdV0s import stdKshorts


# _____________________________________________________________________________________________________________________________________
# Bd -> Ks Pi0 signal MC privately produced with BGx0.
testSIG = ['/gpfs/fs02/belle2/users/pablog/mcGeneration_belle2/Bd_KsPi0_BGx0_2014/root/*30*']
fullSIG = ['/gpfs/fs02/belle2/users/pablog/mcGeneration_belle2/Bd_KsPi0_BGx0_2014/root/*']

# Bd -> Ks Pi0 skim of BGx0 MC 3.5 continuum MC.
testCC = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/ccbar/mc35_ccbar_BGx0_s03.mdst.root']
testSS = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/ssbar/mc35_ssbar_BGx0_s03.mdst.root']
testDD = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/ddbar/mc35_ddbar_BGx0_s03.mdst.root']
testUU = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/uubar/mc35_uubar_BGx0_s03.mdst.root']
fullCC = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/ccbar/*']
fullSS = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/ssbar/*']
fullDD = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/ddbar/*']
fullUU = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/uubar/*']

# Bd -> Ks Pi0 skim of BGx0 MC 3.5 generic MC. (Dataset too small to make a 'test' sample).
fullCH = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/charged/*']
fullMX = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/mixed/*']
# _____________________________________________________________________________________________________________________________________


outFile = ''

if sample == 'signal':
    if sampleSize == 'test':
        inputMdstList('MC5', testSIG)
        outFile = 'Bd_KsPi0_reconstructedSignalMC_test.root'
    else:
        inputMdstList('MC5', fullSIG)
        outFile = 'Bd_KsPi0_reconstructedSignalMC_full.root'
elif sample == 'continuum':
    if sampleSize == 'test':
        inputMdstList('MC5', testCC + testSS + testDD + testUU)
        outFile = 'Bd_KsPi0_reconstructedContinuumMC_test.root'
    else:
        inputMdstList('MC5', fullCC + fullSS + fullDD + fullUU)
        outFile = 'Bd_KsPi0_reconstructedContinuumMC_full.root'
else:
    inputMdstList('MC5', fullCH + fullMX)
    outFile = 'Bd_KsPi0_reconstructedGenericMC.root'


# Do the analysis
fillParticleList('gamma:all', '')
fillParticleList('pi+:good', 'chiProb > 0.001 and piid > 0.5 and abs(dz) < 4.0 and abs(dr) < 0.2')
fillParticleList('pi-:good', 'chiProb > 0.001 and piid > 0.5 and abs(dz) < 4.0 and abs(dr) < 0.2')

reconstructDecay('K_S0 -> pi+:good pi-:good', '0.480<=M<=0.516', 1)
reconstructDecay('pi0  -> gamma:all gamma:all', '0.115<=M<=0.152', 1)
reconstructDecay('B0   -> K_S0 pi0', '5.2 < Mbc < 5.3 and -0.3 < deltaE < 0.2')

matchMCTruth('B0')
buildRestOfEvent('B0')
buildContinuumSuppression('B0')
TagV('B0', 'breco')


# Define the methods
methods = ['FastBDT', 'NeuroBayes']


# TMVA expert
for method in methods:
    expert = register_module('TMVAExpert')
    expert.param('prefix', 'Bd_KsPi0_TMVA')
    expert.param('method', method)
    expert.param('listNames', ['B0'])
    expert.param('expertOutputName', method + '_Probability')
    expert.param('workingDirectory', 'training_qq_' + sampleSize)
    analysis_main.add_module(expert)


# Network output
networkOutput = ['extraInfo({method}_Probability)'.format(method=method) for method in methods]
transformedNetworkOutputNB = ['transformedNetworkOutput(NeuroBayes_Probability,-0.9,1.0)']
transformedNetworkOutputFBDT = ['transformedNetworkOutput(FastBDT_Probability,0.1,1.0)']


# NTupleTools including network output in tree.
toolsB = ['EventMetaData', '^B0']
toolsB += ['RecoStats', '^B0']
toolsB += ['DeltaEMbc', '^B0']
toolsB += ['TagVertex', '^B0']
toolsB += ['MCTagVertex', '^B0']
toolsB += ['DeltaT', '^B0']
toolsB += ['MCDeltaT', '^B0']
toolsB += ['InvMass[BeforeFit]', '^B0 -> [ ^K_S0 -> ^pi+:good ^pi-:good ]   ^pi0                           ']
toolsB += ['PID', ' B0 -> [  K_S0 -> ^pi+:good ^pi-:good ]    pi0                           ']
toolsB += ['Kinematics', '^B0 -> [ ^K_S0 -> ^pi+:good ^pi-:good ] [ ^pi0 -> ^gamma:all ^gamma:all ]']
toolsB += ['MCTruth', '^B0 -> [ ^K_S0 -> ^pi+:good ^pi-:good ] [ ^pi0 -> ^gamma:all ^gamma:all ]']
toolsB += ['MCHierarchy', '^B0 -> [ ^K_S0 -> ^pi+:good ^pi-:good ] [ ^pi0 -> ^gamma:all ^gamma:all ]']
toolsB += ['ContinuumSuppression', '^B0']
toolsB += ['CustomFloats[' + ':'.join(networkOutput) + ']', '^B0']
toolsB += ['CustomFloats[' + ':'.join(transformedNetworkOutputFBDT) + ']', '^B0']
toolsB += ['CustomFloats[' + ':'.join(transformedNetworkOutputNB) + ']', '^B0']


# Write flat ntuples
ntupleFile(outFile)
ntupleTree('ntupB0', 'B0', toolsB)


process(analysis_main)
print(statistics)
