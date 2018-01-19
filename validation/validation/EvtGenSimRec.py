#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>EvtGenSimRec.root</output>
  <cacheable/>
  <contact>tkuhr</contact>
  <description>This steering file produces 1000 generic BBbar events with EvtGen,
  runs the detector simulation with mixed in background, and performs the standard reconstruction.</description>
</header>
"""

from basf2 import *
from simulation import add_simulation
from L1trigger import add_tsim
from reconstruction import add_reconstruction
from validation import statistics_plots, event_timing_plot
import validationtools

set_random_seed(12345)

# set one parallel process to excercise the basf2 parallel code
set_nprocesses(1)

main = create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter', evtNumList=[1000], runList=[1], expList=[1])

# generate BBbar events
main.add_module('EvtGenInput')

# detector simulation
bg = validationtools.get_background_files()
add_simulation(main, bkgfiles=bg)

# trigger simulation
add_tsim(main)

# reconstruction
add_reconstruction(main)

# memory profile
main.add_module('Profile')

# output
main.add_module('RootOutput', additionalBranchNames=['SpacePoints'], outputFileName='../EvtGenSimRec.root')

process(main)

# Print call statistics
print(statistics)

statistics_plots('EvtGenSimRec_statistics.root', contact='tkuhr',
                 jobDesc='a standard simulation and reconstruction job with generic EvtGen events',
                 prefix='EvtGenSimRec')
event_timing_plot('../EvtGenSimRec.root', 'EvtGenSimRec_statistics.root', contact='tkuhr',
                  jobDesc='a standard simulation and reconstruction job with generic EvtGen events',
                  prefix='EvtGenSimRec')
