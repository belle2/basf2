#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# The VariablesToHistogram module saves variables from the VariableManager to TH1F and TH2F

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

analysis_main.add_module('VariablesToHistogram',
                         particleList='D0',
                         variables=[('dM', 100, -2.0, 2.0),
                                    ('isSignal', 2, -0.5, 1.5),
                                    ('mcErrors', 1025, -0.5, 1024.5),
                                    ('p', 1000, 0.0, 5.0),
                                    ('E', 1000, 0.0, 5.0),
                                    ('daughter(0, kaonID)', 100, 0.0, 1.0),
                                    ('daughter(1, pionID)', 100, 0.0, 1.0)],
                         variables_2d=[('dM', 100, -2.0, 2.0, 'isSignal', 2, -0.5, 1.5),
                                       ('p', 100, -2.0, 2.0, 'isSignal', 2, -0.5, 1.5),
                                       ('dM', 100, -2.0, 2.0, 'mcErrors', 1025, -0.5, 1024.5)])


process(analysis_main)
print(statistics)
