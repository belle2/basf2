#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Uses TMVATeacher module to train a classifier to do electron identification
# using a combination of available PID variables.
#
# Use 'showTMVAResults TMVA.root' to view detailed detailed information about
# a completed training

import sys
import os
from basf2 import *
from modularAnalysis import *

main = create_path()

main.add_module(register_module('RootInput'))
main.add_module(register_module('ParticleLoader'))

# Train TMVA Teacher using all electron candidates as input data
# Select the candidates
fillParticleList('e-', path=main)

# Define Variables
variables = ['eid', 'eid_dEdx', 'eid_TOP', 'eid_ARICH',
             'Kid', 'Kid_dEdx', 'Kid_TOP', 'Kid_ARICH',
             'prid', 'prid_dEdx', 'prid_TOP', 'prid_ARICH',
             'muid', 'muid_dEdx', 'muid_TOP', 'muid_ARICH',
             'p', 'pt', 'p_CMS', 'pt_CMS', 'chiProb']

# Define one or multiple methods.
# Every definition consists of 3 string. First the name of the method, secondly the type and thirdly a TMVA config string
methods = [
    ('FastBDT', 'Plugin',
     '!H:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3')
]

# Add TMVA Teacher to path
teacher = register_module('TMVATeacher')
teacher.param('prefix', 'TMVA')
teacher.param('methods', methods)
teacher.param('variables', variables)
teacher.param('target', 'isSignal')
teacher.param('listNames', 'e-')
teacher.param('doNotTrain', True)
main.add_module(teacher)

process(main)
print statistics

import subprocess
subprocess.call("externTeacher --methodName FastBDT  --methodType Plugin --methodConfig '!H:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3' --target isSignal --variables " + " ".join(variables), shell=True)
