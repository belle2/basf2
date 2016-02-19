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
# "UseRealData" to use real data without simulations or simulations (default)
#
# Parameters are not independent, there are some priorities:
#   Priorities: UseRealData, UseTracks, rest is independent
#   if you set UseRealData=True, UseTracks is set automatically to True (noe TrueHit data).
#
# There is possibility to call this macro with presets in parameters:
#    basf2 PXDClasterShape_ApplyCorrections.py UseTracks UseRealData
#    Example for defaults:
#    basf2 PXDClasterShape_ApplyCorrections.py 1 0 1 0
#
##############################################################################
from basf2 import *

# show warnings during processing
set_log_level(LogLevel.WARNING)

# Presets (defaults, no need to set if no change):
UseTracks = True
UseRealData = False

# If exist load from arguments:
argvs = sys.argv
argc = len(argvs)
print("Number of arguments: ", argc - 1)
if argc >= 2:
    if (argvs[1] == '0'):
        UseTracks = False
    if (argvs[1] == '1'):
        UseTracks = True
    print("first argument UseTracks: ", UseTracks)
if argc >= 3:
    if (argvs[2] == '0'):
        UseRealData = False
    if (argvs[2] == '1'):
        UseRealData = True
    print("second argument UseRealData: ", UseRealData)

# Crosscheck of presets:
if (UseRealData is True):
    UseTracks = True

print("Final setting of arguments: ")
print("                 UseTracks: ", UseTracks)
print("               UseRealData: ", UseRealData)

# outputFileName = 'pxdClShapeCalibrationSource_RealData' + str(UseRealData) + '_Track' + str(UseTracks)
# outputFileName = outputFileName + '_Calib' + str(CalibrationKind) + '_Pixel' + str(PixelKindCal) + '.root'

CalibrationKind = 1
PixelKindCal = 0

outputFileName = 'pxdClShCalApply_RealData' + str(UseRealData) + '_Track' + str(UseTracks)
outputFileName = outputFileName + '_Calib' + str(CalibrationKind) + '_Pixel' + str(PixelKindCal) + '.root'

inputCalFileName = 'pxdClShCalSrc_RealData' + str(UseRealData) + '_Track' + str(UseTracks)
inputCalFileName = inputCalFileName + '_Calib' + str(CalibrationKind) + '_Pixel' + str(PixelKindCal) + '.root'
inputCalFileName = "pxdClShCal_RealData0_Track1_Calib1_Pixel0.root"

inputCalFileNamePK0 = 'pxdClShCalSrc_RealData' + str(UseRealData) + '_Track' + str(UseTracks)
inputCalFileNamePK0 = inputCalFileNamePK0 + '_Calib' + str(CalibrationKind) + '_Pixel' + str(PixelKindCal) + '.root'
inputCalFileNamePK1 = 'pxdClShCalSrc_RealData' + str(UseRealData) + '_Track' + str(UseTracks)
inputCalFileNamePK1 = inputCalFileNamePK1 + '_Calib' + str(CalibrationKind) + '_Pixel' + str(PixelKindCal) + '.root'
inputCalFileNamePK2 = 'pxdClShCalSrc_RealData' + str(UseRealData) + '_Track' + str(UseTracks)
inputCalFileNamePK2 = inputCalFileNamePK2 + '_Calib' + str(CalibrationKind) + '_Pixel' + str(PixelKindCal) + '.root'
inputCalFileNamePK3 = 'pxdClShCalSrc_RealData' + str(UseRealData) + '_Track' + str(UseTracks)
inputCalFileNamePK3 = inputCalFileNamePK3 + '_Calib' + str(CalibrationKind) + '_Pixel' + str(PixelKindCal) + '.root'

inputCalFileNamePK0 = "pxdClShCal_RealData0_Track1_Calib1_Pixel0.root"
inputCalFileNamePK1 = "pxdClShCal_RealData0_Track1_Calib1_Pixel0.root"
inputCalFileNamePK2 = "pxdClShCal_RealData0_Track1_Calib1_Pixel0.root"
inputCalFileNamePK3 = "pxdClShCal_RealData0_Track1_Calib1_Pixel0.root"

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
SVDDIGI = register_module('SVDDigitizer')
# SVD clusterizer
SVDCLUST = register_module('SVDClusterizer')
# PXD shape calibration module
PXDApplyClShCorrection = register_module('pxdApplyClusterShapeCorrection')
# Save output of simulation
output = register_module('RootOutput')
# output.param('branchNames', ['EventMetaData'])  # cannot be removed, but of only small effect on file size

