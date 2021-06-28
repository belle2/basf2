#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>EvtGenSim.root</output>
  <cacheable/>
  <contact>Software team b2soft@mail.desy.de</contact>
  <description>This steering file produces 1000 generic BBbar events with EvtGen
  and runs the detector simulation with mixed in background.</description>
</header>
"""

from basf2 import set_random_seed, create_path, process, statistics
from simulation import add_simulation
from validation import statistics_plots, event_timing_plot
from background import get_background_files

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
main.add_module("EventInfoSetter", evtNumList=[1000], runList=[1], expList=[0])

# generate BBbar events
main.add_module("EvtGenInput")

# detector and L1 trigger simulation
add_simulation(main, bkgfiles=get_background_files())

# memory profile
main.add_module("Profile")

# output
main.add_module("RootOutput", outputFileName="../EvtGenSim.root")

process(main)

# Print call statistics
print(statistics)

statistics_plots(
    "EvtGenSim_statistics.root",
    contact="Software team b2soft@mail.desy.de",
    job_desc="a standard simulation job with generic EvtGen events",
    prefix="EvtGenSim",
)
event_timing_plot(
    "../EvtGenSim.root",
    "EvtGenSim_statistics.root",
    contact="Software team b2soft@mail.desy.de",
    job_desc="a standard simulation job with generic EvtGen events",
    prefix="EvtGenSim",
)
