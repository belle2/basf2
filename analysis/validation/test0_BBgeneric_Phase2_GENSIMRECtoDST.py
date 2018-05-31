#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>../BBgeneric_Phase2.dst.root</output>
  <contact>Cate MacQueen, cmq.centaurus@gmail.com, Bryan Fulsom, bryan.fulsom@pnnl.gov</contact>
</header>
"""

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output
from beamparameters import add_beamparameters
from ROOT import Belle2

set_random_seed(10000)

main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [100])
eventinfosetter.param('runList', [0])
eventinfosetter.param('expList', [1002])
main.add_module(eventinfosetter)

evtgen = register_module('EvtGenInput')
evtgen.param('ParentParticle', "Upsilon(4S)")
main.add_module(evtgen)
print_params(evtgen)

# simulation
add_simulation(main)

# reconstruction
add_reconstruction(main)

# dst and mdst output
output = register_module('RootOutput')
output.param('outputFileName', '../BBgeneric_Phase2.dst.root')
main.add_module(output)

# Go!
process(main)

# Print call statistics
print(statistics)
