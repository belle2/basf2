#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Meant to be run after analysis/examples/teacher_per_candidate.py, attaches expert
# output to each Particle (as extra info) and saves the result in a file

import sys
import os
from basf2 import *
from modularAnalysis import *

main = create_path()

main.add_module(register_module('RootInput'))
main.add_module(register_module('ParticleLoader'))
selectParticle('e-', path=main)

methods = ['Fisher', 'FastBDT', 'BDTGradient', 'PDEFoamBoost']

for method in methods:
    expert = register_module('TMVAExpert')
    expert.param('prefix', 'TMVA')
    expert.param('method', method)
    expert.param('listNames', 'e-')
    expert.param('signalProbabilityName', method + '_Probability')
    main.add_module(expert)

output = register_module('VariablesToNtuple')
output.param('particleList', 'e-')
output.param('variables', ['extraInfo({method}_Probability)'.format(method=method) for method in methods])
output.param('fileName', 'expert_per_candidate.root')
output.param('treeName', 'variables')
main.add_module(output)

process(main)
print statistics
