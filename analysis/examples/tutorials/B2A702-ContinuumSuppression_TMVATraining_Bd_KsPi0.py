#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *
from stdFSParticles import stdPi0
from stdV0s import stdKshorts

#######################################################
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
# the continuum MC sample created with no beam background in
# MC campaign 3.5, therefore it can be run only on KEKCC computers.
#
# To run NeuroBayes on KEKCC execute the following before running:
#  export PHIT_LICENCE_PATH=${PHIT_LICENCE_PATH=${SWBELLE}/local/etc/neurobayes}
#  export PHIT_LICENCE_SERVER=bweb3.cc.kek.jp:16820
#  source /sw/belle/local/neurobayes/belle_default_64bit/setup_neurobayes.sh
#
# Contributors: P. Goldenzweig (January 2015)
#
######################################################

### SKIM
filelistSIG = \
    ['/gpfs/fs02/belle/users/pablog/mcGeneration_belle2/Bd_KsPi0_noBeamBkgd_500k/root/*'
     ]
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
inputMdstList(filelistSIG + filelistCC + filelistSS + filelistDD + filelistUU)

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

# Define the input variables.
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
    'CleoCone(8)',
    ]

# Define the methods.
methods = [('FastBDT', 'Plugin',
           'H:V:CreateMVAPdfs:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3'
           ), ('NeuroBayes', 'Plugin',
           'H:V:CreateMVAPdfs:NtrainingIter=20:Preprocessing=612:ShapeTreat=DIAG:TrainingMethod=BFGS'
           )]

# TMVA training/testing
teacher = register_module('TMVATeacher')
teacher.param('prefix', 'Bd_KsPi0_TMVA')
teacher.param('methods', methods)
teacher.param('variables', variables)
teacher.param('target', 'isSignal')
teacher.param('listNames', ['B0'])
teacher.param('workingDirectory', 'training')
analysis_main.add_module(teacher)

process(analysis_main)
print statistics

