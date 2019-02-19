#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
# Simple monopole pair production for monopoles
# with magnetic charge 'mag', electric charge 'el' 
#
######################################################

from basf2 import *
from modularAnalysis import inputMdst
from modularAnalysis import analysis_main
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

analysis_main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=num_events)
pdg.add_particle('monopole', 99666, mass, 0.0, el, 0.5)
pdg.add_particle('anti-monopole', -99666, mass, 0.0, -el, -0.5)

# generate events
pairgen = register_module('PairGen')
pairgen.param('pdgCode', 99666)
pairgen.param('saveBoth', True)
analysis_main.add_module(pairgen)

# define geometry
GEARBOX = register_module('Gearbox')

GEOMETRY = register_module('Geometry')
GEOMETRY_param = {
	'components': [ 'BeamPipe', 'MagneticField', 'PXD', 'SVD', 'CDC', 'ECL', 'ARICH', 'TOP', 'BKLM' ],
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

#SVDDIGITIZER = register_module('SVDDigitizer')
#SVDDIGITIZER_param = {
#	'PoissonSmearing': True,
#	'ElectronicEffects': True
#}
#SVDDIGITIZER.param(SVDDIGITIZER_param)

#CDCDIGITIZER = register_module('CDCDigitizer')
#CDCDIGITIZER.param("Output2ndHit", False)

pxdClusterizer = register_module('PXDClusterizer')

# output
output = register_module('RootOutput')
output.param('outputFileName', 'mplPair_1GeV_test.root')


# Show progress of processing
progress = register_module('ProgressBar')
analysis_main.add_module(GEARBOX)
analysis_main.add_module(GEOMETRY)
analysis_main.add_module(g4sim)
analysis_main.add_module(PXDDIGI)
analysis_main.add_module(pxdClusterizer)

analysis_main.add_module(output)
analysis_main.add_module(progress)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)

