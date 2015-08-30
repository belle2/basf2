#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck 2015

from fei import *
from basf2 import *
from modularAnalysis import *

# One should do a skim for events with <= 12 tracks before doing the training to speedup the training a lot!

# Remove all candidates for training with remaining tracks in event
particles = get_default_channnels(BlevelExtraCut='nRemainingTracksInEvent == 0', neutralB=True)
feistate = fullEventInterpretation(None, None, particles)

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
