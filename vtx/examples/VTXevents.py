#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#####################################################################
# Just to create some ParticleGun events and fill the VTX containers
# to see if everything works
#####################################################################


import basf2 as b2
import ROOT as r
from beamparameters import add_beamparameters

import os
import sys
import random

# ---------------------------------------------------------------------------------------

# Set Random Seed for reproducable simulation. 0 means really random.
rndseed = 11111
# assume the first argument is the random seed
if(len(sys.argv) > 1):
    rndseed = sys.argv[1]

outputDir = './'
# assume second argument is the output directory
if(len(sys.argv) > 2):
    outputDir = sys.argv[2]

b2.set_random_seed(rndseed)


# Set log level. Can be overridden with the "-l LEVEL" flag for basf2.
b2.set_log_level(b2.LogLevel.WARNING)

# ---------------------------------------------------------------------------------------
main = b2.create_path()

eventinfosetter = b2.register_module('EventInfoSetter')
# default phase3 geometry:
exp_number = 0
eventinfosetter.param("expList", [exp_number])
main.add_module(eventinfosetter)

eventinfoprinter = b2.register_module('EventInfoPrinter')
main.add_module(eventinfoprinter)

progress = b2.register_module('Progress')
main.add_module(progress)

# ---------------------------------------------------------------------------------------
# Simulation Settings:


# randomize the vertex position (flatly distributed) to make the sectormap more robust wrt. changing beam position
# minima and maxima of the beam position given in cm
random.seed(rndseed)
vertex_x_min = -0.1
vertex_x_max = 0.1
vertex_y_min = -0.1
vertex_y_max = 0.1
vertex_z_min = -0.5
vertex_z_max = 0.5

vertex_x = random.uniform(vertex_x_min, vertex_x_max)
vertex_y = random.uniform(vertex_y_min, vertex_y_max)
vertex_z = random.uniform(vertex_z_min, vertex_z_max)

print("WARNING: setting non-default beam vertex at x= " + str(vertex_x) + " y= " + str(vertex_y) + " z= " + str(vertex_z))

# Particle Gun:
# One can add more particle gun modules if wanted.

# additional flatly smear the muon vertex between +/- this value
vertex_delta = 0.005  # in cm

particlegun = b2.register_module('ParticleGun')
particlegun.logging.log_level = b2.LogLevel.WARNING
param_pGun = {
    'pdgCodes': [13, -13],   # 13 = muon --> negatively charged!
    'nTracks': 10,
    'momentumGeneration': 'uniform',
    'momentumParams': [0.1, 4],
    'vertexGeneration': 'uniform',
    'phiGeneration': 'uniform',
    'phiParams': [0, 250],
    'xVertexParams': [vertex_x - vertex_delta, vertex_x + vertex_delta],            # in cm...
    'yVertexParams': [vertex_y - vertex_delta, vertex_y + vertex_delta],
    'zVertexParams': [vertex_z - vertex_delta, vertex_z + vertex_delta]
}

particlegun.param(param_pGun)
main.add_module(particlegun)

# ---------------------------------------------------------------------------------------
# Geometry parameter loader
gearbox = b2.register_module('Gearbox')

# Geometry builder
geometry = b2.register_module('Geometry')
geometry.param('useDB', False)
geometry.param('excludedComponents', ['PXD', 'SVD'])
geometry.param('additionalComponents', ['VTX-CMOS-7layer'])

main.add_module(gearbox)
main.add_module(geometry)

# G4 simulation
main.add_module('FullSim')

# Digitizer
main.add_module('VTXDigitizer')
# needed for fitting

main.add_module('SetupGenfitExtrapolation')

# Clusturizer
main.add_module('VTXClusterizer')

# main.add_module('SimpleVTXSpacePointCreator')

# build the name of the output file
outputFileName = outputDir + './' + "SimEvts_Belle2_" + str(rndseed) + '.root'

# Root output. Default filename can be overriden with '-o' basf2 option.
rootOutput = b2.register_module('RootOutput')
rootOutput.param('outputFileName', outputFileName)
# to save some space exclude everything except stuff needed for tracking
rootOutput.param('excludeBranchNames', ["ARICHAeroHits",
                                        "ARICHDigits",
                                        "ARICHSimHits",
                                        "BKLMDigits",
                                        "BKLMSimHitPositions",
                                        "BKLMSimHits",
                                        "CDCHits",
                                        "CDCHits4Trg",
                                        "CDCSimHits",
                                        "CDCSimHitsToCDCHits4Trg",
                                        "ECLDigits",
                                        "ECLDsps",
                                        "ECLHits",
                                        "ECLSimHits",
                                        "ECLTrigs",
                                        "ECLDiodeHits",
                                        "EKLMDigits",
                                        "EKLMSimHits",
                                        "TOPBarHits",
                                        "TOPDigits",
                                        "TOPRawDigits",
                                        "TOPSimHits"
                                        ])
main.add_module(rootOutput)

b2.print_path(main)

b2.process(main)
print(b2.statistics)
