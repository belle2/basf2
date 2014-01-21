#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from basf2 import *

if not os.path.isfile('/storage/6/cpulvermacher/mdst_large.root'):
    sys.exit('output.root file does not exist.')

# Create main path
main = create_path()

input = register_module('RootInput')
input.param('inputFileName', '/storage/6/cpulvermacher/mdst_large.root')
main.add_module(input)

particleloader = register_module('ParticleLoader')
main.add_module(particleloader)

selector = register_module('ParticleSelector')
selector.param('PDG', 11)
selector.param('ListName', 'e-')
main.add_module(selector)

# Test TMVA methods: http://root.cern.ch/svn/root/trunk/tmva/test/TMVAClassification.C

variables = [
    'eid',
    'eid_dEdx',
    'eid_TOP',
    'eid_ARICH',
    'Kid',
    'Kid_dEdx',
    'Kid_TOP',
    'Kid_ARICH',
    'prid',
    'prid_dEdx',
    'prid_TOP',
    'prid_ARICH',
    'muid',
    'muid_dEdx',
    'muid_TOP',
    'muid_ARICH',
    'p',
    'pt',
    'p_CMS',
    'pt_CMS',
    'chiProb',
    ]

methods = [('Fisher', 'Fisher',
           'H:!V:Fisher:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10'
           ), ('BDTGradient', 'BDT',
           '!H:!V:CreateMVAPdfs:NTrees=1000:BoostType=Grad:Shrinkage=0.10:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=20:MaxDepth=2'
           ), ('PDEFoamBoost', 'PDEFoam',
           '!H:!V:CreateMVAPdfs:Boost_Num=30:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T'
           ), ('NeuroBayes', 'NeuroBayes',
           '!H:V:CreateMVAPdfs:NTrainingIter=50:TrainingMethod=BFGS:NBIndiPreproFlagByVarname='
            + '=34,'.join([s.replace('*', 'C') for s in variables]) + '=34')]

teacher = register_module('TMVATeacher')
teacher.param('identifier', 'TMVA')
teacher.param('methods', methods)
teacher.param('variables', variables)
teacher.param('target', 'truth')
teacher.param('listNames', ['e-'])
main.add_module(teacher)

# ----> start processing of modules
process(main)

# ----> Print call statistics
print statistics
