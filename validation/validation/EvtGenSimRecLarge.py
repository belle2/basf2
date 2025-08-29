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
  <contact>arul.prakash@physik.uni-muenchen.de</contact>
  <output>EvtGenSimRecLarge.root</output>
  <description>
    This steering file produces 10000 generic BBbar events with
    EvtGen, runs the detector simulation with mixed in background, and performs
    the standard reconstruction. It will only be run for release validation in
    order to test the software on a larger set of events to be more closer to
    the typical runtime on a grid site.
  </description>
  <interval>release</interval>
</header>
"""

from basf2 import set_random_seed, create_path, process
from simulation import add_simulation
from reconstruction import add_reconstruction
from beamparameters import add_beamparameters
from validation import statistics_plots, event_timing_plot
from background import get_background_files

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
main.add_module("EventInfoSetter", evtNumList=[10000], runList=[1], expList=[0])

# beam parameters
add_beamparameters(main, "Y4S")

# generate BBbar events
main.add_module("EvtGenInput")

# detector and L1 trigger simulation
add_simulation(main, bkgfiles=get_background_files())

# reconstruction
add_reconstruction(main)

main.add_module('Progress')
# memory profile
main.add_module("Profile")

# output
main.add_module("RootOutput",
                outputFileName="../EvtGenSimRecLarge.root",
                branchNames=["ProfileInfo"])

process(main, calculateStatistics=True)

statistics_plots(
    "EvtGenSimRecLarge_statistics.root",
    contact="arul.prakash@physik.uni-muenchen.de",
    job_desc="a standard simulation and reconstruction job with generic "
    "EvtGen events",
    prefix="EvtGenSimRecLarge",
)
event_timing_plot(
    "../EvtGenSimRecLarge.root",
    "EvtGenSimRecLarge_statistics.root",
    contact="arul.prakash@physik.uni-muenchen.de",
    job_desc="a standard simulation and reconstruction job with generic "
    "EvtGen events",
    prefix="EvtGenSimRecLarge",
)
