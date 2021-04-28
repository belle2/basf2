#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import glob
import os

# --------------------------------------------------------------------------
# Example of using TOPTriggerDigitizer to make time stamps for trigger input
# M. Staric, October 2017
# --------------------------------------------------------------------------

# Create path
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [100]})
main.add_module(eventinfosetter)

# Gearbox
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = b2.register_module('Geometry')
main.add_module(geometry)

# generate BBbar events
evtgeninput = b2.register_module('EvtGenInput')
main.add_module(evtgeninput)

# Beam background mixer
bg = None
if 'BELLE2_BACKGROUND_MIXING_DIR' in os.environ:
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_MIXING_DIR'] + '/*.root')
if bg is not None:
    bkgmixer = b2.register_module('BeamBkgMixer')
    bkgmixer.param('backgroundFiles', bg)
    main.add_module(bkgmixer)
    b2.B2RESULT('Simulaton w/ beam background, samples taken from folder ' +
                os.environ['BELLE2_BACKGROUND_MIXING_DIR'])
else:
    b2.B2RESULT('Simulaton w/o beam background')

# Simulation
simulation = b2.register_module('FullSim')
main.add_module(simulation)

# TOP digitization
main.add_module('TOPDigitizer', allChannels=True, readoutWindows=12, offsetWindows=4)

# TOP trigger digitization (time stamps)
trigdigi = b2.register_module('TOPTriggerDigitizer')
trigdigi.param('threshold', 28)  # 3 sigma of electronic noise
main.add_module(trigdigi)

# Output
output = b2.register_module('RootOutput')
output.param('branchNames', ['TOPDigits', 'TOPTriggerDigits', 'TOPTriggerMCInfo'])
main.add_module(output)

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
