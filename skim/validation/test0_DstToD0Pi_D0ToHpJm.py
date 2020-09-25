#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: [D0 -> K- pi+ ]cc

"""
<header>
  <output>../DstToD0Pi_D0ToHpJm.dst.root</output>
  <contact>gonggd@mail.ustc.edu.cn</contact>
</header>
"""

import basf2 as b2
from generators import add_inclusive_continuum_generator
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from ROOT import Belle2

b2.set_random_seed(12345)
decfile = Belle2.FileSystem.findFile('/decfiles/dec/2610030000.dec')


# create path
main = b2.create_path()

# specify number of events to be generated
eventinfosetter = b2.register_module('EventInfoSetter')
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
b2.process(main)
print(b2.statistics)
