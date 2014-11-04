#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to perform a
# TMVA training on continuum suppression variables
# for B0 candidates in B0->PhiK*0 decays using the following
# input variables:
#  KSFW moments, thrust related quantities,
#  R2, and CLEO Cones.
#
# 4 TMVA methods are used: Likelihood, BDT, FastBDT, and NBayes
#
# Note: This example uses a privately produced skim of
# the continuum MC sample created with no beam background in
# MC campaign 3.5, therefore it can be run only on KEKCC computers.
#
# Contributors: P. Goldenzweig (July 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdst
from modularAnalysis import loadReconstructedParticles
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from stdFSParticles import goodPhoton

from modularAnalysis import *

## FULL SKIM
filelistSIG = \
    ['/gpfs/fs02/belle/users/pablog/mcGeneration_belle2/Bd_PhiKst_noBeamBkgd/fL_1/root/*'
     ]
filelistCC = \
    ['/gpfs/fs02/belle/users/pablog/analysis/rec_Bd_PhiKst_noBeamBkgd_18July2014/mdst/ccbar/*'
     ]
filelistSS = \
    ['/gpfs/fs02/belle/users/pablog/analysis/rec_Bd_PhiKst_noBeamBkgd_18July2014/mdst/ssbar/*'
     ]
filelistDD = \
    ['/gpfs/fs02/belle/users/pablog/analysis/rec_Bd_PhiKst_noBeamBkgd_18July2014/mdst/ddbar/*'
     ]
filelistUU = \
    ['/gpfs/fs02/belle/users/pablog/analysis/rec_Bd_PhiKst_noBeamBkgd_18July2014/mdst/uubar/*'
     ]
inputMdstList(filelistSIG + filelistCC + filelistSS + filelistDD + filelistUU)

# Do the analysis
loadReconstructedParticles()

selectParticle('K+',
               'chiProb > 0.001 and Kid  > 0.5 and -4.0 < dz < 4.0 and -0.2 < dr < 0.2'
               )
selectParticle('pi-',
               'chiProb > 0.001 and piid > 0.5 and -4.0 < dz < 4.0 and -0.2 < dr < 0.2'
               )

reconstructDecay('K*0 -> K+ pi-', '0.75 < M < 1.25')
reconstructDecay('phi -> K+ K-', '1.00 < M < 1.05')

reconstructDecay('B0 -> K*0 phi', '5.20 < M < 5.40')

applyCuts('B0', '-0.05 < deltaE < 0.05')
applyCuts('B0', ' 5.24 < Mbc    < 5.29')

matchMCTruth('B0')
buildRestOfEvent('B0')
buildContinuumSuppression('B0')

# Define the input variables.
variables = [
    'R2',
    'cosTBTO',
    'KSFWVariables(k0hso02)',
    'KSFWVariables(k0hso12)',
    'cosTBz',
    'CleoCone(9)',
    'thrustBm',
    'thrustOm',
    'KSFWVariables(k0et)',
    'KSFWVariables(k0mm2)',
    'KSFWVariables(k0hso00)',
    'KSFWVariables(k0hso04)',
    'KSFWVariables(k0hso10)',
    'KSFWVariables(k0hso14)',
    'KSFWVariables(k0hso20)',
    'KSFWVariables(k0hso22)',
    'KSFWVariables(k0hso24)',
    'KSFWVariables(k0hoo0)',
    'KSFWVariables(k0hoo1)',
    'KSFWVariables(k0hoo2)',
    'KSFWVariables(k0hoo3)',
    'KSFWVariables(k0hoo4)',
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
# In this case Likelihood, BDT, FastBDT, and NBayes are defined.
# The last arguments are TMVA options.
# Refer to TMVA manual for instructions.
methods = [('LPCA', 'Likelihood',
           'H:V:!TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=PCA'
           ), ('BDTG', 'BDT',
           'H:V:CreateMVAPdfs:NTrees=100:Shrinkage=0.10:UseBaggedBoost:GradBaggingFraction=0.5:nCuts=256:MaxDepth=3:BoostType=Grad'
           ), ('FastBDT', 'Plugin',
           'H:V:CreateMVAPdfs:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3'
           ), ('NeuroBayes', 'Plugin',
           'H:V:CreateMVAPdfs:NtrainingIter=20:Preprocessing=612:ShapeTreat=DIAG:TrainingMethod=BFGS'
           )]
               # fei match
               # fei
               # ov

# At the moment the python wrapper function for the TMVATeacher module
# does not exist, therefore we have to register the module by ourselves
# and set all module parameters.
teacher = register_module('TMVATeacher')
teacher.param('prefix', 'Bd_PhiKst_TMVA')
teacher.param('methods', methods)
teacher.param('variables', variables)
teacher.param('target', 'isSignal')
teacher.param('listNames', ['B0'])
teacher.param('workingDirectory',
              '/home/belle/pablog/work/belle2/basf2/analysis/examples/Bd_PhiKst/training'
              )
analysis_main.add_module(teacher)

process(analysis_main)
print statistics

