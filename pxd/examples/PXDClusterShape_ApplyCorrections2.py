#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##############################################################################
#
# This is an example steering file to use PXD shape correction.
# Shape correction is realize in three steps, se exampes:
#   pxd/examples/PXDClasterShape_PrepareSources.py
#   pxd/examples/PXDClasterShape_CalculateCorrections.py
#   pxd/examples/PXDClasterShape_ApplyCorrections.py
#   pxd/examples/PXDClasterShape_ApplyCorrections2.py
# It uses ParicleGun module to generate tracks,
# (see "generators/example/ParticleGunFull.py" for detailed usage)
# builds PXD geometry, performs geant4 and PXD simulation,
# and stores output in a root file.
#
# "PrefereSimulation" To use simulations rather than real data calculated corrections, default=False
#
# There is possibility to call this macro with presets in parameters:
#    basf2 PXDClasterShape_ApplyCorrections.py PrefereSimulation
#    Example for defaults:
#    basf2 pxd/examples/PXDClasterShape_ApplyCorrections.py 0
#    basf2 pxd/examples/PXDClasterShape_ApplyCorrections2.py 0
#
##############################################################################
from basf2 import *

# show warnings during processing
set_log_level(LogLevel.WARNING)

# Presets (defaults, no need to set if no change):
PrefereSimulation = False

# If exist load from arguments:
argvs = sys.argv
argc = len(argvs)
print("Number of arguments: ", argc - 1)
if argc >= 2:
    if (argvs[1] == '0'):
        PrefereSimulation = False
    if (argvs[1] == '1'):
        PrefereSimulation = True
    print("first argument PrefereSimulation: ", PrefereSimulation)

print("Setting of arguments: ")
print("         PrefereSimulation: ", PrefereSimulation)

inputFileName = 'pxdClShCalApply_Step1.root'
outputFileName = 'pxdClShCalApply_Step2.root'

# Name of file contain basic calibration, default="pxdCalibrationBasic.root" - creating from excluded residuals
inputCalFileName = "pxd/data/pxdClShCal_RealData0_Track1_Calib1_Pixel0.root"
# Name of file contain basic calibration, default="pxdCalibrationBasic.root" - creating from included residuals - not good!
# inputCalFileName = "pxd/data/pxdClShCal_RealData0_Track0_Calib1_Pixel0.root"

# Name of files contain calibration for pixel kind 0..3 (PitchV=55um, 60um, 70um, 85um), default="pxdCalibrationPixelKind0..3.root"
inputCalFileNamePK0 = "pxd/data/pxdClShCal_RealData0_Track0_Calib2_Pixel0.root"
inputCalFileNamePK1 = "pxd/data/pxdClShCal_RealData0_Track0_Calib2_Pixel1.root"
inputCalFileNamePK2 = "pxd/data/pxdClShCal_RealData0_Track0_Calib2_Pixel2.root"
inputCalFileNamePK3 = "pxd/data/pxdClShCal_RealData0_Track0_Calib2_Pixel3.root"

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
# PXD shape calibration check module
PXDCheckClShCorrection = register_module('pxdCheckClusterShapeCorrection')
# PXD shape calibration apply module
PXDApplyClShCorrection = register_module('pxdApplyClusterShapeCorrection')

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
eventinfosetter.param({'evtNumList': [25000], 'runList': [1]})

# Save output of simulation step 1
output = register_module('RootOutput')
# Set output filename
output.param('outputFileName', outputFileName)

input2 = register_module('RootInput')
input2.param('inputFileName', inputFileName)
input2.param('excludeBranchNames', [
    'VXDTFInfoBoards',
    'GF2Tracks',
    'mcTracks',
    'caTracks'
])

# Select subdetectors to be built
# geometry.param('Components', ['PXD','SVD'])
geometry.param('components', ['MagneticField', 'PXD', 'SVD'])

# To use simulations rather than real data calculated corrections, default=False
PXDApplyClShCorrection.param('PrefereSimulation', PrefereSimulation)

# Name of file contain basic calibration, default="pxdCalibrationBasic.root"
PXDApplyClShCorrection.param('CalFileBasic', inputCalFileName)
# Name of file contain calibration for pixel kind 0 (PitchV=55um), default="pxdCalibrationPixelKind0.root"
PXDApplyClShCorrection.param('CalFilePK0', inputCalFileNamePK0)
# Name of file contain calibration for pixel kind 1 (PitchV=60um), default="pxdCalibrationPixelKind1.root"
PXDApplyClShCorrection.param('CalFilePK1', inputCalFileNamePK1)
# Name of file contain calibration for pixel kind 2 (PitchV=70um), default="pxdCalibrationPixelKind2.root"
PXDApplyClShCorrection.param('CalFilePK2', inputCalFileNamePK2)
# Name of file contain calibration for pixel kind 3 (PitchV=85um), default="pxdCalibrationPixelKind3.root"
PXDApplyClShCorrection.param('CalFilePK3', inputCalFileNamePK3)

# Mark of loop to save monitored data to different file, default=0
PXDApplyClShCorrection.param('MarkOfLoopForHistogramsFile', 1)

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
# Use from TB packet:
# TrackFinderMCTruthRecoTrack = register_module('TrackFinderMCVXDTB')
# Should be use:
TrackFinderMCTruthRecoTrack = register_module('TrackFinderMCTruthRecoTracks')

TrackFinderMCTruthRecoTrack.logging.log_level = LogLevel.INFO

# TrackFinderMCTruth = register_module('TrackFinderMCTruth')
# TrackFinderMCTruth.logging.log_level = LogLevel.INFO
# select which detectors you would like to use
# param_TrackFinderMCTruth = {
#     'UseCDCHits': 0,
#     'UseSVDHits': 1,
#     'UsePXDHits': doPXD,
#     'MinimalNDF': 6,
#     'WhichParticles': ['primary'],
#     'GFTrackCandidatesColName': 'mcTracks',
# }
# 'GFTrackCandidatesColName': 'mcTracks2',
# TrackFinderMCTruth.param(param_TrackFinderMCTruth)

# Use from TB packet:
GenfitterVXDTB = register_module('GenFitterVXDTB')
GenfitterVXDTB.param('GFTrackCandidatesColName', 'caTracks')
GenfitterVXDTB.param('FilterId', 'Kalman')

setupGenfit = register_module('SetupGenfitExtrapolation')

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)


# ============================================================================
# Do the simulation step 2 - use corrected clusters

main = create_path()
main.add_module(eventinfosetter)
# main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(input2)
main.add_module(eventCounter)
# main2.add_module('SetupGenfitExtrapolation')
main.add_module(setupGenfit)
main.add_module(vxdtf)
main.add_module(TrackFinderMCTruthRecoTrack)
# main.add_module(TrackFinderMCTruth)
main.add_module(GenfitterVXDTB)

main.add_module(PXDApplyClShCorrection)

main.add_module("PrintCollections")
main.add_module(output)
# Process events
process(main)

# Print call statistics
print(statistics)
#
