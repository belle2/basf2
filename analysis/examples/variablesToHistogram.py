#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

analysis_main.add_module('RootInput')
analysis_main.add_module('Gearbox')
fillParticleLists([('K-', ''), ('pi+', '')])
reconstructDecay('D0 -> K- pi+', '1.500 < M < 2.000')
matchMCTruth('D0')

analysis_main.add_module('VariablesToHistogram',
                         variables=['NumberOfMCParticlesInEvent(211)',
                                    'NumberOfMCParticlesInEvent(11)',
                                    'NumberOfMCParticlesInEvent(321)',
                                    'countInList(K-)',
                                    'countInList(K-, isSignal == 1)',
                                    'countInList(K-, isSignal == 0)',
                                    'countInList(pi+)',
                                    'countInList(pi+, isSignal == 1)',
                                    'countInList(pi+, isSignal == 0)',
                                    'countInList(D0)',
                                    'countInList(D0, isSignal == 1)',
                                    'countInList(D0, isSignal == 0)'])


process(analysis_main)
print statistics
