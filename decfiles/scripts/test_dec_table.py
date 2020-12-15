#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: script to test decay tables

"""
<header>
  <output>../test.dst.root</output>
  <contact>morda@pd.infn.it</contact>
</header>
"""


from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
import glob
import sys


event_type = str(sys.argv[1])
set_random_seed(12345)

# background (collision) files
bg = glob.glob('./BG/[A-Z]*.root')

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
evtgeninput.param('userDECFile', find_file('/decfiles/dec/{0}.dec'.format(event_type)))
main.add_module(evtgeninput)

# detector simulation
# add_simulation(main, bkgfiles=bg)
add_simulation(main)

# reconstruction
add_reconstruction(main)

# Finally add mdst output
output_filename = "test.dst.root"
add_mdst_output(main, filename=output_filename)

# process events and print call statistics
process(main)
print(statistics)
