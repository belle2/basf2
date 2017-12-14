#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Common PXD&SVD TestBeam Jan 2014 @ DESY Simulation
# This is the default simulation scenario for VXD beam test WITHOUT telescopes

# Important parameters of the simulation:
events = 100  # Number of events to simulate
fieldOn = True  # Turn field on or off (changes geometry components and digi/clust params)
momentum = 6.0  # GeV/c
momentum_spread = 0.05  # %
theta = 90.0  # degrees
theta_spread = 0.005  # degrees (sigma of gaussian)
phi = 0.0  # degrees
phi_spread = 0.005  # degrees (sigma of gaussian)
gun_x_position = -100.  # cm ... 100cm ... outside magnet + plastic shielding + Al scatterer (air equiv.)
# gun_x_position = 40. # cm ... 40cm ... inside magnet
beamspot_size_y = 0.3  # cm (sigma of gaussian)
beamspot_size_z = 0.3  # cm (sigma of gaussian)

from basf2 import *
# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)
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
# gun position must be in positive values of x.
# Magnet wall starts at 424mm and ends at 590mm
# Plastic 1cm shielding is at 650mm
# Aluminium target at 750mm to "simulate" 15m air between collimator and TB setup
particlegun.param('vertexGeneration', 'normal')
particlegun.param('xVertexParams', [gun_x_position, 0.])
particlegun.param('yVertexParams', [0., beamspot_size_y])
particlegun.param('zVertexParams', [0., beamspot_size_z])
particlegun.param('independentVertices', True)

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [events], 'runList': [1]})

# Show progress of processing
progress = register_module('Progress')

# Load parameters from xml
gearbox = register_module('Gearbox')
# This file contains the VXD (no Telescopes) beam test geometry including the real PCMAG magnetic field
gearbox.param('fileName', 'testbeam/vxd/FullVXDTB2016.xml')

# Create geometry
geometry = register_module('Geometry')
# geometry.param('additionalComponents', ['Target_Pb_10mm'])
if not fieldOn:
    # To turn off magnetic field:
    geometry.param('excludedComponents', ['MagneticField'])

# Full simulation module
simulation = register_module('FullSim')
simulation.param('StoreAllSecondaries', True)
# Uncomment the following lines to get particle tracks visualization
# simulation.param('EnableVisualization', True)
# simulation.param('UICommands', ['/vis/open VRML2FILE', '/vis/drawVolume',
#                 '/vis/scene/add/axes 0 0 0 100 mm',
#                 '/vis/scene/add/trajectories smooth',
#                 '/vis/modeling/trajectories/create/drawByCharge'])

# for simulation:
RunNoVXD = 0

# Masking for sensors
pxd_mask1 = 'testbeam/vxd/data/PXD1_MaskListRun'+str(RunNoVXD)+'.xml'  # Gear path
pxd_mask2 = 'testbeam/vxd/data/PXD2_MaskListRun'+str(RunNoVXD)+'.xml'  # Gear path
svd_mask1 = 'testbeam/vxd/data/SVD3_MaskListRun'+str(RunNoVXD)+'.xml'  # Gear path
svd_mask2 = 'testbeam/vxd/data/SVD4_MaskListRun'+str(RunNoVXD)+'.xml'  # Gear path
svd_mask3 = 'testbeam/vxd/data/SVD5_MaskListRun'+str(RunNoVXD)+'.xml'  # Gear path
svd_mask4 = 'testbeam/vxd/data/SVD6_MaskListRun'+str(RunNoVXD)+'.xml'  # Gear path
tel_mask1 = 'testbeam/vxd/data/Tel1_MaskListRun'+str(RunNoVXD)+'.xml'  # Gear path
tel_mask2 = 'testbeam/vxd/data/Tel2_MaskListRun'+str(RunNoVXD)+'.xml'  # Gear path
tel_mask3 = 'testbeam/vxd/data/Tel3_MaskListRun'+str(RunNoVXD)+'.xml'  # Gear path
tel_mask4 = 'testbeam/vxd/data/Tel4_MaskListRun'+str(RunNoVXD)+'.xml'  # Gear path
tel_mask5 = 'testbeam/vxd/data/Tel5_MaskListRun'+str(RunNoVXD)+'.xml'  # Gear path
tel_mask6 = 'testbeam/vxd/data/Tel6_MaskListRun'+str(RunNoVXD)+'.xml'  # Gear path

