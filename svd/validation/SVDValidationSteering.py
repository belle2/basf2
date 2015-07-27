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
  <output>SVDValidationOutput.root</output>
  <contact>G. Caria, gcaria@student.unimelb.edu.au</contact>
  <description>This is the SVD validation steering file.</description>
</header>
"""

from basf2 import *
from beamparameters import add_beamparameters
# Individual validation packages
from SVDValidationTTree import *
from SVDValidationTTreeDigit import *
from SVDValidationTTreeSimhit import *
from SVDValidationTTreeEfficiency import *

set_random_seed(12345)

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

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
# Specify number of events to be generated
eventinfosetter.param({'evtNumList': [500], 'runList': [1]})

# Show progress of processing
progress = register_module('Progress')

# Load parameters
gearbox = register_module('Gearbox')

# Create geometry
geometry = register_module('Geometry')
# Select subdetectors to be built
geometry.param('components', ['MagneticField', 'PXD', 'SVD'])
# geometry.param("excludedComponents", "MagneticField")

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters.param("smearVertex", False)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')

# Run simulation
fullsim = register_module('FullSim')

# SVD modules
svddigitizer = register_module('SVDDigitizer')
svdclusterizer = register_module('SVDClusterizer')
# OldValues = { 'NoiseSN'  : 2.5,
#             'SeedSN'   : 5.0,
#             'ClusterSN': 8.0 }
# svdclusterizer.param(OldValues)

# Save output of simulation
output = register_module('RootOutput')
# Set output filename
output.param('outputFileName', '../simulationOutput.root')

# SVD validation modules
svdvalidation = SVDValidationTTree()
svdvalidationdigit = SVDValidationTTreeDigit()
svdvalidationsimhit = SVDValidationTTreeSimhit()
svdefficiency = SVDValidationTTreeEfficiency()

# ============================================================================
# create processing path
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
# main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(evtgeninput)
main.add_module(fullsim)
main.add_module(svddigitizer)
main.add_module(svdclusterizer)
# main.add_module(output)
main.add_module(svdvalidation)
main.add_module(svdvalidationdigit)
main.add_module(svdvalidationsimhit)
main.add_module(svdefficiency)

process(main)

# Print call statistics
print statistics
