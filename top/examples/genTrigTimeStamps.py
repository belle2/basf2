#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import glob

# --------------------------------------------------------------------------
# Example of using TOPTriggerDigitizer to make time stamps for trigger input
# M. Staric, October 2017
# --------------------------------------------------------------------------

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [100]})
main.add_module(eventinfosetter)

# Gearbox (set larger time window and offset for TOP)
gearbox = register_module('Gearbox')
gearbox.param('override', [("/DetectorComponent[@name='TOP']//numWindows", '12', ''),
                           ("/DetectorComponent[@name='TOP']//offset", '150', 'ns')])
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
main.add_module(geometry)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
main.add_module(evtgeninput)

# Beam background mixer
bg = None
if 'BELLE2_BACKGROUND_MIXING_DIR' in os.environ:
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_MIXING_DIR'] + '/*.root')
if bg is not None:
    bkgmixer = register_module('BeamBkgMixer')
    bkgmixer.param('backgroundFiles', bg)
    main.add_module(bkgmixer)
    B2RESULT('Simulaton w/ beam background, samples taken from folder ' +
             os.environ['BELLE2_BACKGROUND_MIXING_DIR'])
else:
    B2RESULT('Simulaton w/o beam background')

# Simulation
simulation = register_module('FullSim')
main.add_module(simulation)

# TOP digitization
topdigi = register_module('TOPDigitizer')
topdigi.param('useWaveforms', True)
topdigi.param('allChannels', True)
main.add_module(topdigi)

# TOP trigger digitization (time stamps)
trigdigi = register_module('TOPTriggerDigitizer')
trigdigi.param('threshold', 28)  # 3 sigma of electronic noise
main.add_module(trigdigi)

# Output
output = register_module('RootOutput')
output.param('branchNames', ['TOPDigits', 'TOPTriggerDigits', 'TOPTriggerMCInfo'])
main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
