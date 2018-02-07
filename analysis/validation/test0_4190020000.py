#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: [Upsilon6S_piZb10610_pihb1p]

"""
<header>
  <output>../4190020000.dst.root</output>
  <contact>karim.trabelsi@kek.jp</contact>
</header>
"""

import pdg
from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from beamparameters import add_beamparameters
from HLTTrigger import add_HLT_Y4S
from ROOT import Belle2
import glob

set_random_seed(12345)

# background (collision) files
bg = glob.glob('./BG/[A-Z]*.root')

# create path
main = create_path()

# set the BeamParameters for running at Y(6S)
beamparameters = add_beamparameters(main, "Y6S")
print_params(beamparameters)

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('ParentParticle', 'Upsilon(6S)')
evtgeninput.param('userDECFile', Belle2.FileSystem.findFile('/decfiles/dec/4190020000.dec'))
main.add_module(evtgeninput)

# Y(6S) pdl file
pdg.load(Belle2.FileSystem.findFile('/decfiles/dec/Y6S.pdl'))

# detector simulation
# add_simulation(main, bkgfiles=bg)
add_simulation(main)

# reconstruction
add_reconstruction(main)

# HLT physics trigger
add_HLT_Y4S(main)

# Finally add mdst output
output_filename = "../4190020000.dst.root"
add_mdst_output(main, filename=output_filename)

# process events and print call statistics
process(main)
print(statistics)
