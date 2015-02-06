#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck 2014

from FEI import *
from basf2 import *
from modularAnalysis import *


selection_path = create_path()
selection_path.add_module(register_module('RootInput'))
fillParticleList('mu+', 'muid > 0.6 and nTracks <= 12', writeOut=True, path=selection_path)
reconstructDecay('tau+ ->  mu+', '', writeOut=True, path=selection_path)
matchMCTruth('tau+', path=selection_path)
reconstructDecay('B+:sig -> tau+', '', writeOut=True, path=selection_path)
matchMCTruth('B+:sig', path=selection_path)
buildRestOfEvent('B+:sig', path=selection_path)

analysis_path = create_path()
analysis_path.add_module(register_module('RootOutput'))

particles = get_default_channnels(BlevelExtraCut='nRemainingTracksInRestOfEvent == 0')
main = fullEventInterpretation(selection_path, analysis_path, particles)

# show constructed path
print main

process(main)
B2WARNING('event() statistics:')
print statistics
B2WARNING('endRun() statistics:')
print statistics(statistics.END_RUN)
