#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

import sys

if len(sys.argv) != 3:
    sys.exit('Must provide enough arguments: [# of events_to_generate] [output file name]')

nOfEvents = int(sys.argv[1])
outputName = sys.argv[2]

# generate events
generateY4S(nOfEvents, 'BS102-BBNeutralGen.dec')

# if simulation/reconstruction scripts are not added than one needs to load gearbox
loadGearbox()

# output file
outputMdst(outputName)

# Process the events
process(analysis_main)
