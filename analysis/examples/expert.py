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

methods = ['Fisher', 'NeuroBayes', 'BDTGradient', 'PDEFoamBoost']

for method in methods:
    expert = register_module('TMVAExpert')
    expert.param('identifier', 'TMVA')
    expert.param('method', method)
    expert.param('listNames', ['e-'])
    expert.param('target', method + '_Probability')
    main.add_module(expert)

    histMaker = register_module('HistMaker')
    histMaker.param('file', method + '_hist.root')
    mass = 511e-6
    histMaker.param('histVariables', [(method + '_Probability', 100, 0, 1),
                    ('p_CMS', 100, 0, 1), ('eid', 100, 0, 1), ('chiProb', 100,
                    0, 1)])
    histMaker.param('truthVariable', 'truth')
    histMaker.param('listNames', ['e-'])
    histMaker.param('make2dHists', True)
    main.add_module(histMaker)

# ----> start processing of modules
process(main)

# ----> Print call statistics
print statistics
