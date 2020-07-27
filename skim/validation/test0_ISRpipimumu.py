#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: [ee -> ISR pi+pi- [J/psi -> mu+mu-]]

"""
<header>
  <output>../ISRpipimumu.dst.root</output>
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
ISRskimpath = b2.Path()

# specify number of events to be generated
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [5000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [0])
ISRskimpath.add_module(eventinfosetter)

evtgeninput = b2.register_module('EvtGenInput')
evtgeninput.param('userDECFile', Belle2.FileSystem.findFile('/decfiles/dec/2411440000.dec'))
ISRskimpath.add_module(evtgeninput)

# detector simulation
add_simulation(ISRskimpath)

# reconstruction
add_reconstruction(ISRskimpath)

# Finally add mdst output
output_filename = "../ISRpipimumu.dst.root"
add_mdst_output(ISRskimpath, filename=output_filename)

# process events and print call statistics
b2.process(ISRskimpath)
print(b2.statistics)
