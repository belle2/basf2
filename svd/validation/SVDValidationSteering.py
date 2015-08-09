#!/usr/bin/env python
# -*- coding: utf-8 -*-

#####################################################################
#
# History:
# 21 Jul 2015, Commit
#
#####################################################################


"""
<header>
  <input>EvtGenSimRec.root</input>
  <output>SVDValidationOutput.root</output>
  <output>SVDValidationTTree.root</output>
  <output>SVDValidationTTreeDigit.root</output>
  <output>SVDValidationTTreeSimhit.root</output>
  <output>SVDValidationTTreeEfficiency.root</output>
  <contact>G. Caria, gcaria@student.unimelb.edu.au</contact>
  <description>This is the SVD validation steering file.</description>
</header>
"""

from basf2 import *
# Individual validation packages
from SVDValidationTTree import *
from SVDValidationTTreeDigit import *
from SVDValidationTTreeSimhit import *
from SVDValidationTTreeEfficiency import *

# set_random_seed(12345)

main = create_path()

# Particle gun module
# particlegun = register_module('ParticleGun')
# particlegun.param({
#    'nTracks': 1,
#    'varyNTracks': False,
#    'pdgCodes': [13, -13],
#    'momentumGeneration': 'normal',
#    'momentumParams': [3, 1],
#    'phiGeneration': 'uniform',
#    'phiParams': [0, 360],
#    'thetaGeneration': 'uniformCos',
#    'thetaParams': [15, 150],
#    'vertexGeneration': 'normal',
#    'xVertexParams': [0.0, 0.02],
#    'yVertexParams': [0.0, 0.02],
#    'zVertexParams': [0.0, 0.02],
#    'independentVertices': False,
#    })
# main.add_module(particlegun)

# Create Event information
# eventinfosetter = register_module('EventInfoSetter')
# Specify number of events to be generated
# eventinfosetter.param({'evtNumList': [500], 'runList': [1]})
# main.add_module(eventinfosetter)

# main.add_module('EventInfoPrinter')
# Show progress of processing
# progress = register_module('Progress')
# main.add_module(progress)

# Load parameters
# gearbox = register_module('Gearbox')
# main.add_module(gearbox)

# Create geometry
# geometry = register_module('Geometry')
# Select subdetectors to be built
# geometry.param('components', ['MagneticField', 'BeamPipe', 'PXD', 'SVD'])
# geometry.param("excludedComponents", "MagneticField")
# main.add_module(geometry)

# beam parameters
# from beamparameters import add_beamparameters
# add_beamparameters(main, "Y4S")
# beamparameters.param("smearVertex", False)

# generate BBbar events
# evtgeninput = register_module('EvtGenInput')
# main.add_module(evtgeninput)

# Run simulation
# fullsim = register_module('FullSim')
# main.add_module(fullsim)

# SVD modules
# svddigitizer = register_module('SVDDigitizer')
# main.add_module(svddigitizer)
# svdclusterizer = register_module('SVDClusterizer')
# OldValues = { 'NoiseSN'  : 2.5,
#             'SeedSN'   : 5.0,
#             'ClusterSN': 8.0 }
# svdclusterizer.param(OldValues)
# main.add_module(svdclusterizer)

input = register_module('RootInput')
input.param('inputFileName', '../EvtGenSimRec.root')
main.add_module(input)

# SVD validation modules
svdvalidation = SVDValidationTTree()
main.add_module(svdvalidation)
svdvalidationdigit = SVDValidationTTreeDigit()
main.add_module(svdvalidationdigit)
svdvalidationsimhit = SVDValidationTTreeSimhit()
main.add_module(svdvalidationsimhit)
svdefficiency = SVDValidationTTreeEfficiency()
main.add_module(svdefficiency)

process(main)

# Print call statistics
print statistics
