#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <output>EvtGenSimRec_B2Kpi.root</output>
  <contact>staric</contact>
  <description>Generates signal MC of B0 -> K-pi+, 200 events</description>
</header>
"""

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from ROOT import Belle2

set_random_seed(123451)

main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [2])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# generate events (B0 -> K+pi- + cc, other B0 generic)
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('userDECFile', Belle2.FileSystem.findFile('top/validation/B2Kpi.dec'))
main.add_module(evtgeninput)

# detector simulation
add_simulation(main)

# reconstruction
add_reconstruction(main)

# output
output = register_module('RootOutput')
output.param('outputFileName', '../EvtGenSimRec_B2Kpi.root')
main.add_module(output)

process(main)

# Print call statistics
print statistics
