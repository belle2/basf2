#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck 2015

from fei import *
from basf2 import *
from modularAnalysis import *

selection_path = create_path()
selection_path.add_module('RootInput')
fillParticleList('gamma', 'goodGamma > 0.5 and E > 0.5 and nTracks <= 12', writeOut=True, path=selection_path)
reconstructDecay('B0:sig -> gamma', '', writeOut=True, path=selection_path)
matchMCTruth('B0:sig', path=selection_path)

particles = get_default_channels(BlevelExtraCut='nRemainingTracksInRestOfEvent == 0', neutralB=True, chargedB=False)
feistate = fullEventInterpretation('B0:sig', selection_path, particles)

if feistate.is_trained:
    # Add your analysis path
    open('FEI_finished_its_training', 'a').close()
    import sys
    # show constructed path
    print feistate.path

    process(feistate.path)
    sys.exit(1)


# show constructed path
print feistate.path

process(feistate.path)
B2WARNING('event() statistics:')
print statistics
