#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <output>Bu_D0pi_Kpi_GENSIMRECtoDST.dst.root</output>
  <contact>Luis Pesantez; pesantez@uni-bonn.de</contact>
</header>
"""

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from ROOT import Belle2

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# generate Bu->D0(K pi) pi events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
evtgeninput.param('userDECFile', Belle2.FileSystem.findFile('/generators/evtgen/decayfiles/Bu_D0pi,Kpi.dec'))

main.add_module(evtgeninput)

# detector simulation,  digitisers and clusterisers
add_simulation(main)

# reconstruction
add_reconstruction(main)

# dst output
output = register_module('RootOutput')
output.param('outputFileName', '../Bu_D0pi_Kpi_GENSIMRECtoDST.dst.root')
main.add_module(output)

# Go!
process(main)

# Print call statistics
print statistics
