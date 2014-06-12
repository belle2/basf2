#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Uses TMVATeacher module to train a classifier to do event classification.
# In this example the teacher trains a very simple continuum supression
#
# Use 'show_tmva_results TMVA.root' to view detailed detailed information about
# a completed training
# Root input file for this example can be downloaded here:
# http://www-ekp.physik.uni-karlsruhe.de/~tkeck/contiunuum_and_friends.root

import sys
import os
from basf2 import *
from modularAnalysis import *

main = create_path()

main.add_module(register_module('RootInput'))

# Define Variables, only event-type variables are allowed!
variables = ['nTracks', 'nECLClusters', 'nKLMClusters', 'ECLEnergy', 'KLMEnergy']

# Define one or multiple methods.
# Every definition consists of 3 string. First the name of the method, secondly the type and thirdly a TMVA config string
methods = [
    ('Fisher', 'Fisher',
     '!H:!V:Fisher:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10'),
    ('BDTGradient', 'BDT',
     '!H:!V:NTrees=100:BoostType=Grad:Shrinkage=0.10:GradBaggingFraction=0.5:nCuts=200:MaxDepth=2'),
    ('PDEFoamBoost', 'PDEFoam',
     '!H:!V:Boost_Num=10:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T'),
    ('FastBDT', 'Plugin',
     '!H:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3')
]

# Add TMVA Teacher to path
teacher = register_module('TMVATeacher')
teacher.param('prefix', 'TMVA')
teacher.param('methods', methods)
teacher.param('variables', variables)
teacher.param('target', 'isContinuumEvent')
teacher.param('trainOncePerJob', True)
main.add_module(teacher)

process(main)
print statistics

B2INFO("")
B2INFO("Training completed. Run 'show_tmva_results TMVA_1_vs_0.root' to view detailed"
       " information about the trained methods.")
