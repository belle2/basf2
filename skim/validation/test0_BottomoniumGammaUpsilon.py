#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: [Upsilon3S -> gam chib -> gam Y(1S)(ll)]

"""
<header>
  <output>../BottomoniumGammaUpsilon.dst.root</output>
  <contact>jiasen@buaa.edu.cn</contact>
</header>
"""

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from beamparameters import add_beamparameters
from ROOT import Belle2
import glob

b2.set_random_seed(12345)

# background (collision) files
bg = glob.glob('./BG/[A-Z]*.root')

# create a new path
BottomoniumGammaUpsilonskimpath = b2.Path()

# set the BeamParameters for running at Y(3S)
beamparameters = add_beamparameters(BottomoniumGammaUpsilonskimpath, "Y3S")
b2.print_params(beamparameters)

# specify number of events to be generated
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [5000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [0])
BottomoniumGammaUpsilonskimpath.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = b2.register_module('EvtGenInput')
evtgeninput.param('ParentParticle', 'Upsilon(3S)')
evtgeninput.param('userDECFile', Belle2.FileSystem.findFile('/decfiles/dec/1730822030.dec'))
BottomoniumGammaUpsilonskimpath.add_module(evtgeninput)

# detector simulation
add_simulation(BottomoniumGammaUpsilonskimpath)

# reconstruction
add_reconstruction(BottomoniumGammaUpsilonskimpath)


# Finally add mdst output
output_filename = "../BottomoniumGammaUpsilon.dst.root"
add_mdst_output(BottomoniumGammaUpsilonskimpath, filename=output_filename)

# process events and print call statistics
b2.process(BottomoniumGammaUpsilonskimpath)
print(b2.statistics)
