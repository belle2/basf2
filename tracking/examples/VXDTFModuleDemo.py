#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will
import os
from sys import argv
from basf2 import *
from time import time

numEvents = 250
initialValue = 2

# secSetup = ['sectorList_evtNormSecHIGH_SVD', 'sectorList_evtNormSecMED_SVD',
#            'sectorList_evtNormSecLOW_SVD']

secSetup = ['evtGenStdSVD-moreThan350MeV_SVD', 'evtGenStdSVD-100to350MeV_SVD',
            'evtGenStdSVD-30to100MeV_SVD']

# secSetup = ['sectorList_evtNormSecHIGH_VXD', 'sectorList_evtNormSecMED_VXD', 'sectorList_evtNormSecLOW_VXD']
# WARNING if you want to use SVD only, please uncomment secSetup ending with SVD, then comment the VXD-version - and don't forget to set the clusters for the detector type you want in the TrackFinderMCTruth down below!

print 'running {events:} events, Seed {theSeed:} - evtGen No BG'.format(events=numEvents,
        theSeed=initialValue)

set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

gearbox = register_module('Gearbox')

pxdDigitizer = register_module('PXDDigitizer')
svdDigitizer = register_module('SVDDigitizer')
pxdClusterizer = register_module('PXDClusterizer')
svdClusterizer = register_module('SVDClusterizer')
evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING

geometry = register_module('Geometry')
##geometry.param('Components', ['MagneticField', 'PXD', 'SVD'])
geometry.param('Components', [
    'BeamPipe',
    'Cryostat',
    'HeavyMetalShield',
    'MagneticField',
    'PXD',
    'SVD',
    ])

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)

vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.DEBUG
vxdtf.logging.debug_level = 1
param_vxdtf = {'sectorSetup': secSetup}
# , 'calcQIType': 'kalman'
vxdtf.param(param_vxdtf)

track_finder_mc_truth = register_module('TrackFinderMCTruth')
track_finder_mc_truth.logging.log_level = LogLevel.INFO
# select which detectors you would like to use
param_track_finder_mc_truth = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 0,
    'MinimalNDF': 6,
    'WhichParticles': ['primary'],
    'GFTrackCandidatesColName': 'mcTracks',
    }
track_finder_mc_truth.param(param_track_finder_mc_truth)

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)

analyzer = register_module('TFAnalizer')
analyzer.logging.log_level = LogLevel.INFO
analyzer.logging.debug_level = 11
param_analyzer = {'printExtentialAnalysisData': False}
analyzer.param(param_analyzer)

# Create paths
main = create_path()

# Add modules to paths

# main.add_module(inputM)
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(evtgeninput)
main.add_module(g4sim)
main.add_module(pxdDigitizer)
main.add_module(pxdClusterizer)
main.add_module(svdDigitizer)
main.add_module(svdClusterizer)
main.add_module(eventCounter)
main.add_module(vxdtf)
main.add_module(track_finder_mc_truth)
main.add_module(analyzer)
# Process events
process(main)

print 'Event Statistics :'
print statistics
