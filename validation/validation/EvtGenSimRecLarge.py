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
  <contact>Software team b2soft@mail.desy.de</contact>
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

from basf2 import (
    set_random_seed,
    create_path,
    process,
    statistics,
    register_module,
)
from simulation import add_simulation
from reconstruction import add_reconstruction
from beamparameters import add_beamparameters
from background import get_background_files
from validation import statistics_plots, event_timing_plot

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
eventinfosetter = register_module("EventInfoSetter")
# will roughly run for 10 hours
eventinfosetter.param("evtNumList", [40000])
eventinfosetter.param("runList", [1])
eventinfosetter.param("expList", [0])
main.add_module(eventinfosetter)

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters.param("smearVertex", False)

# generate BBbar events
evtgeninput = register_module("EvtGenInput")
main.add_module(evtgeninput)

# detector and L1 trigger simulation
add_simulation(main, bkgfiles=get_background_files())

# reconstruction
add_reconstruction(main)

# memory profile
main.add_module(register_module("Profile"))

# do not output to save storage space
process(main)

# Print call statistics
print(statistics)

statistics_plots(
    "EvtGenSimRecLarge_statistics.root",
    contact="Software team b2soft@mail.desy.de",
    job_desc="a standard simulation and reconstruction job with generic "
    "EvtGen events",
    prefix="EvtGenSimRecLarge",
)
event_timing_plot(
    "../EvtGenSimRecLarge.root",
    "EvtGenSimRecLarge_statistics.root",
    contact="Software team b2soft@mail.desy.de",
    job_desc="a standard simulation and reconstruction job with generic "
    "EvtGen events",
    prefix="EvtGenSimRecLarge",
)
