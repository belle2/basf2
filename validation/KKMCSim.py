#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <output>KKMCSim.root</output>
  <contact>tkuhr</contact>
  <description>This steering file produces 10000 tau pair events with KKMC and runs the detector simulation with mixed in background.</description>
</header>
"""

from basf2 import *
from simulation import add_simulation
import glob

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# generate tau pair events
kkmc = register_module('KKGenInput')
main.add_module(kkmc)

# detector simulation
bg = None
if os.environ.has_key('BELLE2_BACKGROUND_DIR'):
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
add_simulation(main, bkgfiles=bg)

# memory profile
main.add_module(register_module('Profile'))

# output
output = register_module('RootOutput')
output.param('outputFileName', '../KKMCSim.root')
main.add_module(output)

process(main)

# Print call statistics
print statistics

from validation import *
statistics_plots('KKMCSim_statistics.root', contact='tkuhr',
                 jobDesc='a standard simulation job with KKMC tau pair events'
                 , prefix='KKMCSim')
event_timing_plot('../KKMCSim.root', 'KKMCSim_statistics.root', contact='tkuhr'
                  ,
                  jobDesc='a standard simulation job with KKMC tau pair events'
                  , prefix='KKMCSim')
