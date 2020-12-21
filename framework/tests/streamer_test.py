#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import basf2
from ROOT import TFile

from b2test_utils import skip_test_if_light
skip_test_if_light()  # light builds don't contain simulation, reconstruction etc; skip before trying to import  # noqa

from simulation import add_simulation
from reconstruction import add_reconstruction

main = basf2.Path()
main.add_module('EventInfoSetter', evtNumList=[5])
main.add_module('ParticleGun', pdgCodes=[211, -211, 321, -321], nTracks=2)

# detector simulation
components = ['MagneticField', 'BeamPipe', 'CDC']
simulation_components = ['CDC']
add_simulation(main, simulation_components)
add_reconstruction(main, simulation_components)
basf2.set_module_parameters(main, type="Geometry", useDB=False, components=components)

# output path
main.add_module('RootOutput', outputFileName='streamer_test.root')

basf2.process(main)

# load file and see if it can be read properly
tfile = TFile('streamer_test.root')
tree = tfile.Get('tree')

# seems to be ok, most of the time
tree.Project("", "MCParticles.m_pdg")

# Used to create problems when genfit::AbsTrackRep didn't have schema
# evolution. With no '+' in linkdef, this crashes
tree.Project("", "abs(MCParticles.m_pdg)")

# wether TTreeFormula chokes on something depends on alphabetical order...
# so this should get it to iterate over the entire contents
tree.Project("", "ZZZ.Doesnt.Exist")

# also test using some class members
tree.Project("", "GF2Tracks.getNumPoints()")

os.remove('streamer_test.root')
