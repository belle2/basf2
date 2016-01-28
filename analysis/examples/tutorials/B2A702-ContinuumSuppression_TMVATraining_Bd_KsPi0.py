#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################################
#
# This tutorial demonstrates how to perform a
# TMVA training on continuum suppression variables
# for B0 candidates in Bd->KsPi0 decays using the following
# input variables:
#  KSFW moments, thrust related quantities,
#  R2, and CLEO Cones.
#
# 2 TMVA methods are used: FastBDT, and NeuroBayes.
#
# One can apply the TMVAExpert and prepare the ntuple for
# analysis using:
# B2A703-ContinuumSuppression_TMVAExpert_Bd_KsPi0.py
#
# Note: This example uses a privately produced skim of
# the continuum MC sample created with no beam background (BGx0) in
# MC campaign 3.5, therefore it can be run only on KEKCC computers.
#
# To run NeuroBayes on KEKCC execute the following before running:
#  export PHIT_LICENCE_PATH=${PHIT_LICENCE_PATH=${SWBELLE}/local/etc/neurobayes}
#  export PHIT_LICENCE_SERVER=bweb3.cc.kek.jp:16820
#  source /sw/belle/local/neurobayes/belle_default_64bit/setup_neurobayes.sh
#
# Usage:
#  basf2 B2A702-ContinuumSuppression_TMVATraining_Bd_KsPi0.py <test,full>
#
# This will produce /training_qq_<test,full> directories containing
# the training output
#
# Important NOTE:
#    This updated version sets the target to:
#    teacher.param('target', 'isNotContinuumEvent')
#    Previously, the target was set to 'isSignal', but this was incorrect,
#    as it only set MCTruth matched signal as signal, and therefore characterized
#    all misreconstructed signal as background.
#
# Contributors: P. Goldenzweig (January 2015)
#
################################################################################


import sys

# Command-line usage
if len(sys.argv) != 2:
    sys.exit(
        '\nSpecify sampleSize to process. Either `test` or `full`. \n\n'
        '`test` will train on a subset of the skim (~400 B->KsPi0 signal and ~400 qq bkgd) and will take ~2 min to complete. \n'
        '       WARNING: Not a good training!! Only useful for testing. \n\n'
        '`full` will train on the full skim and will take ~1 hour to complete. \n\n'
        'Usage: ./training_Bd_KsPi0.py sampleSize \n')

sampleSize = str(sys.argv[1])

import os
from basf2 import *
from modularAnalysis import *
from stdFSParticles import stdPi0
from stdV0s import stdKshorts


# _____________________________________________________________________________________________________________________________________
# Test skim of BGx0 MC 3.5    -    (2 min to process)
testSIG1 = [
    '/gpfs/fs02/belle2/users/pablog/mcGeneration_belle2/Bd_KsPi0_BGx0_2014_500k/root/EvtGenSimRec_Bd_KsPi0_noBeamBkgd_*10*.root']
testSIG2 = [
    '/gpfs/fs02/belle2/users/pablog/mcGeneration_belle2/Bd_KsPi0_BGx0_2014_500k/root/EvtGenSimRec_Bd_KsPi0_noBeamBkgd_*20*.root']
testCC = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/ccbar/mc35_ccbar_BGx0_s01.mdst.root']
testSS = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/ssbar/mc35_ssbar_BGx0_s01.mdst.root']
testDD = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/ddbar/mc35_ddbar_BGx0_s01.mdst.root']
testUU = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/uubar/mc35_uubar_BGx0_s01.mdst.root']

# Full skim of BGx0 MC 3.5    -    (1 hour to process)
fullSIG = ['/gpfs/fs02/belle2/users/pablog/mcGeneration_belle2/Bd_KsPi0_BGx0_2014_500k/root/*']
fullCC = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/ccbar/*']
fullSS = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/ssbar/*']
fullDD = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/ddbar/*']
fullUU = ['/gpfs/fs02/belle2/users/pablog/analysis/rec_Bd_K0Pi0_BGx0_MC35_2014/Bd_KsPi0/mdst/uubar/*']
# _____________________________________________________________________________________________________________________________________


if sampleSize == 'test':
    inputMdstList(testSIG1 + testSIG2 + testCC + testSS + testDD + testUU)
elif sampleSize == 'full':
    inputMdstList(fullSIG + fullCC + fullSS + fullDD + fullUU)
else:
    sys.exit('sampleSize does not match any of the available samples: `test` or `full`')


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


# Define the input variables for training.
variables = [
    'R2',
    'cosTBTO',
    'KSFWVariables(hso02)',
    'KSFWVariables(hso12)',
    'cosTBz',
    'CleoCone(9)',
    'thrustBm',
    'thrustOm',
    'KSFWVariables(et)',
    'KSFWVariables(mm2)',
    'KSFWVariables(hso00)',
    'KSFWVariables(hso04)',
    'KSFWVariables(hso10)',
    'KSFWVariables(hso14)',
    'KSFWVariables(hso20)',
    'KSFWVariables(hso22)',
    'KSFWVariables(hso24)',
    'KSFWVariables(hoo0)',
    'KSFWVariables(hoo1)',
    'KSFWVariables(hoo2)',
    'KSFWVariables(hoo3)',
    'KSFWVariables(hoo4)',
    'CleoCone(1)',
    'CleoCone(2)',
    'CleoCone(3)',
    'CleoCone(4)',
    'CleoCone(5)',
    'CleoCone(6)',
    'CleoCone(7)',
    'CleoCone(8)'
]


# Define the methods
methods = [
    ('FastBDT', 'Plugin', 'H:V:CreateMVAPdfs:NbinsMVAPdf=100:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3'),
    ('NeuroBayes', 'Plugin', 'H:V:CreateMVAPdfs:NbinsMVAPdf=100:NtrainingIter=20:Preprocessing=612:ShapeTreat=DIAG:'
     'TrainingMethod=BFGS')
]


# Create directory for TMVA teacher output
outDirForTMVA = 'training_qq_' + sampleSize
if not os.path.exists(outDirForTMVA):
    os.makedirs(outDirForTMVA)


# TMVA training/testing
teacher = register_module('TMVAOnTheFlyTeacher')
teacher.param('prefix', 'Bd_KsPi0_TMVA')
teacher.param('methods', methods)
teacher.param('variables', variables)
teacher.param('target', 'isNotContinuumEvent')
teacher.param('listNames', ['B0'])
teacher.param('workingDirectory', outDirForTMVA)
analysis_main.add_module(teacher)


process(analysis_main)
print(statistics)
