#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>tkuhr</contact>
  <output>EvtGenSimRecLarge.root</output>
  <description>This steering file produces 10000 generic BBbar events with EvtGen,
  runs the detector simulation with mixed in background, and performs the standard reconstruction.
  It will only be run for release validation in order to test the software on a larger set of events to
  be more closer to the typical runtime on a grid site.</description>
  <interval>release</interval>
</header>
"""

from basf2 import *
from simulation import add_simulation
from L1trigger import add_tsim
from reconstruction import add_reconstruction
from beamparameters import add_beamparameters
from background import get_background_files
import validationtools

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
# will roughly run for 10 hours
eventinfosetter.param('evtNumList', [40000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters.param("smearVertex", False)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
main.add_module(evtgeninput)

# detector simulation
add_simulation(main, bkgfiles=get_background_files())

# trigger simulation
add_tsim(main)

# reconstruction
add_reconstruction(main)

# memory profile
main.add_module(register_module('Profile'))

# do not output to save storage space
process(main)

# Print call statistics
print(statistics)

from validation import *
statistics_plots('EvtGenSimRecLarge_statistics.root', contact='tkuhr',
                 jobDesc='a standard simulation and reconstruction job with generic EvtGen events', prefix='EvtGenSimRecLarge')
event_timing_plot('../EvtGenSimRecLarge.root', 'EvtGenSimRecLarge_statistics.root',
                  contact='tkuhr',
                  jobDesc='a standard simulation and reconstruction job with generic EvtGen events', prefix='EvtGenSimRecLarge')
