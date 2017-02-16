#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>EvtGenSim.root</output>
  <cacheable/>
  <contact>tkuhr</contact>
  <description>This steering file produces 10000 generic BBbar events with EvtGen
  and runs the detector simulation with mixed in background.</description>
</header>
"""

from basf2 import *
from simulation import add_simulation
from beamparameters import add_beamparameters
import validationtools

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters.param("smearVertex", False)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
main.add_module(evtgeninput)

bg = validationtools.get_background_files()
add_simulation(main, bkgfiles=bg)

# memory profile
main.add_module(register_module('Profile'))

# output
output = register_module('RootOutput')
output.param('outputFileName', '../EvtGenSim.root')
main.add_module(output)

process(main)

# Print call statistics
print(statistics)

from validation import *
statistics_plots('EvtGenSim_statistics.root', contact='tkuhr',
                 jobDesc='a standard simulation job with generic EvtGen events', prefix='EvtGenSim')
event_timing_plot('../EvtGenSim.root', 'EvtGenSim_statistics.root',
                  contact='tkuhr',
                  jobDesc='a standard simulation job with generic EvtGen events', prefix='EvtGenSim')
