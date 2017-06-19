#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# The VariablesToNtuple module saves variables from the VariableManager to a candidate-based TNtuple

from basf2 import *
from modularAnalysis import *

inputMdstList('MC7', ['/storage/jbod/tkeck/MC7/evtgen-charged/sub00/mdst_000240_prod00000788_task00000685.root'])

fillParticleLists([('K-', 'Kid > 0.2'), ('pi+', 'piid > 0.2')])
reconstructDecay('D0 -> K- pi+', '1.750 < M < 1.95')
matchMCTruth('D0')

# This will write out one row per candidate in the D0 list
analysis_main.add_module('VariablesToNtuple',
                         particleList='D0',
                         variables=['dM', 'isSignal', 'mcErrors', 'p', 'E',
                                    'daughter(0, Kid)', 'daughter(1, piid)'],
                         fileName='CandidateVariables.root')

# If the particle list is empty one row per event is written to the Ntuple,
# but all the variables to specify have to be event-based
analysis_main.add_module('VariablesToNtuple',
                         particleList='',
                         variables=['nTracks', 'evtNum', 'runNum', 'expNum'],
                         fileName='EventVariables.root')

process(analysis_main)
print(statistics)
