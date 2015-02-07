#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <output>SVDGenData.root</output>
  <contact>cervenkov@ipnp.troja.mff.cuni.cz</contact>
  <description>This steering file is an SVD test.</description>
</header>
"""

from basf2 import *
from SVDClusterPull import *
from SVDEtaDistribution import *
from SVDEnergyLoss import *
import glob

set_random_seed(12345)

# Particle gun module
particlegun = register_module('ParticleGun')
particlegun.param({  # Generate 5 tracks (on average)
                     # Vary the number according to Poisson distribution
                     # Generate pi+, pi-, e+ and e-
                     # with a normal distributed transversal momentum
                     # with a center of 5 GeV and a width of 1 GeV
                     # a normal distributed phi angle,
                     # center of 180 degree and a width of 30 degree
                     # Generate theta angles uniform in cos theta
                     # between 17 and 150 degree
                     # normal distributed vertex generation
                     # around the origin with a sigma of 2cm in the xy plane
                     # and no deviation in z
                     # all tracks sharing the same vertex per event
    'nTracks': 1,
    'varyNTracks': False,
    'pdgCodes': [211, -211, 11, -11],
    'momentumGeneration': 'normal',
    'momentumParams': [2, 0.2],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'thetaGeneration': 'uniformCos',
    'thetaParams': [15, 150],
    'vertexGeneration': 'normal',
    'xVertexParams': [0.0, 0.1],
    'yVertexParams': [0.0, 0.1],
    'zVertexParams': [0.0, 0.1],
    'independentVertices': False,
    })

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
# specify number of events to be generated
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})

# Show progress of processing
progress = register_module('Progress')

# Load parameters
gearbox = register_module('Gearbox')

# Create geometry
geometry = register_module('Geometry')
# Select subdetectors to be built
geometry.param('components', ['MagneticField', 'PXD', 'SVD'])

# Run simulation
fullsim = register_module('FullSim')

# SVD modules
svddigitizer = register_module('SVDDigitizer')
svdclusterizer = register_module('SVDClusterizer')

# SVD validation modules
svdclusterpull = SVDClusterPull()
svdetadistribution = SVDEtaDistribution()
svdenergyloss = SVDEnergyLoss()

# Save to a ROOT file
rootoutput = register_module('RootOutput')
rootoutput.param('outputFileName', 'SVDGenData.root')

# ============================================================================
# create processing path
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(fullsim)
main.add_module(svddigitizer)
main.add_module(svdclusterizer)
main.add_module(svdclusterpull)
main.add_module(svdetadistribution)
main.add_module(svdenergyloss)
main.add_module(rootoutput)

process(main)

# Print call statistics
print statistics
