#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>CosmicsSimNoBkg.root</output>
  <contact>Software team b2soft@mail.desy.de</contact>
  <cacheable/>
  <description>
    This steering file produces 10000 cosmic ray events without background, for the early_phase3 geometry.
  </description>
</header>
"""

from basf2 import create_path, statistics, set_random_seed, process
from simulation import add_simulation
from validation import statistics_plots, event_timing_plot

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
main.add_module(
    "EventInfoSetter", evtNumList=[10000], runList=[1], expList=[1003]
)

# Generate cosmic events.
# Set initial radius to 350 cm to include KLM (excluded by default).
main.add_module("Cosmics", cylindricalR=350)

# detector simulation
add_simulation(main)

# memory profile
main.add_module("Profile")

# output
main.add_module("RootOutput", outputFileName="../CosmicsSimNoBkg.root")

process(main)

# Print call statistics
print(statistics)

statistics_plots(
    "CosmicsSimNoBkg_statistics.root",
    contact="Software team b2soft@mail.desy.de",
    job_desc="a standard simulation job with Cosmics events",
    prefix="CosmicsSimNoBkg",
)
event_timing_plot(
    "../CosmicsSimNoBkg.root",
    "CosmicsSimNoBkg_statistics.root",
    contact="Software team b2soft@mail.desy.de",
    job_desc="a standard simulation job with Cosmics events",
    prefix="CosmicsSimNoBkg",
)
