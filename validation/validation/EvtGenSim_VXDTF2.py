#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>EvtGenSim.root</output>
  <cacheable/>
  <contact>tkuhr</contact>
  <description>This steering file produces 1000 generic BBbar events with EvtGen
  and runs the detector simulation with mixed in background.
  This file uses the new VXDTF2 (instead of VXDTF1) for data reduction in the simulation. As
  soon as VXDTF2 becomes the default, this file can be removed.
  </description>
</header>
"""

from basf2 import *
from simulation import add_simulation
from L1trigger import add_tsim
from validation import statistics_plots, event_timing_plot
import validationtools

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter', evtNumList=[1000], runList=[1], expList=[1])

# generate BBbar events
main.add_module('EvtGenInput')

# detector simulation
bg = validationtools.get_background_files()
add_simulation(main, bkgfiles=bg, use_vxdtf2=True)

# trigger simulation
add_tsim(main)

# memory profile
main.add_module('Profile')

# output
main.add_module('RootOutput', outputFileName='../EvtGenSim_VXDTF2.root')

process(main)

# Print call statistics
print(statistics)

statistics_plots('EvtGenSim_statistics_VXDTF2.root', contact='tkuhr',
                 jobDesc='a standard simulation job with generic EvtGen events',
                 prefix='EvtGenSim')
event_timing_plot('../EvtGenSim_VXDTF2.root', 'EvtGenSim_statistics_VXDTF2.root', contact='tkuhr',
                  jobDesc='a standard simulation job with generic EvtGen events',
                  prefix='EvtGenSim')
