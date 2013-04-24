#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will
import os
from sys import argv
from basf2 import *
from time import time

numEvents = 100
initialValue = 1
secSetup = ['evtgenHIGH', 'evtgenSTD', 'evtgenLOW']  # ,'evtgenHIGH' 'evtgenSTD', 'evtgenLOW'] # when using the same secSetup several times, use this for setting Value

rSeed = initialValue

print 'running {events:} events, Seed {theSeed:} - evtGen No BG'.format(events=numEvents,
        theSeed=initialValue)

set_log_level(LogLevel.ERROR)
set_random_seed(rSeed)

evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [numEvents])

evtmetainfo = register_module('EvtMetaInfo')

gearbox = register_module('Gearbox')

pxdDigitizer = register_module('PXDDigitizer')
svdDigitizer = register_module('SVDDigitizer')
pxdClusterizer = register_module('PXDClusterizer')
svdClusterizer = register_module('SVDClusterizer')
evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING

geometry = register_module('Geometry')
geometry.param('Components', ['MagneticField', 'PXD', 'SVD'])

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)

vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.INFO
vxdtf.logging.debug_level = 11
# detectorType: Supports 'SVD' and 'VXD' so far
# calcQIType:  Supports 'kalman', 'circleFit' or 'trackLength'
param_vxdtf = {
    'tccMinState': [2],
    'tccMinLayer': [3],
    'detectorType': ['VXD'],
    'sectorSetup': secSetup,
    'calcQIType': 'circleFit',
    'cleanOverlappingSet': True,
    'useHopfield': True,
    'TESTERexpandedTestingRoutines': True,
    }
vxdtf.param(param_vxdtf)

mctrackfinder = register_module('MCTrackFinder')
mctrackfinder.logging.log_level = LogLevel.INFO
mctrackfinder.logging.debug_level = 101

# select which detectors you would like to use
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'MinimalNDF': 6,
    'UseClusters': 1,
    'WhichParticles': ['PXD', 'SVD'],
    'GFTrackCandidatesColName': 'mcTracks',
    }
mctrackfinder.param(param_mctrackfinder)

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)

analyzer = register_module('TFAnalizer')
analyzer.logging.log_level = LogLevel.INFO
analyzer.logging.debug_level = 11
param_analyzer = {'printExtentialAnalysisData': False}  # set true if PRINTINFO is wanted
analyzer.param(param_analyzer)

display = register_module('Display')
param_Display = {'ShowGFTrackCands': 1, 'UseClusters': 1}
display.param(param_Display)

# Create paths
main = create_path()

# Add modules to paths

# main.add_module(inputM)
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(evtgeninput)
main.add_module(g4sim)
main.add_module(eventCounter)
main.add_module(pxdDigitizer)
main.add_module(pxdClusterizer)
main.add_module(svdDigitizer)
main.add_module(svdClusterizer)
main.add_module(vxdtf)
main.add_module(mctrackfinder)
main.add_module(analyzer)
# main.add_module(display)
# Process events
process(main)

print statistics
