#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
import datetime
from tracking import add_cdc_cr_track_finding
from simulation import *
import os.path
import sys
from cdc.cr import *

import argparse
parser = argparse.ArgumentParser()
parser.add_argument('--period', dest='period', default='normal', help='Data period')
args = parser.parse_args()

main_path = create_path()
empty_path = create_path()

main_path.add_module('EventInfoSetter')
main_path.add_module('Progress')

# Propagation velocity of the light in the scinti.
lightPropSpeed = 12.9925
# Run range.
run_range = {'normal': [-1, -1]}
# Size of trigger counter.
triggerSize = {'normal': [100.0, 8.0, 10.0]}
# Center position of trigger counter.
triggerPosition = {'normal': [0.0, 0.0, 0.0]}
# Normal direction of the trigger plane.
triggerPlaneDirection = {'normal': [0, 1, 0]}
# PMT position.
pmtPosition = {'normal': [0, 0, -50.0]}
lengthOfCounter = 100.0
widthOfCounter = 8.0
triggerPos = []
normTriggerPlanDirection = []
readOutPos = []

lengthOfCounter = triggerSize[args.period][0]
widthOfCounter = triggerSize[args.period][1]
triggerPos = triggerPosition[args.period]
normTriggerPlanDirection = triggerPlaneDirection[args.period]
readOutPos = pmtPosition[args.period]

main_path.add_module('Gearbox',
                     fileName="/geometry/GCR_Summer2017.xml",
                     override=[
                              ("/Global/length", "8.", "m"),
                              ("/Global/width", "8.", "m"),
                              ("/Global/height", "8.", "m"),
                     ])
main_path.add_module('Geometry')

# Register the CRY module
cry = register_module('CRYInput')
cry.param('CosmicDataDir', Belle2.FileSystem.findFile('data/generators/modules/cryinput/'))
cry.param('SetupFile', 'cry.setup')

# cry.param('acceptLength', 8.0)
# cry.param('acceptWidth', 8.0)
# cry.param('acceptHeight', 8.0)
# cry.param('keepLength', 8.0)
# cry.param('keepWidth', 8.0)
# cry.param('keepHeight', 8.0)

cry.param('acceptLength', 0.6)
cry.param('acceptWidth', 0.2)
cry.param('acceptHeight', 0.2)
cry.param('keepLength', 0.6)
cry.param('keepWidth', 0.2)
cry.param('keepHeight', 0.2)

cry.param('maxTrials', 10000)
cry.param('kineticEnergyThreshold', 1.)
main_path.add_module(cry)

# Selector module.
#    sel = register_module('CDCCosmicSelector',
#                          lOfCounter=lengthOfCounter,
#                          wOfCounter=widthOfCounter,
#                          xOfCounter=triggerPos[0],
#                          yOfCounter=triggerPos[1],
#                          zOfCounter=triggerPos[2],
#                          phiOfCounter=0.,
#                          TOP=True,
#                          propSpeed=lightPropSpeed,
#                          TOF=1,
#                          cryGenerator=True
#                          )

#    main_path.add_module(sel)
#    sel.if_false(empty_path)

# Full simulation
add_simulation(main_path)

# Overwrite default module configurations
modules = main_path.modules()
for m in modules:
    if "ECLDigitizer" in m.name():
        m.param('DiodeDeposition', True)

# Output
main_path.add_module('RootOutput')

print_path(main_path)
process(main_path)
print(statistics)
