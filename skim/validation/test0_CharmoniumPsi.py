#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: [J/psi psi(2S)]

"""
<header>
  <output>../CharmoniumPsi.dst.root</output>
  <contact>jiasen@buaa.edu.cn</contact>
</header>
"""

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from ROOT import Belle2
import glob

b2.set_random_seed(12345)

# background (collision) files
bg = glob.glob('./BG/[A-Z]*.root')

# create a new path
path = b2.Path()

# specify number of events to be generated
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [500])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [0])
path.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = b2.register_module('EvtGenInput')
evtgeninput.param('userDECFile', Belle2.FileSystem.findFile('decfiles/dec/1211530000.dec'))
path.add_module(evtgeninput)

# detector simulation
add_simulation(path)

# reconstruction
add_reconstruction(path)

# Finally add mdst output
output_filename = "../CharmoniumPsi.dst.root"
add_mdst_output(path, filename=output_filename)

# process events and print call statistics
b2.process(path)
print(b2.statistics)
