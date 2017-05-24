#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: [B0 -> rho0 gamma]cc

"""
<header>
  <output>../1110021010.dst.root</output>
  <contact>karim.trabelsi@kek.jp</contact>
</header>
"""

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from HLTTrigger import add_HLT_Y4S
from ROOT import Belle2
import glob

# background (collision) files
bg = glob.glob('./BG/[A-Z]*.root')

# create path
main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [100])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('userDECFile', Belle2.FileSystem.findFile('/decfiles/dec/1110021010.dec'))
main.add_module(evtgeninput)

# detector simulation
# add_simulation(main, bkgfiles=bg)
add_simulation(main)

# HLT L3 simulation
main.add_module('Level3')

# reconstruction
add_reconstruction(main)

# HLT physics trigger
add_HLT_Y4S(main)

# Finally add mdst output
output_filename = "../1110021010.dst.root"
add_mdst_output(main, filename=output_filename)

# process events and print call statistics
process(main)
print(statistics)
