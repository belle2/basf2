#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to perform a
# TMVA training on pi0 candidates using the following
# input variables:
#  - CMS momentum of the pi0
#  - polar angle of the pi0
#  - estimated uncertainty of the gammagamma invariant mass
#  - angle between one of the photon's momentum in pi0 frame and pi0 momentum in lab frame
#  - energy of the first daughter photon
#  - energy of the second daughter photon
#  - E9/E25 of the first daughter photon
#  - E9/E25 of the second daughter photon
#
# Two TMVA methods are used: Likelihood and Boosted Decision Tree
#
# Note: This example uses the generic MC sample created in
# MC campaign 3.5, therefore it can be ran only on KEKCC computers.
#
# Contributors: A. Zupanc (June 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdst
from modularAnalysis import loadReconstructedParticles
from modularAnalysis import reconDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from stdFSParticles import goodPhoton

from modularAnalysis import *

filelistMIX = \
    ['/group/belle2/MC/generic/mixed/mcprod1405/BGx1/mc35_mixed_BGx1_s00/mixed_e0001r0010_s00_BGx1.mdst.root'
     ]
filelistCHG = \
    ['/group/belle2/MC/generic/charged/mcprod1405/BGx1/mc35_charged_BGx1_s00/charged_e0001r0010_s00_BGx1.mdst.root'
     ]
filelistCC = \
    ['/group/belle2/MC/generic/ccbar/mcprod1405/BGx1/mc35_ccbar_BGx1_s00/ccbar_e0001r0010_s00_BGx1.mdst.root'
     ]
filelistSS = \
    ['/group/belle2/MC/generic/ssbar/mcprod1405/BGx1/mc35_ssbar_BGx1_s00/ssbar_e0001r0010_s00_BGx1.mdst.root'
     ]
filelistDD = \
    ['/group/belle2/MC/generic/ddbar/mcprod1405/BGx1/mc35_ddbar_BGx1_s00/ddbar_e0001r0010_s00_BGx1.mdst.root'
     ]
filelistUU = \
    ['/group/belle2/MC/generic/uubar/mcprod1405/BGx1/mc35_uubar_BGx1_s00/uubar_e0001r0010_s00_BGx1.mdst.root'
     ]

inputMdstList(filelistMIX + filelistCHG + filelistCC + filelistSS + filelistDD
              + filelistUU)

# do the analysis
loadReconstructedParticles()

# create final state particle lists
# use standard-good photon candidates
# (good photons pass good ECL selection criteria)
# creates "gamma:good" list
goodPhoton()

# construct pi0 candidates
reconDecay('pi0 -> gamma:good gamma:good', {'M': (0.100, 0.180)})
# perform mass fit
massKFit('pi0', 0.0, '')
# perform MC matching
matchMCTruth('pi0')

# define the input variables
# all PSelectorFunctions/VariableManager
variables = [
    'p_CMS',
    'cosTheta',
    'ErrM',
    'decayAngle',
    'daughter0(E)',
    'daughter1(E)',
    'daughter0(clusterE9E25)',
    'daughter1(clusterE9E25)',
    ]

# define the methods
# in this case Likelihood and BDT are defined
# the last argument are TMVA options. Refer to
# TMVA manual for instructions.
methods = [('LPCA', 'Likelihood',
           '!H:!V:!TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=PCA'
           ), ('BDTG', 'BDT',
           '!H:!V:CreateMVAPdfs:NTrees=100:BoostType=Grad:Shrinkage=0.10:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=200:MaxDepth=2'
           )]

# at the moment the python wrapper function for the TMVATeacher module
# does not exist, therefore we have to register the module by ourselves
# and set all module parameters
teacher = register_module('TMVATeacher')
teacher.param('prefix', 'PI0-B2A501-TMVA')
teacher.param('methods', methods)
teacher.param('variables', variables)
teacher.param('target', 'isSignal')
teacher.param('listNames', ['pi0'])
teacher.param('trainOncePerJob', True)

analysis_main.add_module(teacher)

process(analysis_main)
print statistics

