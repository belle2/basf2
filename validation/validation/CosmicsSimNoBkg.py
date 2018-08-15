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
from validation import statistics_plots, event_timing_plot

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter', evtNumList=[10000], runList=[1], expList=[0])

# generate BBbar events
main.add_module('Cosmics')

# detector simulation
add_simulation(main)

# memory profile
main.add_module('Profile')

# output
main.add_module('RootOutput', outputFileName='../CosmicsSimNoBkg.root')

process(main)

# Print call statistics
print(statistics)

statistics_plots('CosmicsSimNoBkg_statistics.root', contact='tkuhr',
                 jobDesc='a standard simulation job with Cosmics events',
                 prefix='CosmicsSimNoBkg')
event_timing_plot('../CosmicsSimNoBkg.root', 'CosmicsSimNoBkg_statistics.root', contact='tkuhr',
                  jobDesc='a standard simulation job with Cosmics events',
                  prefix='CosmicsSimNoBkg')
