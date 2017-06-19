#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# The VariablesToTree module saves variables from the VariableManager to an event-based TTree

from basf2 import *
from modularAnalysis import *

inputMdstList('MC7', ['/storage/jbod/tkeck/MC7/evtgen-charged/sub00/mdst_000240_prod00000788_task00000685.root'])

fillParticleLists([('K-', 'Kid > 0.2'), ('pi+', 'piid > 0.2')])
reconstructDecay('D0 -> K- pi+', '1.750 < M < 1.95')
matchMCTruth('D0')

# This will write out one row per event in the TTree
# The branches of the TTree are arrays containing the variables for each D0 candidate in the event
# The event_variables are handled differently, the branches of these event_variables are just floats,
# you can use this to write out candidate independent information (aka event-based variables)
analysis_main.add_module('VariablesToTree',
                         particleList='D0',
                         variables=['dM', 'isSignal', 'mcErrors', 'p', 'E',
                                    'daughter(0, Kid)', 'daughter(1, piid)'],
                         event_variables=['nTracks', 'expNum', 'runNum', 'evtNum'])

process(analysis_main)
print(statistics)
