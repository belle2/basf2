#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: [B_s0 -> (KS0 -> pi+ pi-) (KS0 -> pi+ pi-)]cc

"""
<header>
  <output>../1310040140.dst.root</output>
  <contact>karim.trabelsi@kek.jp</contact>
</header>
"""

import pdg
from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from beamparameters import add_beamparameters
from ROOT import Belle2
import glob

set_random_seed(12345)

# create path
main = create_path()

# set the BeamParameters for running at Y(5S)
beamparameters = add_beamparameters(main, "Y5S")
print_params(beamparameters)

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [0])
eventinfosetter.param('expList', [0])
main.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('ParentParticle', 'Upsilon(5S)')
evtgeninput.param('userDECFile', Belle2.FileSystem.findFile('/decfiles/dec/1310040140.dec'))
main.add_module(evtgeninput)

pdg.load(Belle2.FileSystem.findFile('/decfiles/dec/Y5S.pdl'))  # to load a different file

# detector simulation
add_simulation(main)

# reconstruction
add_reconstruction(main)


# Finally add mdst output
output_filename = "../1310040140.dst.root"
add_mdst_output(main, filename=output_filename)

# process events and print call statistics
process(main)
print(statistics)
