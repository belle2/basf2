#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# avoid race conditions beetween pyroot and GUI thread
from ROOT import PyConfig
PyConfig.StartGuiThread = False

from simulation import add_simulation
from reconstruction import add_reconstruction


particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [211, -211, 321, -321])
particlegun.param('nTracks', 2)

eventinfosetter = register_module('EventInfoSetter')

eventinfosetter.param('evtNumList', [5])

# Set output filename
output = register_module('RootOutput')
output.param('outputFileName', 'streamer_test.root')

# ============================================================================

main = create_path()
main.add_module(eventinfosetter)
main.add_module(particlegun)

# detecor simulation
components = ['MagneticField',
              'BeamPipe',
              #'PXD',
              #'SVD',
              'CDC']
add_simulation(main, components)
# or add_simulation(main) to simulate all detectors

add_reconstruction(main, components)

# output path
main.add_module(output)

process(main)


#load file and see if it can be read properly
from ROOT import TFile
from ROOT import TTree
file = TFile('streamer_test.root')
tree = file.Get('tree')

#seems to be ok, most of the time
tree.Project("", "MCParticles.m_pdg")

# Used to create problems when genfit::AbsTrackRep didn't have schema
# evolution. With no '+' in linkdef, this crashes
tree.Project("", "abs(MCParticles.m_pdg)")

# wether TTreeFormula chokes on something depends on alphabetical order...
# so this should get it to iterate over the entire contents
tree.Project("", "ZZZ.Doesnt.Exist")

# also test using some class members
tree.Project("", "GF2Tracks.getNumPoints()")

import os
os.remove('streamer_test.root')
