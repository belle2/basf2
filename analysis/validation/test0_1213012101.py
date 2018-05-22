#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: [B+ -> [anti-D0 -> K_S0 pi0] K+]cc

"""
<header>
  <output>../1213012101.dst.root</output>
  <contact>karim.trabelsi@kek.jp</contact>
</header>
"""

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from ROOT import Belle2

set_random_seed(12345)


# create path
main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('userDECFile', Belle2.FileSystem.findFile('/decfiles/dec/1213012101.dec'))
main.add_module(evtgeninput)

# detector simulation
add_simulation(main)

# reconstruction
add_reconstruction(main)


# Finally add mdst output
output_filename = "../1213012101.dst.root"
add_mdst_output(main, filename=output_filename)

# process events and print call statistics
process(main)
print(statistics)
