#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>CosmicsSimNoBkg.root</output>
  <contact>tkuhr</contact>
  <cacheable/>
  <description>This steering file produces 10000 cosmic ray events without background.</description>
</header>
"""

from basf2 import *
from simulation import add_simulation
from beamparameters import add_beamparameters
import glob

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters.param("smearVertex", False)

# generate BBbar events
comsics = register_module('Cosmics')
main.add_module(comsics)

# detector simulation
add_simulation(main)

# memory profile
main.add_module(register_module('Profile'))

# output
output = register_module('RootOutput')
output.param('outputFileName', '../CosmicsSimNoBkg.root')
main.add_module(output)

process(main)

# Print call statistics
print(statistics)

from validation import *
statistics_plots('CosmicsSimNoBkg_statistics.root', contact='tkuhr',
                 jobDesc='a standard simulation job with Cosmics events', prefix='CosmicsSimNoBkg')
event_timing_plot('../CosmicsSimNoBkg.root', 'CosmicsSimNoBkg_statistics.root',
                  contact='tkuhr',
                  jobDesc='a standard simulation job with Cosmics events', prefix='CosmicsSimNoBkg')