# Masking in fact:
PXDSort1 = register_module('PXDDigitSorter')
PXDSort1.param('ignoredPixelsListName', pxd_mask1)
PXDSort2 = register_module('PXDDigitSorter')
PXDSort2.param('ignoredPixelsListName', pxd_mask2)
# PXD clusterizer
PXDClust = register_module('PXDClusterizer')
# PXD DQM modules
# PXD_DQM = register_module('PXDDQM')
# raw_pxd_dqm = register_module('pxdRawDQM')

# SVD clusterizer
SVDClust = register_module('SVDClusterizer')
SVDClust.param('TanLorentz_holes', 0.)
SVDClust.param('TanLorentz_electrons', 0.)
# SVDClust.param('applyTimeWindow', True)
# SVDClust.param('triggerTime', -20.0)
# SVDClust.param('acceptanceWindowSize', 20.0)

SVDSort1 = register_module('SVDDigitSorter')
SVDSort1.param('mergeDuplicates', False)
SVDSort1.param('ignoredStripsListName', svd_mask1)
SVDSort2 = register_module('SVDDigitSorter')
SVDSort2.param('mergeDuplicates', False)
SVDSort2.param('ignoredStripsListName', svd_mask2)
SVDSort3 = register_module('SVDDigitSorter')
SVDSort3.param('mergeDuplicates', False)
SVDSort3.param('ignoredStripsListName', svd_mask3)
SVDSort4 = register_module('SVDDigitSorter')
SVDSort4.param('mergeDuplicates', False)
SVDSort4.param('ignoredStripsListName', svd_mask4)

# SVD DQM module
# svd_dqm = register_module('SVDDQM')
# svd_dqm.param('SVDStrongMasking', 1)

# Masking in fact:
TelSort1 = register_module('TelDigitSorter')
TelSort1.param('digits', 'TelDigits')
TelSort1.param('ignoredPixelsListName', tel_mask1)
TelSort2 = register_module('TelDigitSorter')
TelSort2.param('digits', 'TelDigits')
TelSort2.param('ignoredPixelsListName', tel_mask2)
TelSort3 = register_module('TelDigitSorter')
TelSort3.param('digits', 'TelDigits')
TelSort3.param('ignoredPixelsListName', tel_mask3)
TelSort4 = register_module('TelDigitSorter')
TelSort4.param('digits', 'TelDigits')
TelSort4.param('ignoredPixelsListName', tel_mask4)
TelSort5 = register_module('TelDigitSorter')
TelSort5.param('digits', 'TelDigits')
TelSort5.param('ignoredPixelsListName', tel_mask5)
TelSort6 = register_module('TelDigitSorter')
TelSort6.param('digits', 'TelDigits')
TelSort6.param('ignoredPixelsListName', tel_mask6)

TelClust = register_module('TelClusterizer')

# Path construction
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module('PXDDigitizer')
main.add_module(PXDSort1)
main.add_module(PXDSort2)
main.add_module(PXDClust)
main.add_module('SVDDigitizer')
main.add_module(SVDSort1)
main.add_module(SVDSort2)
main.add_module(SVDSort3)
main.add_module(SVDSort4)
main.add_module(SVDClust)
main.add_module('TelDigitizer')
main.add_module(TelSort1)
main.add_module(TelSort2)
main.add_module(TelSort3)
main.add_module(TelSort4)
main.add_module(TelSort5)
main.add_module(TelSort6)
main.add_module(TelClust)


main.add_module('ExportGeometry', Filename='TBGeometry.root')
main.add_module('RootOutput', outputFileName='TBSimulation.root')
# main.add_module('OverlapChecker')
# main.add_module('Display', fullGeometry=True)

# Process events
process(main)

# Print call statistics
print(statistics)
