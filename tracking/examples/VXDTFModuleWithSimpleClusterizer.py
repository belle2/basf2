#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file will
import os
from sys import argv
from basf2 import *
from beamparameters import add_beamparameters
from time import time


usePXD = True

numEvents = 250
initialValue = 2


tuneValue = 0.06

secSetup = [
    'shiftedL3IssueTestSVDStd-moreThan400MeV_SVD',
    'shiftedL3IssueTestSVDStd-100to400MeV_SVD',
    'shiftedL3IssueTestSVDStd-25to100MeV_SVD']

if usePXD:
    secSetup = \
        ['shiftedL3IssueTestVXDStd-moreThan400MeV_PXDSVD',
         'shiftedL3IssueTestVXDStd-100to400MeV_PXDSVD',
         'shiftedL3IssueTestVXDStd-25to100MeV_PXDSVD'
         ]
    tuneValue = 0.22

# WARNING if you want to use SVD only, please uncomment secSetup ending
# with SVD, then comment the VXD-version (of vice versa if you want to
# have the svd version)

print('running {events:} events, Seed {theSeed:} - evtGen No BG'.format(events=numEvents,
                                                                        theSeed=initialValue))

set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

gearbox = register_module('Gearbox')

vxdClusterizer = register_module('VXDSimpleClusterizer')
vxdClusterizer.logging.log_level = LogLevel.DEBUG
vxdClusterizer.logging.debug_level = 10
vxdClusterizer.param('onlyPrimaries', True)

evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING

geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD',
                              'PXD', 'SVD'])

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)

vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.DEBUG
vxdtf.logging.debug_level = 1
param_vxdtf = {'sectorSetup': secSetup,
               'GFTrackCandidatesColName': 'caTracks',
               'tuneCutoffs': tuneValue,
               # 'calcQIType': 'kalman'
               }

vxdtf.param(param_vxdtf)


doPXD = 0
if usePXD:
    doPXD = 1
track_finder_mc_truth = register_module('TrackFinderMCTruth')
track_finder_mc_truth.logging.log_level = LogLevel.INFO
# select which detectors you would like to use
param_track_finder_mc_truth = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': doPXD,
    'MinimalNDF': 6,
    'WhichParticles': ['primary'],
    'GFTrackCandidatesColName': 'mcTracks',
}
track_finder_mc_truth.param(param_track_finder_mc_truth)

setupGenfit = register_module('SetupGenfitExtrapolation')

trackfitter = register_module('GenFitter')
# trackfitter.logging.log_level = LogLevel.WARNING
trackfitter.param('GFTrackCandidatesColName', 'caTracks')
trackfitter.param('FilterId', 'Kalman')

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)

analyzer = register_module('TFAnalizer')
analyzer.logging.log_level = LogLevel.INFO
analyzer.logging.debug_level = 11
param_analyzer = {'printExtentialAnalysisData': False, 'caTCname': 'caTracks'}
analyzer.param(param_analyzer)

# Create paths
main = create_path()
# Add modules to paths


# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters = add_beamparameters(main, "Y1S")
# beamparameters.param("generateCMS", True)
# beamparameters.param("smearVertex", False)
# beamparameters.param("smearEnergy", False)
# print_params(beamparameters)


main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(evtgeninput)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(setupGenfit)
main.add_module(g4sim)
main.add_module(vxdClusterizer)
main.add_module(eventCounter)
main.add_module(vxdtf)
main.add_module(track_finder_mc_truth)
main.add_module(analyzer)
# main.add_module(trackfitter)

# Process events
process(main)

print('Event Statistics :')
print(statistics)
