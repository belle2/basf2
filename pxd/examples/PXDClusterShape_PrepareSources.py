#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##############################################################################
#
# This is an example steering file to use PXD shape correction.
# Shape correction is realize in three steps, se exampes:
#   pxd/examples/PXDClasterShape_PrepareSources.py
#   pxd/examples/PXDClasterShape_CalculateCorrections.py
#   pxd/examples/PXDClasterShape_ApplyCorrections.py
# It uses ParicleGun module to generate tracks,
# (see "generators/example/ParticleGunFull.py" for detailed usage)
# builds PXD geometry, performs geant4 and PXD simulation,
# and stores output in a root file.
#
# "UseTracks" to use track information (default) or simulations
# "CalibrationKind" to run this task:
# 1) (default) standard calibration based on realistic physics or real data
# 2) special calbration for full range of angles, only for simulation!
#    this case is split for every kind of pixel size (4 cases)
#    partical gun is locate close to sensor with giving pixel kind
#    there are storing only data for giving pixel kind
# "PixelKind" only for CalibrationKind=2 for every kind of pixel
#    PixelKind=0: 55 um pitch: sensor 1.3.2 , vertex: x: [-0.2050,-0.2], y: [1.35], z: [0.7,0.7055]
#    PixelKind=1: 60 um pitch: sensor 1.3.2 , vertex: x: [-0.2050,-0.2], y: [1.35], z: [-1.5,-1.5060]
#    PixelKind=2: 70 um pitch: sensor 2.4.2 , vertex: x: [-0.2050,-0.2], y: [2.18], z: [0.9,0.9070]
#    PixelKind=3: 85 um pitch: sensor 2.4.2 , vertex: x: [-0.2050,-0.2], y: [2.18], z: [-2.0,-2.0085]
#    TODO: Need to check influence of orientation of u & v axes for corections
#
# There is possibility to call this macro with presets in parameters:
#    basf2 PXDClasterShape_PrepareSources.py UseTracks CalibrationKind PixelKind
#    Example for defaults:
#    basf2 PXDClasterShape_PrepareSources.py 0 1 0
#
##############################################################################
from basf2 import *

# show warnings during processing
set_log_level(LogLevel.WARNING)

# Presets:
EdgeClose = 3
UseTracks = False
CalibrationKind = 1
PixelKind = 0
SpecialLayerNo = 1
SpecialLadderNo = 3
SpecialSensorNo = 2

# If exist load from arguments:
argvs = sys.argv
argc = len(argvs)
print("Number of arguments: ", argc - 1)
if argc >= 2:
    if (argvs[1] == 0):
        UseTracks = False
    if (argvs[1] == 1):
        UseTracks = True
    print("first argument UseTracks: ", UseTracks)
if argc >= 3:
    print("second argument CalibrationKind: ", argvs[2])
    CalibrationKind = int(argvs[2])
if argc >= 4:
    print("third argument PixelKind: ", argvs[3])
    PixelKind = int(argvs[3])

# Crosscheck of presets:
if UseTracks == 1:
    CalibrationKind = 1
if PixelKind == 0:
    SpecialLayerNo = 1
    SpecialLadderNo = 3
    SpecialSensorNo = 2
if PixelKind == 1:
    SpecialLayerNo = 1
    SpecialLadderNo = 3
    SpecialSensorNo = 2
if PixelKind == 2:
    SpecialLayerNo = 2
    SpecialLadderNo = 4
    SpecialSensorNo = 2
if PixelKind == 3:
    SpecialLayerNo = 2
    SpecialLadderNo = 4
    SpecialSensorNo = 2

outputFileName = 'pxdClShapeCalibrationSource_Track' + \
    str(UseTracks) + '_Calib' + str(CalibrationKind) + '_Pixel' + str(PixelKind) + '.root'

# Register modules

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
# PXD digitization module
PXDDIGI = register_module('PXDDigitizer')
# PXD clusterizer
PXDCLUST = register_module('PXDClusterizer')
# SVD digitization module
# SVDDIGI = register_module('SVDDigitizer')
# SVD clusterizer
# SVDCLUST = register_module('SVDClusterizer')
# PXD shape calibration module
PXDSHCAL = register_module('pxdClusterShapeCalibration')
# Save output of simulation
output = register_module('RootOutput')

# ============================================================================
# Set a fixed random seed for particle generation:
set_random_seed(1028307)

# ============================================================================
# Setting the list of particle codes (PDG codes) for the generated particles
particlegun.param('pdgCodes', [-11, 11])

# ============================================================================
# Setting the number of tracks to be generated per event:
particlegun.param('nTracks', 1)

