#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will
import os
from sys import argv
from basf2 import *
from time import time

numEvents = 100
initialValue = 1

# secSetup = ['evtNormSecHIGH_SVD', 'evtNormSecMED_SVD', 'evtNormSecLOW_SVD']
secSetup = ['evtNormSecHIGH_VXD', 'evtNormSecMED_VXD', 'evtNormSecLOW_VXD']
# WARNING if you want to use SVD only, please uncomment secSetup ending with SVD, then comment the VXD-version - and don't forget to set the clusters for the detector type you want in the mcTrackFinder down below!

print 'running {events:} events, Seed {theSeed:} - evtGen No BG'.format(events=numEvents,
        theSeed=initialValue)

set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('expList', [0])
evtmetagen.param('runList', [1])
evtmetagen.param('evtNumList', [numEvents])

evtmetainfo = register_module('EvtMetaInfo')

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
    'SVD-Support',
    ])

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)

vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.DEBUG
vxdtf.logging.debug_level = 1
param_vxdtf = {'sectorSetup': secSetup}
vxdtf.param(param_vxdtf)

mctrackfinder = register_module('MCTrackFinder')
mctrackfinder.logging.log_level = LogLevel.INFO
# select which detectors you would like to use
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'MinimalNDF': 6,
    'WhichParticles': ['primary'],
    'GFTrackCandidatesColName': 'mcTracks',
    }
mctrackfinder.param(param_mctrackfinder)

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
# Process events
process(main)

print 'Event Statistics :'
print statistics

# print 'Event Statistics detailed:'
# print statistics(statistics.TOTAL)

# print 'Event Statistics for vxdtf:'
# print statistics([vxdtf])

# print 'Memory statistics'
# for stats in statistics.modules:
    # print 'Module %s:' % stats.name
    # print ' -> initialize(): %10d KB' % stats.memory(statistics.INIT)
    # print ' -> beginRun():   %10d KB' % stats.memory(statistics.BEGIN_RUN)
    # print ' -> event():      %10d KB' % stats.memory()
    # print ' -> endRun():     %10d KB' % stats.memory(statistics.END_RUN)
    # print ' -> terminate():  %10d KB' % stats.memory(statistics.TERM)
