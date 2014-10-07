#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <output>EvtGenSimRec.root</output>
  <contact>tkuhr</contact>
  <description>This steering file produces 100 generic BBbar events with EvtGen, runs the detector simulation with mixed in background, and performs the standard reconstruction.</description>
</header>
"""

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
import glob

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [100])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
main.add_module(evtgeninput)

# detector simulation
bg = None
if os.environ.has_key('BELLE2_BACKGROUND_DIR'):
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
add_simulation(main, bkgfiles=bg)

# reconstruction
add_reconstruction(main)

# output
output = register_module('RootOutput')
output.param('outputFileName', '../EvtGenSimRec.root')
main.add_module(output)

process(main)

# Print call statistics
print statistics
