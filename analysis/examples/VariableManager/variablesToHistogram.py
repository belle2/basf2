#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# The VariablesToHistogram module saves variables from the VariableManager to TH1F and TH2F

from basf2 import *
from modularAnalysis import *

inputMdstList('default', ['/local/scratch/MC/MC5/Charged00/mdst_000001_prod00000002_task02000001.root'])

fillParticleLists([('K-', 'Kid > 0.2'), ('pi+', 'piid > 0.2')])
reconstructDecay('D0 -> K- pi+', '1.750 < M < 1.95')
matchMCTruth('D0')

analysis_main.add_module('VariablesToHistogram',
                         particleList='D0',
                         variables=[('dM', 100, -2.0, 2.0),
                                    ('isSignal', 2, -0.5, 1.5),
                                    ('mcErrors', 1025, -0.5, 1024.5),
                                    ('p', 1000, 0.0, 5.0),
                                    ('E', 1000, 0.0, 5.0),
                                    ('daughter(0, Kid)', 100, 0.0, 1.0),
                                    ('daughter(1, piid)', 100, 0.0, 1.0)],
                         variables_2d=[('dM', 100, -2.0, 2.0, 'isSignal', 2, -0.5, 1.5),
                                       ('p', 100, -2.0, 2.0, 'isSignal', 2, -0.5, 1.5),
                                       ('dM', 100, -2.0, 2.0, 'mcErrors', 1025, -0.5, 1024.5)])


process(analysis_main)
print(statistics)