if CalibrationKind == 2:
    # ============================================================================
    # Setting common properties for the generated particles
    particlegun.param('nTracks', 1)
    particlegun.param('varyNTracks', False)
    particlegun.param('pdgCodes', [211, -211, 11, -11])
    # particlegun.param('pdgCodes', [-11, 11])
    particlegun.param('momentumGeneration', 'normal')
    particlegun.param('momentumParams', [2, 0.2])
    particlegun.param('phiGeneration', 'uniform')
    particlegun.param('phiParams', [0, 180])
    # particlegun.param('phiParams', [20, 160])
    particlegun.param('thetaGeneration', 'uniformCos')
    # particlegun.param('thetaGeneration', 'uniform')
    particlegun.param('thetaParams', [1, 179])
    # particlegun.param('thetaParams', [110, 170])

    # ============================================================================
    # Setting the number of tracks to be generated per event:
    # For setting of vertex close to surface of for PXD kinds of pixels set:
    #  55 um pitch: sensor 1.3.2 , vertex: x: [-0.2050,-0.2], y: [1.35], z: [0.7,0.7055]
    #  60 um pitch: sensor 1.3.2 , vertex: x: [-0.2050,-0.2], y: [1.35], z: [-1.5,-1.5060]
    #  70 um pitch: sensor 2.4.2 , vertex: x: [-0.2050,-0.2], y: [2.18], z: [0.9,0.9070]
    #  85 um pitch: sensor 2.4.2 , vertex: x: [-0.2050,-0.2], y: [2.18], z: [-2.0,-2.0085]
    if PixelKind == 0:
        # Pixel size v = 55 microns

        # Set parameters for particlegun
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

            # 'nTracks': 1,
            # 'varyNTracks': False,
            # 'pdgCodes': [211, -211, 11, -11],
            # 'momentumGeneration': 'normal',
            # 'momentumParams': [2, 0.2],
            # 'phiGeneration': 'uniform',
            # 'phiParams': [0, 360],
            # # 'thetaGeneration': 'uniformCos',
            # 'thetaGeneration': 'uniform',
            # 'thetaParams': [10, 170],
            'xVertexGeneration': 'uniform',
            'xVertexParams': [-0.2050, -0.2],
            'yVertexGeneration': 'fixed',
            'yVertexParams': [1.35],
            # 'yVertexParams': [2.18],
            'zVertexGeneration': 'uniform',
            'zVertexParams': [0.7, 0.7055],
            # 'zVertexParams': [-1.5060, -1.5],
            # 'zVertexParams': [0.9, 0.9070],
            # 'zVertexParams': [-2.0085, -2.0],
            'independentVertices': False,
        })
    if PixelKind == 1:
        # Pixel size v = 60 microns

        particlegun.param({
            'xVertexGeneration': 'uniform',
            'xVertexParams': [-0.2050, -0.2],
            'yVertexGeneration': 'fixed',
            'yVertexParams': [1.35],
            # 'yVertexParams': [2.18],
            'zVertexGeneration': 'uniform',
            # 'zVertexParams': [0.7, 0.7055],
            'zVertexParams': [-1.5060, -1.5],
            # 'zVertexParams': [0.9, 0.9070],
            # 'zVertexParams': [-2.0085, -2.0],
            'independentVertices': False,
        })
    if PixelKind == 2:
        # Pixel size v = 70 microns

        particlegun.param({
            'xVertexGeneration': 'uniform',
            'xVertexParams': [-0.2050, -0.2],
            'yVertexGeneration': 'fixed',
            # 'yVertexParams': [1.35],
            'yVertexParams': [2.18],
            'zVertexGeneration': 'uniform',
            # 'zVertexParams': [0.7, 0.7055],
            # 'zVertexParams': [-1.5060, -1.5],
            'zVertexParams': [0.9, 0.9070],
            # 'zVertexParams': [-2.0085, -2.0],
            'independentVertices': False,
        })
    if PixelKind == 3:
        # Pixel size v = 85 microns

        particlegun.param({
            'xVertexGeneration': 'uniform',
            'xVertexParams': [-0.2050, -0.2],
            'yVertexGeneration': 'fixed',
            # 'yVertexParams': [1.35],
            'yVertexParams': [2.18],
            'zVertexGeneration': 'uniform',
            # 'zVertexParams': [0.7, 0.7055],
            # 'zVertexParams': [-1.5060, -1.5],
            # 'zVertexParams': [0.9, 0.9070],
            'zVertexParams': [-2.0085, -2.0],
            'independentVertices': False,
        })

# ============================================================================
# Print the parameters of the particle gun
print_params(particlegun)

# Set the number of events to be processed (10 events)
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})

# Set output filename
output.param('outputFileName', outputFileName)

# Select subdetectors to be built
# geometry.param('Components', ['PXD','SVD'])
geometry.param('components', ['MagneticField', 'PXD', 'SVD'])

# PXDDIGI.param('SimpleDriftModel', False)
# PXDDIGI.param('statisticsFilename', 'PXDDiags.root')
# PXDDIGI.param('PoissonSmearing', True)
# PXDDIGI.param('ElectronicEffects', False)
# PXDDIGI.param('NoiseSN', 1.0)
# PXDCLUST.param('NoiseSN', 1.0)

PXDCLUST.param('useClusterShape', True)

# Set the region close edge where remove cluster shape corrections, default=3
PXDSHCAL.param('EdgeClose', EdgeClose)

# To use track information (default) or simulations, default=True
PXDSHCAL.param('UseTracks', UseTracks)

# 1: standard calibration based on realistic physics or real data (default)
# 2: special, for full range of angles for every kind of pixel
PXDSHCAL.param('CalibrationKind', CalibrationKind)

# For CalibrationKind=2 set pixel kind (pixel size) in range 1..4, default=1
PXDSHCAL.param('PixelKind', PixelKind)

# For CalibrationKind=2 set sensor ID for special analysis, default=1.3.2
PXDSHCAL.param('SpecialLayerNo', SpecialLayerNo)
PXDSHCAL.param('SpecialLadderNo', SpecialLadderNo)
PXDSHCAL.param('SpecialSensorNo', SpecialSensorNo)


PXDSHCAL.set_log_level(LogLevel.INFO)

# ============================================================================
# Do the simulation

main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(PXDDIGI)
main.add_module(PXDCLUST)
main.add_module(PXDSHCAL)

main.add_module("PrintCollections")
main.add_module(output)
# Process events
process(main)

# Print call statistics
print(statistics)
#
