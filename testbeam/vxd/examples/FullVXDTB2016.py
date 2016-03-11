#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Common PXD&SVD TestBeam Jan 2014 @ DESY Simulation
# This is the default simulation scenario for VXD beam test WITHOUT telescopes

# Important parameters of the simulation:
events = 10000  # Number of events to simulate
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


# Use truth information to create track candidates
mctrackfinder = register_module('TrackFinderMCVXDTB')
mctrackfinder.logging.log_level = LogLevel.WARNING
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'UseTelHits': 1,
    'Smearing': 0,
    'UseClusters': True,
    'WhichParticles': ['SVD'],
}
mctrackfinder.param(param_mctrackfinder)

filterOverlaps = 'hopfield'

if fieldOn:
    # SVD and PXD sec map
    # secSetup = ['TB2016Test8Feb2016MagnetOnPXDSVD-moreThan1500MeV_PXDSVD']
    # only SVD:
    secSetup = ['TB2016Test8Feb2016MagnetOnSVD-moreThan1500MeV_SVD']
    qiType = 'circleFit'  # circleFit
else:
    # To turn off magnetic field:
    # SVD and PXD sec map:
    # secSetup = ['TB2016Test8Feb2016MagnetOffPXDSVD-moreThan1500MeV_PXDSVD']
    # only SVD
    secSetup = ['TB2016Test8Feb2016MagnetOffSVD-moreThan1500MeV_SVD']
    qiType = 'straightLine'  # straightLine

vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.DEBUG
vxdtf.logging.debug_level = 2
# calcQIType:
# Supports 'kalman', 'circleFit' or 'trackLength.
# 'circleFit' has best performance at the moment

# filterOverlappingTCs:
# Supports 'hopfield', 'greedy' or 'none'.
# 'hopfield' has best performance at the moment
param_vxdtf = {  # normally we don't know the particleID, but in the case of the testbeam,
                 # we can expect (anti-?)electrons...
                 # True
                 # 'artificialMomentum': 5., ## uncomment if there is no magnetic field!
                 # 7
                 # 'activateDistance3D': [False],
                 # 'activateDistanceZ': [True],
                 # 'activateAngles3D': [False],
                 # 'activateAnglesXY': [True],  #### noMagnet
                 # ### withMagnet
                 # 'activateAnglesRZHioC': [True], #### noMagnet
                 # ### withMagnet r51x
                 # True
    'activateBaselineTF': 1,
    'debugMode': 0,
    'tccMinState': [2],
    'tccMinLayer': [3],
    'reserveHitsThreshold': [0.],
    'highestAllowedLayer': [6],
    'standardPdgCode': -11,
    'artificialMomentum': 3,
    'sectorSetup': secSetup,
    'calcQIType': qiType,
    'killEventForHighOccupancyThreshold': 500,
    'highOccupancyThreshold': 111,
    'cleanOverlappingSet': False,
    'filterOverlappingTCs': filterOverlaps,
    'TESTERexpandedTestingRoutines': True,
    'qiSmear': False,
    'smearSigma': 0.000001,
    'GFTrackCandidatesColName': 'TrackCands',
    'tuneCutoffs': 0.51,
    'activateDistanceXY': [False],
    'activateDistance3D': [True],
    'activateDistanceZ': [False],
    'activateSlopeRZ': [False],
    'activateNormedDistance3D': [False],
    'activateAngles3D': [True],
    'activateAnglesXY': [False],
    'activateAnglesRZ': [False],
    'activateDeltaSlopeRZ': [False],
    'activateDistance2IP': [False],
    'activatePT': [False],
    'activateHelixParameterFit': [False],
    'activateAngles3DHioC': [True],
    'activateAnglesXYHioC': [True],
    'activateAnglesRZHioC': [False],
    'activateDeltaSlopeRZHioC': [False],
    'activateDistance2IPHioC': [False],
    'activatePTHioC': [False],
    'activateHelixParameterFitHioC': [False],
    'activateDeltaPtHioC': [False],
    'activateDeltaDistance2IPHioC': [False],
    'activateZigZagXY': [False],
    'activateZigZagRZ': [False],
    'activateDeltaPt': [False],
    'activateCircleFit': [False],
}
vxdtf.param(param_vxdtf)

# mctrackfinder.logging.log_level = LogLevel.DEBUG


# Path construction
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module('PXDDigitizer')
main.add_module('SVDDigitizer')
main.add_module('PXDClusterizer')
main.add_module('SVDClusterizer')
main.add_module('TelDigitizer')
main.add_module('TelClusterizer')
main.add_module('SetupGenfitExtrapolation')
main.add_module(vxdtf)
# main.add_module(mctrackfinder)
main.add_module('GenFitterVXDTB')
main.add_module('ExportGeometry', Filename='TBGeometry.root')
main.add_module('RootOutput', outputFileName='TBSimulation.root')
main.add_module('OverlapChecker')
main.add_module('Display', fullGeometry=True)

# Fieldmap creator
import math
fieldmap = register_module('CreateFieldMap')
fieldmap.param({
    # Filename for the histograms
    'filename': 'FieldMap.root',
    # type of the scan: along xy, zx or zy
    "type": "zx",
    # number of steps along the first coordinate, in the case of a zx scan this
    # would be z
    "nU": 1600,
    # start of the first coordinate in cm
    "minU": -400,
    # end of the first coordinate in cm
    "maxU": 500,
    # number of steps along the second coordinate, in the case of a zx scan
    # this would be x
    "nV": 1600,
    # start of the second coordinate in cm
    "minV": -400,
    # end of the second coordinate in cm
    "maxV": 400,
    # rotation the plane for the scan around the global z axis: a value of
    # pi/2 will convert a zx in a zy scan
    "phi": math.pi / 4,
    # offset of the plane with respect to the global origin
    "wOffset": 0,
})

main.add_module(fieldmap)


# Process events
process(main)

# Print call statistics
print(statistics)
