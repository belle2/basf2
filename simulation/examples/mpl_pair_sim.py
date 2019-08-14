#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
# Simple monopole pair production for monopoles
# with magnetic charge 'mag', electric charge 'el'
#
######################################################

from basf2 import *
from modularAnalysis import inputMdst
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output

import sys
import glob
import pdg

# monopole characteristics
mag = 68.5
el = 0
mass = 1
# number of events
num_events = 1

mypath = create_path()

mypath.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=num_events)
pdg.add_particle('monopole', 99666, mass, 0.0, el, 0.5)
pdg.add_particle('anti-monopole', -99666, mass, 0.0, -el, -0.5)

# generate events
pairgen = register_module('PairGen')
pairgen.param('pdgCode', 99666)
pairgen.param('saveBoth', True)
mypath.add_module(pairgen)

# define geometry
GEARBOX = register_module('Gearbox')

GEOMETRY = register_module('Geometry')
GEOMETRY_param = {
    'components': ['BeamPipe', 'MagneticField', 'PXD', 'SVD', 'CDC', 'ECL', 'ARICH', 'TOP', 'KLM'],
    'geometryType': 0
}
GEOMETRY.param(GEOMETRY_param)

# full simulation
g4sim = register_module('FullSim')
g4sim.param('RegisterMonopoles', True)
g4sim.param('MonopoleMagCharge', mag)
g4sim.param('trajectoryStore', 1)

# digitization
PXDDIGI = register_module('PXDDigitizer')
PXDDIGI_param = {
    'Digits': 'PXDDigits',
    'PoissonSmearing': True,
    'ElectronicEffects': True
}
PXDDIGI.param(PXDDIGI_param)

# SVDDIGITIZER = register_module('SVDDigitizer')
# SVDDIGITIZER_param = {
#     'PoissonSmearing': True,
#     'ElectronicEffects': True
# }
# SVDDIGITIZER.param(SVDDIGITIZER_param)

# CDCDIGITIZER = register_module('CDCDigitizer')
# CDCDIGITIZER.param("Output2ndHit", False)

pxdClusterizer = register_module('PXDClusterizer')

# output
output = register_module('RootOutput')
output.param('outputFileName', 'mplPair_1GeV_test.root')


# Show progress of processing
progress = register_module('ProgressBar')
mypath.add_module(GEARBOX)
mypath.add_module(GEOMETRY)
mypath.add_module(g4sim)
mypath.add_module(PXDDIGI)
mypath.add_module(pxdClusterizer)

mypath.add_module(output)
mypath.add_module(progress)

# Process the events
process(mypath)

# print out the summary
print(statistics)
