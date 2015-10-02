#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

analysis_main.add_module('RootInput')
analysis_main.add_module('Gearbox')
fillParticleLists([('K-', ''), ('pi+', '')])
reconstructDecay('D0 -> K- pi+', '1.500 < M < 2.000')
matchMCTruth('D0')

analysis_main.add_module('VariablesToHistogram',
                         variables=[('NumberOfMCParticlesInEvent(211)', 100, -0.5, 99.5),
                                    ('NumberOfMCParticlesInEvent(11)', 100, -0.5, 99.5),
                                    ('NumberOfMCParticlesInEvent(321)', 100, -0.5, 99.5),
                                    ('countInList(K-)', 100, -0.5, 99.5),
                                    ('countInList(K-, isSignal == 1)', 100, -0.5, 99.5),
                                    ('countInList(K-, isSignal == 0)', 100, -0.5, 99.5),
                                    ('countInList(pi+)', 100, -0.5, 99.5),
                                    ('countInList(pi+, isSignal == 1)', 100, -0.5, 99.5),
                                    ('countInList(pi+, isSignal == 0)', 100, -0.5, 99.5),
                                    ('countInList(D0)', 100, -0.5, 99.5),
                                    ('countInList(D0, isSignal == 1)', 100, -0.5, 99.5),
                                    ('countInList(D0, isSignal == 0)', 100, -0.5, 99.5)])


process(analysis_main)
print(statistics)
