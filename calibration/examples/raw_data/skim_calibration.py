# Add the cut module to the pat#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# An example of how to skim off events that pass a set of trigger lines (using a custom module for now)
# and how to also convert SROOT->ROOT at the same time. This is roughly the planned data flow after the DAQ
# finishes with the data and outputs SROOT files. We want to copy/skim off calibration events (muon trigger here
# is the stand-in since the calib triggers aren't ready) into their own ROOT files while allowing ALL events
# to be output to RAW ROOT files.

# The TriggerSkim module allows us to query SoftwareTriggerResult objects in the RAW output without having to
# re-do the reconstruction/calculation again i.e. using the Trigger decisions already done by the HLT.
# Notice that we NEVER UNPACK the RAW data during this step, making this very fast.

from basf2 import *
from ROOT import Belle2

# Create path
main = create_path()

# Root input
roinput = register_module('SeqRootInput')
# roinput = register_module('RootInput')
main.add_module(roinput)

# Output only events that pass
output_calib_path = create_path()
output = register_module('RootOutput')
output.param('outputFileName', "calib_mumu.root")
output_calib_path.add_module(output)

cut_decision = register_module("TriggerSkim")
cut_decision.param('triggerLines', ['software_trigger_cut&hlt&accept_2_tracks',
                                    'software_trigger_cut&hlt&accept_mu_mu'])
cut_decision.param('expectedResult', 1)
# cut_decision.param('logicMode', 'and')  # Default == 'or'
cut_decision.if_value("==1", output_calib_path, AfterConditionPath.CONTINUE)
main.add_module(cut_decision)

# Output all events but in ROOT (not SeqROOT) format
output_path = create_path()
output = register_module('RootOutput')
output.param('outputFileName', "raw.root")
main.add_module(output)

# Process events
process(main)
print(statistics)
