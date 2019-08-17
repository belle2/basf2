#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: [D0 -> K- pi+ ]cc

"""
<header>
  <output>DstToD0Pi_D0ToHpJm.dst.root</output>
  <contact>gonggd@mail.ustc.edu.cn</contact>
</header>
"""

from basf2 import *
from generators import add_inclusive_continuum_generator
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from ROOT import Belle2

set_random_seed(12345)
decfile = Belle2.FileSystem.findFile('/decfiles/dec/2610030000.dec')


# create path
main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [0])
eventinfosetter.param('expList', [0])
main.add_module(eventinfosetter)

# generate inclusive continuum events
add_inclusive_continuum_generator(main, finalstate='ccbar', particles=['D*+'], userdecfile=decfile, include_conjugates=1)

# detector simulation
add_simulation(main)

# reconstruction
add_reconstruction(main)


# Finally add mdst output
output_filename = "../DstToD0Pi_D0ToHpJm.dst.root"
add_mdst_output(main, filename=output_filename)

# process events and print call statistics
process(main)
print(statistics)
