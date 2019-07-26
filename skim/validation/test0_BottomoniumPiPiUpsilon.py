#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: [Upsilon3S pi+pi- Y(1S)]

"""
<header>
  <output>../BottomoniumPiPiUpsilon.dst.root</output>
  <contact>jiasen@buaa.edu.cn</contact>
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

# background (collision) files
bg = glob.glob('./BG/[A-Z]*.root')

# create a new path
BottomoniumPiPiUpsilonskimpath = Path()

# set the BeamParameters for running at Y(3S)
beamparameters = add_beamparameters(BottomoniumPiPiUpsilonskimpath, "Y3S")
print_params(beamparameters)

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [0])
BottomoniumPiPiUpsilonskimpath.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('ParentParticle', 'Upsilon(3S)')
evtgeninput.param('userDECFile', Belle2.FileSystem.findFile('/decfiles/dec/1730840030.dec'))
BottomoniumPiPiUpsilonskimpath.add_module(evtgeninput)

# detector simulation
add_simulation(BottomoniumPiPiUpsilonskimpath)

# reconstruction
add_reconstruction(BottomoniumPiPiUpsilonskimpath)

# Finally add mdst output
output_filename = "../BottomoniumPiPiUpsilon.dst.root"
add_mdst_output(BottomoniumPiPiUpsilonskimpath, filename=output_filename)

# process events and print call statistics
process(BottomoniumPiPiUpsilonskimpath)
print(statistics)
