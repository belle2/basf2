#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# The VariablesToNtuple module saves variables from the VariableManager to a candidate-based TNtuple

from basf2 import *
from modularAnalysis import *

inputMdstList('default', ['/local/scratch/MC/MC5/Charged00/mdst_000001_prod00000002_task02000001.root'])

fillParticleLists([('K-', 'Kid > 0.2'), ('pi+', 'piid > 0.2')])
reconstructDecay('D0 -> K- pi+', '1.750 < M < 1.95')
matchMCTruth('D0')

# This will write out one row per candidate in the D0 list
analysis_main.add_module('VariablesToExtraInfo',
                         particleList='D0',
                         variables={'M': 'M_before_vertex_fit'})

fitVertex('D0')

analysis_main.add_module('VariablesToNtuple',
                         particleList='D0',
                         variables=['M', 'exraInfo(M_before_vertex_fit)'])

process(analysis_main)
print(statistics)