# ============================================================================
# Set a fixed random seed for particle generation:
set_random_seed(1028307)

# ============================================================================
# Setting the list of particle codes (PDG codes) for the generated particles
particlegun.param('pdgCodes', [-11, 11])

# ============================================================================
# Setting the number of tracks to be generated per event:
particlegun.param('nTracks', 1)

# ============================================================================
# Print the parameters of the particle gun
print_params(particlegun)

# Set the number of events to be processed (10 events)
eventinfosetter.param({'evtNumList': [200], 'runList': [1]})

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

# To use track information (default) or simulations, default=True
PXDApplyClShCorrection.param('UseTracks', UseTracks)

# To use real data without simulations or simulations, default=False
PXDApplyClShCorrection.param('UseRealData', UseRealData)

# Name of file contain basic calibration, default="pxdCalibrationBasic"
PXDApplyClShCorrection.param('CalFileBasic', inputCalFileName)
# Name of file contain calibration for pixel kind 0 (PitchV=55um), default="pxdCalibrationPixelKind0"
PXDApplyClShCorrection.param('CalFilePK0', inputCalFileNamePK0)
# Name of file contain calibration for pixel kind 1 (PitchV=60um), default="pxdCalibrationPixelKind1"
PXDApplyClShCorrection.param('CalFilePK1', inputCalFileNamePK1)
# Name of file contain calibration for pixel kind 2 (PitchV=70um), default="pxdCalibrationPixelKind2"
PXDApplyClShCorrection.param('CalFilePK2', inputCalFileNamePK2)
# Name of file contain calibration for pixel kind 3 (PitchV=85um), default="pxdCalibrationPixelKind3"
PXDApplyClShCorrection.param('CalFilePK3', inputCalFileNamePK3)

# Do expert histograms"
PXDApplyClShCorrection.param('DoExpertHistograms', True)

PXDApplyClShCorrection.set_log_level(LogLevel.INFO)
PXDApplyClShCorrection.set_log_level(LogLevel.DEBUG)
PXDApplyClShCorrection.set_debug_level(100)

secSetup = \
    ['shiftedL3IssueTestVXDStd-moreThan400MeV_PXDSVD',
     'shiftedL3IssueTestVXDStd-100to400MeV_PXDSVD',
     'shiftedL3IssueTestVXDStd-25to100MeV_PXDSVD'
     ]
tuneValue = 0.22

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)

vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.DEBUG
vxdtf.logging.debug_level = 1
param_vxdtf = {'sectorSetup': secSetup,
               'GFTrackCandidatesColName': 'caTracks',
               'tuneCutoffs': tuneValue,
               'displayCollector': 2
               # 'calcQIType': 'kalman'
               }

vxdtf.param(param_vxdtf)

doPXD = 1
TrackFinderMCTruth = register_module('TrackFinderMCTruth')
TrackFinderMCTruth.logging.log_level = LogLevel.INFO
# select which detectors you would like to use
param_TrackFinderMCTruth = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': doPXD,
    'MinimalNDF': 6,
    'WhichParticles': ['primary'],
    'GFTrackCandidatesColName': 'mcTracks',
}
TrackFinderMCTruth.param(param_TrackFinderMCTruth)

setupGenfit = register_module('SetupGenfitExtrapolation')
GenFitter = register_module('GenFitter')
# GenFitter.logging.log_level = LogLevel.WARNING
GenFitter.param('GFTrackCandidatesColName', 'caTracks')
GenFitter.param('FilterId', 'Kalman')

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)


# ============================================================================
# Do the simulation

main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(setupGenfit)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(PXDDIGI)
main.add_module(PXDCLUST)
if (UseTracks is True):
    main.add_module(SVDDIGI)
    main.add_module(SVDCLUST)
    main.add_module(eventCounter)
    main.add_module(vxdtf)
    main.add_module(TrackFinderMCTruth)
    main.add_module(GenFitter)
main.add_module(PXDApplyClShCorrection)

main.add_module("PrintCollections")
main.add_module(output)
# Process events
process(main)

# Print call statistics
print(statistics)
#
