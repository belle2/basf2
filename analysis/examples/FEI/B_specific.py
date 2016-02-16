#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2014

from fei import *
from basf2 import *
from modularAnalysis import *

selection_path = create_path()
selection_path.add_module('RootInput')
fillParticleList('mu+', 'muid > 0.6', writeOut=True, path=selection_path)
reconstructDecay('tau+ ->  mu+', '', writeOut=True, path=selection_path)
matchMCTruth('tau+', path=selection_path)
reconstructDecay('B+:sig -> tau+', '', writeOut=True, path=selection_path)
matchMCTruth('B+:sig', path=selection_path)
buildRestOfEvent('B+:sig', path=selection_path)

# neutral B channels disabled, since we only intend to use B+/B- for our decay
particles = get_default_channels(BlevelExtraCut='nRemainingTracksInRestOfEvent == 0')
feistate = fullEventInterpretation('B+:sig', selection_path, particles)

if feistate.is_trained:
    # Add your analysis path
    open('FEI_finished_its_training', 'a').close()
    import sys
    # show constructed path
    print(feistate.path)

    process(feistate.path)
    sys.exit(1)


# show constructed path
print(feistate.path)

process(feistate.path)
B2WARNING('event() statistics:')
print(statistics)
