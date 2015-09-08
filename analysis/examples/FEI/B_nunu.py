#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck 2015

from fei import *
from basf2 import *
from modularAnalysis import *

# Use selection path to throw away events with more than 12 tracks,
# because the FEI cannot reconstruct channels with more tracks
selection_path = create_path()
trash_path = create_path()
skim = register_module('VariableToReturnValue')
skim.param("variable", 'nTracks')
skim.if_value('>12', trash_path, AfterConditionPath.END)
selection_path.add_module(skim)

# Remove all candidates for training with remaining tracks in event
particles = get_default_channels(BlevelExtraCut='nRemainingTracksInEvent == 0', neutralB=True, chargedB=False)
feistate = fullEventInterpretation(None, selection_path, particles)

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
