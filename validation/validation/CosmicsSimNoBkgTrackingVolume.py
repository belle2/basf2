#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <output>CosmicsSimNoBkgTrackingVolume.root</output>
  <contact>Software team b2soft@mail.desy.de</contact>
  <cacheable/>
  <description>
    This steering file produces 10000 cosmic ray events without background, for the early_phase3 geometry.
    The particles are created on a cylinder of radius 125cm (right outside the tracking volume).
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

# Generate cosmic events. Note: with default settings the cosmics are
# generated on a cylinder of 125cm (closely outside the tracking valume)!
main.add_module("Cosmics")

# detector simulation
add_simulation(main)

# memory profile
main.add_module("Profile")

# output
main.add_module(
    "RootOutput", outputFileName="../CosmicsSimNoBkgTrackingVolume.root"
)

process(main)

# Print call statistics
print(statistics)

statistics_plots(
    "CosmicsSimNoBkgTrackingVolume_statistics.root",
    contact="Software team b2soft@mail.desy.de",
    job_desc="a standard simulation job with Cosmics events",
    prefix="CosmicsSimNoBkgTrackingVolume",
)
event_timing_plot(
    "../CosmicsSimNoBkgTrackingVolume.root",
    "CosmicsSimNoBkgTrackingVolume_statistics.root",
    contact="Software team b2soft@mail.desy.de",
    job_desc="a standard simulation job with Cosmics events",
    prefix="CosmicsSimNoBkgTrackingVolume",
)
