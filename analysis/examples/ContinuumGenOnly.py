#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

# suppress messages and warnings during processing:
set_log_level(LogLevel.INFO)
# set_random_seed(75628607)

# generate 10000 events according to the provided DECAY file
generateEvents(10000, os.environ['BELLE2_LOCAL_DIR']
               + '/generators/evtgen/decayfiles/ccbar+Dst.dec')

# if simulation/reconstruction scripts are not added than one needs to load gerbox
loadGearbox()

outputMdst('evtgen-Dstar.root')

# Process the events
process(main)
