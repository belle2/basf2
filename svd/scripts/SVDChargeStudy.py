#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from SVDChargeSharing import SVDChargeSharing
logging.log_level = LogLevel.WARNING

# Particle gun module
particlegun = register_module('ParticleGun')
# Create Event information
eventinfosetter = register_module('EventInfoSetter')
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
# Run simulation
simulation = register_module('FullSim')
# SVD digitization module
svddigi = register_module('SVDDigitizer')
# SVD clustering module
svdclust = register_module('SVDClusterizer')
# RootOutput
output = register_module('RootOutput')

analyze = SVDChargeSharing()

# Specify number of events to generate
eventinfosetter.param('evtNumList', [100000])

# Set parameters for particlegun
particlegun.param({
    'nTracks': 1,
    'pdgCodes': [211, -211, 11, -11],
    'momentumGeneration': 'normal',
    'momentumParams': [3, 0.2],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'thetaGeneration': 'uniform',
    'thetaParams': [89, 91],
    'vertexGeneration': 'normal',
    'xVertexParams': [0.0, 0.1],
    'yVertexParams': [0.0, 0.1],
    'zVertexParams': [0.0, 5.0]
})

# Override backplane capacitances in gearbox:

factor = 2.0

if (factor != 1.0):
    base_params = {
        'Barrel': {'U': 0.12, 'V': 0.39},
        'Layer3': {'U': 0.08, 'V': 0.26},
        'Slanted': {'U': 0.11, 'V': 0.42}
    }

    gearbox.param({
        "overridePrefix": "//DetectorComponent[@name='SVD']//Components/",
        "override": [
            (('Sensor' if sensor == 'Layer3' else 'SensorBase') +
                '[@type=\"' + sensor + '\"]/Active/BackplaneCapacitance' + coordinate,
             str(factor * value), 'pF')
            for (sensor, vals) in base_params.items() for (coordinate, value) in vals.items()
        ],
    })

# Select subdetectors to be built
geometry.param('components', ['SVD'])

# svddigi.param('statisticsFilename', 'digi.root')
svddigi.param('ElectronicEffects', True)

# create processing path
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(svddigi)
main.add_module(svdclust)
main.add_module(analyze)

# generate events
process(main)

# show call statistics
print(statistics)
