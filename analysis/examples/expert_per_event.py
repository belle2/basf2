#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Meant to be run after analysis/examples/teacher_per_event.py, attaches expert
# output to each Event(as EventExtraInfo) and saves the result in a file

import sys
import os
from basf2 import *
from modularAnalysis import *

main = create_path()

main.add_module(register_module('RootInput'))

methods = ['Fisher', 'FastBDT', 'BDTGradient', 'PDEFoamBoost']

for method in methods:
    expert = register_module('TMVAExpert')
    expert.param('prefix', 'TMVA')
    expert.param('method', method)
    expert.param('signalProbabilityName', method + '_Probability')
    main.add_module(expert)

output = register_module('VariablesToNtuple')
output.param('variables', ['getExtraInfo({method}_Probability)'.format(method=method) for method in methods] + ['isContinuumEvent'])
output.param('fileName', 'expert_per_event.root')
output.param('treeName', 'variables')
main.add_module(output)

process(main)
print statistics
