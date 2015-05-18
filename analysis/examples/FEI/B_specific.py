#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck 2014

from fei import *
from basf2 import *
from modularAnalysis import *

selection_path = create_path()
selection_path.add_module('RootInput')
fillParticleList('mu+', 'muid > 0.6 and nTracks <= 12', writeOut=True, path=selection_path)
reconstructDecay('tau+ ->  mu+', '', writeOut=True, path=selection_path)
matchMCTruth('tau+', path=selection_path)
reconstructDecay('B+:sig -> tau+', '', writeOut=True, path=selection_path)
matchMCTruth('B+:sig', path=selection_path)
buildRestOfEvent('B+:sig', path=selection_path)

# neutral B channels disabled, since we only intend to use B+/B- for our decay
particles = get_default_channnels(BlevelExtraCut='nRemainingTracksInRestOfEvent == 0', neutralB=False)
feistate = fullEventInterpretation(selection_path, particles)

if feistate.is_trained:
    # Add your analysis path
    feistate.path.add_module('RootOutput', outputFileName='analysisPathDone.root', ignoreCommandLineOverride=True)

# show constructed path
print feistate.path

process(feistate.path)
B2WARNING('event() statistics:')
print statistics
