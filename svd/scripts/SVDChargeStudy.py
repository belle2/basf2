#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import svd as svd
from SVDChargeSharing import SVDChargeSharing
b2.logging.log_level = b2.LogLevel.WARNING

# Particle gun module
particlegun = b2.register_module('ParticleGun')
# Create Event information
eventinfosetter = b2.register_module('EventInfoSetter')
# Show progress of processing
progress = b2.register_module('Progress')
# Load parameters
gearbox = b2.register_module('Gearbox')
# Create geometry
geometry = b2.register_module('Geometry')
# RootOutput
output = b2.register_module('RootOutput')

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

# create processing path
main = b2.create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module("FullSim")
svd.add_svd_simulation(main)
svd.add_svd_reconstruction(main)
main.add_module(analyze)

# generate events
b2.process(main)

# show call statistics
print(b2.statistics)
