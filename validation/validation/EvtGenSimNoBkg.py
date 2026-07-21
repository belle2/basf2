#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <output>EvtGenSimNoBkg.root</output>
  <cacheable/>
  <contact>giacomo.pietro@kit.edu</contact>
  <description>This steering file produces 1000 generic BBbar events with EvtGen
  and runs the detector simulation without mixed in background.</description>
</header>
"""

from basf2 import set_random_seed, create_path, process
from simulation import add_simulation
from validation import statistics_plots, event_timing_plot
from validationgenerators import add_evtgen_for_validation

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
main.add_module("EventInfoSetter", evtNumList=[1000], runList=[1], expList=[0])

# generate BBbar events
add_evtgen_for_validation(main)

# detector and L1 trigger simulation WITHOUT beam background
add_simulation(main)

# memory profile
main.add_module("Profile")

# output
main.add_module("RootOutput", outputFileName="../EvtGenSimNoBkg.root")

main.add_module('Progress')
process(main, calculateStatistics=True)

statistics_plots(
    "EvtGenSimNoBkg_statistics.root",
    contact="giacomo.pietro@kit.edu",
    job_desc="a standard simulation job with generic EvtGen events and no beam background",
    prefix="EvtGenSimNoBkg",
)
event_timing_plot(
    "../EvtGenSimNoBkg.root",
    "EvtGenSimNoBkg_statistics.root",
    contact="giacomo.pietro@kit.edu",
    job_desc="a standard simulation job with generic EvtGen events and no beam background",
    prefix="EvtGenSimNoBkg",
)
