#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Common PXD&SVD TestBeam Jan 2016 @ DESY Simulation
# This is the default simulation scenario for VXD beam test

from basf2 import *
# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# Important parameters of the simulation:
fieldOn = True  # Turn field on or off (changes geometry components and digi/clust params)
momentum = 5.0  # GeV/c
momentum_spread = 0.05  # %
theta = 90.0  # degrees
theta_spread = 0.005  # degrees (sigma of gaussian)
phi = 0.0  # degrees
phi_spread = 0.005  # degrees (sigma of gaussian)
gun_x_position = -100.  # cm ... 100cm ... outside magnet + plastic shielding + Al scatterer (air equiv.)
# gun_x_position = 40. # cm ... 40cm ... inside magnet
beamspot_size_y = 0.3  # cm (sigma of gaussian)
beamspot_size_z = 0.3  # cm (sigma of gaussian)

main = create_path()
main.add_module('EventInfoSetter', evtNumList=[100])
main.add_module('Progress')
main.add_module('Gearbox', fileName='testbeam/vxd/FullVXDTB2016.xml')

if fieldOn:
    main.add_module('Geometry')
else:
    main.add_module('Geometry', excludedComponents=['MagneticField'])

# ParticleGun
particlegun = register_module('ParticleGun')
# number of primaries per event
particlegun.param('nTracks', 1)
# DESY electrons:
particlegun.param('pdgCodes', [11])
# momentum magnitude 2 GeV/c or something above or around.
# At DESY we can have up to 6 GeV/c(+-5%) electron beam.
# Beam divergence divergence and spot size is adjusted similar to reality
# See studies of Benjamin Schwenker
particlegun.param('momentumGeneration', 'normal')
particlegun.param('momentumParams', [momentum, momentum * momentum_spread])
# momentum direction must be around theta=90, phi=180
particlegun.param('thetaGeneration', 'normal')
particlegun.param('thetaParams', [theta, theta_spread])
particlegun.param('phiGeneration', 'normal')
particlegun.param('phiParams', [phi, phi_spread])
particlegun.param('vertexGeneration', 'normal')
particlegun.param('xVertexParams', [gun_x_position, 0.])
particlegun.param('yVertexParams', [0., beamspot_size_y])
particlegun.param('zVertexParams', [0., beamspot_size_z])
particlegun.param('independentVertices', True)

main.add_module(particlegun)

main.add_module('FullSim', StoreAllSecondaries=True)
main.add_module('PXDDigitizer')
main.add_module('SVDDigitizer')
main.add_module('TelDigitizer')


main.add_module('RootOutput')
# main.add_module('Display', fullGeometry=True)

process(main)
print(statistics)
