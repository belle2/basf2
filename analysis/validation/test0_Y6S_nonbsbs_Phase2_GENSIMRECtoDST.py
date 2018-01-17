#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>../Y6S_nonbsbs_Phase2.dst.root</output>
  <contact>Cate MacQueen, cmq.centaurus@gmail.com</contact>
</header>
"""

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output
from beamparameters import add_beamparameters
from ROOT import Belle2

set_random_seed(10000)

main = create_path()

# set the BeamParameters for running at Y(6S)
beamparameters = add_beamparameters(main, "Y6S")
print_params(beamparameters)

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [100])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# specify Phase2 geometry--comment below three lines to use full geometry
gearbox = register_module('Gearbox')
gearbox.param('fileName', '/geometry/Beast2_phase2.xml')
main.add_module(gearbox)

evtgen = register_module('EvtGenInput')
evtgen.param('ParentParticle', "Upsilon(6S)")
evtgen.param('userDECFile', Belle2.FileSystem.findFile('generators/evtgen/decayfiles/Y6S_nonbsbs.dec'))
main.add_module(evtgen)
print_params(evtgen)
import pdg
pdg.load(Belle2.FileSystem.findFile('/decfiles/dec/Y6S.pdl'))  # to load a different file

# simulation
add_simulation(main)

# reconstruction
add_reconstruction(main)

# dst and mdst output
output = register_module('RootOutput')
output.param('outputFileName', '../Y6S_nonbsbs_Phase2.dst.root')
main.add_module(output)

# Go!
process(main)

# Print call statistics
print(statistics)
