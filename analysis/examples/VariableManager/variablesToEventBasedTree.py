#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# The VariablesToEventBasedTree module saves variables from the VariableManager to an event-based TTree

from basf2 import *
from modularAnalysis import *

import os
if os.path.isfile('mdst.root'):
    filename = 'mdst.root'
elif os.path.isfile('/storage/jbod/tkeck/MC7/evtgen-charged/sub00/mdst_000240_prod00000788_task00000685.root'):
    filename = '/storage/jbod/tkeck/MC7/evtgen-charged/sub00/mdst_000240_prod00000788_task00000685.root'
elif os.path.isfile('/ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/MC7/prod00000788/s00/e0000/4S/r00000/'
                    'charged/sub00/mdst_000240_prod00000788_task00000685.root'):
    filename = '/ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/MC7/prod00000788/s00/e0000/4S/r00000/'\
               'charged/sub00/mdst_000240_prod00000788_task00000685.root'
else:
    raise RuntimeError("Please copy an mdst file from KEKCC into this directory named mdst.root")

inputMdstList('MC7', [filename])

fillParticleLists([('K-', 'kaonID > 0.2'), ('pi+', 'pionID > 0.2')])
reconstructDecay('D0 -> K- pi+', '1.750 < M < 1.95')
matchMCTruth('D0')

# This will write out one row per event in the TTree
# The branches of the TTree are arrays containing the variables for each D0 candidate in the event
# The event_variables are handled differently, the branches of these event_variables are just floats,
# you can use this to write out candidate independent information (aka event-based variables)
analysis_main.add_module('VariablesToEventBasedTree',
                         particleList='D0',
                         variables=['dM', 'isSignal', 'mcErrors', 'p', 'E',
                                    'daughter(0, kaonID)', 'daughter(1, pionID)'],
                         event_variables=['nTracks', 'expNum', 'runNum', 'evtNum'])

process(analysis_main)
print(statistics)
