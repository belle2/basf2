#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

######################################################
# Simple monopole pair production for monopoles
# with magnetic charge 'mag', electric charge 'el'
#
######################################################

import basf2 as b2

import pdg

# monopole characteristics
mag = 68.5
el = 0
mass = 1
# number of events
num_events = 1

mypath = b2.create_path()

mypath.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=num_events)
pdg.add_particle('monopole', 99666, mass, 0.0, el, 0.5)
pdg.add_particle('anti-monopole', -99666, mass, 0.0, -el, -0.5)

# generate events
pairgen = b2.register_module('PairGen')
pairgen.param('pdgCode', 99666)
pairgen.param('saveBoth', True)
mypath.add_module(pairgen)

# define geometry
GEARBOX = b2.register_module('Gearbox')

GEOMETRY = b2.register_module('Geometry')
GEOMETRY_param = {
    'components': ['BeamPipe', 'MagneticField', 'PXD', 'SVD', 'CDC', 'ECL', 'ARICH', 'TOP', 'KLM'],
    'geometryType': 0
}
GEOMETRY.param(GEOMETRY_param)

# full simulation
g4sim = b2.register_module('FullSim')
g4sim.param('RegisterMonopoles', True)
g4sim.param('MonopoleMagCharge', mag)
g4sim.param('trajectoryStore', 1)

# digitization
PXDDIGI = b2.register_module('PXDDigitizer')
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

pxdClusterizer = b2.register_module('PXDClusterizer')

# output
output = b2.register_module('RootOutput')
output.param('outputFileName', 'mplPair_1GeV_test.root')


# Show progress of processing
progress = b2.register_module('ProgressBar')
mypath.add_module(GEARBOX)
mypath.add_module(GEOMETRY)
mypath.add_module(g4sim)
mypath.add_module(PXDDIGI)
mypath.add_module(pxdClusterizer)

mypath.add_module(output)
mypath.add_module(progress)

# Process the events
b2.process(mypath)

# print out the summary
print(b2.statistics)
