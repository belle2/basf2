#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

import sys

if len(sys.argv) != 2:
    sys.exit('Must provide [#events_to_generate]')

# generate events
nOfEvents = int(sys.argv[1])
generateY4S(nOfEvents, 'BS101-BBChargedGen.dec')

# if simulation/reconstruction scripts are not added than one needs to load gearbox
loadGearbox()

# output file
outputMdst('BS101-BBChargedGen.root')

# process the events
process(analysis_main)
