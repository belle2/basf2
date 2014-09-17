#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will
import os
from sys import argv
from basf2 import *
from time import time

numEvents = 200
initialValue = 1

usePXD = True

tuneValue = 0.06
secSetup = ['secMapEvtGenOnR10933June2014SVDStd-moreThan500MeV_SVD',
            'secMapEvtGenOnR10933June2014SVDStd-125to500MeV_SVD',
            'secMapEvtGenOnR10933June2014SVDStd-30to125MeV_SVD']

if usePXD:
    secSetup = ['secMapEvtGenOnR10933June2014VXDStd-moreThan500MeV_PXDSVD',
                'secMapEvtGenOnR10933June2014VXDStd-125to500MeV_PXDSVD',
                'secMapEvtGenOnR10933June2014VXDStd-30to125MeV_PXDSVD']
    tuneValue = 0.22

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
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD',
               'PXD', 'SVD'])

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)

vxdtf = register_module('TFRedesign')
vxdtf.logging.log_level = LogLevel.DEBUG
vxdtf.logging.debug_level = 1
param_vxdtf = {'sectorSetup': secSetup,
               'GFTrackCandidatesColName': 'caTracks',
               'tuneCutoffs': tuneValue}
# , 'calcQIType': 'kalman'
vxdtf.param(param_vxdtf)

# VXDTF DQM module
# vxdtf_dqm = register_module('VXDTFDQM')
# vxdtf_dqm.param('GFTrackCandidatesColName', 'caTracks')

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

trackfitter = register_module('GenFitter')
# trackfitter.logging.log_level = LogLevel.WARNING
trackfitter.param('GFTrackCandidatesColName', 'caTracks')
trackfitter.param('FilterId', 'Kalman')
trackfitter.param('UseClusters', True)

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
# histo = register_module('HistoManager')
# histo.param('histoFileName', 'DQM-VXDTFdemo.root')  # File to save histograms
# main.add_module(histo)
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
main.add_module(trackfitter)
# main.add_module(vxdtf_dqm)
# Process events
process(main)

print 'Event Statistics :'
print statistics
