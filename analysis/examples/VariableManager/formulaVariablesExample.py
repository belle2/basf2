#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# An example/test of using the formula() metavariable
# note: it's the pion momentuim + the number of tracks in the event which is total nonsense

from basf2 import process, statistics
from modularAnalysis import inputMdst, analysis_main
from stdCharged import stdPi

inputMdst(
    'default',
    '/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002166/'
    'e0000/4S/r00000/mixed/sub00/sub00/mdst_000001_prod00002166_task00000001.root'
)

# grab the standard pions list
stdPi('95eff')

nonsense_formula = 'formula(p+nTracks)'

analysis_main.add_module(
    'VariablesToNtuple',
    particleList='pi+:95eff',
    variables=['nTracks', 'p', nonsense_formula],
    fileName='formulaTest.root'
)

process(analysis_main, 10)  # process ten events
print(statistics)
