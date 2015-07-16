#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
from subprocess import call
from sys import argv

#################

generateSecMap = True  # <----------- here you can switch between generating sectorMaps and testing with the VXDTF!
useSimpleClusterizer = True
useEvtGen = True
# useEDeposit: If you want to work w/o E-deposit, edit pxd/data/PXD.xml and svd/data/SVD.xml,
# where you have to activate see neutrons = true:
useEDeposit = True
#################
# Important parameters:

numTracks = 10
numEvents = 1
initialValue = 1  # want random events, if set to 0

# parameters for the secMap-calculation:
pTcuts = [0.025, 0.1, 0.4]


setupFileName = 'evtGenPGunR19630Jul2015Old'
setupFileName2 = 'evtGenPGunR19630Jul2015New'


setupFileNamesvdStd = setupFileName + 'SVD' + 'Std'
setupFileNamevxdStd = setupFileName + 'VXD' + 'Std'

setupFileName2s = setupFileName2 + 'SVD' + 'Std'
setupFileName2 = setupFileName2 + 'VXD' + 'Std'

doNew = 1
if len(argv) is 1:
    print 'no arguments given, using standard values'
elif len(argv) is 2:
    doNew = int(argv[1])
    print '1 argument given, if passedValue is 1: only new secMap, 0: only old one: ' + str(doNew)

set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)


eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])


eventinfoprinter = register_module('EventInfoPrinter')


gearbox = register_module('Gearbox')


# Show progress of processing
progress = register_module('Progress')

if doNew is 0:
    exportVXDstd = register_module('ExportSectorMap')
    exportVXDstd.logging.log_level = LogLevel.INFO
    exportVXDstd.logging.debug_level = 30
    exportVXDstd.param('rootFileName', setupFileNamevxdStd)
    exportVXDstd.param('stretchFactor', [0, 0])  # first for small quantiles, second for normal ones
    exportVXDstd.param('sampleThreshold', [2, 1000])  # first for small quantiles, second for normal ones
    exportVXDstd.param('sampleQuantiles', [0.001, 0.999])  # first for min, second for max
    exportVXDstd.param('smallSampleQuantiles', [0, 1])  # first for min, second for max

    exportSVDstd = register_module('ExportSectorMap')
    exportSVDstd.logging.log_level = LogLevel.INFO
    exportSVDstd.logging.debug_level = 30
    exportSVDstd.param('rootFileName', setupFileNamesvdStd)
    exportSVDstd.param('stretchFactor', [0, 0])  # first for small quantiles, second for normal ones
    exportSVDstd.param('sampleThreshold', [2, 1000])  # first for small quantiles, second for normal ones
    exportSVDstd.param('sampleQuantiles', [0.001, 0.999])  # first for min, second for max
    exportSVDstd.param('smallSampleQuantiles', [0, 1])  # first for min, second for max
elif doNew is 1:
    exportSVDnew = register_module('RawSecMapMerger')
    exportSVDnew.logging.log_level = LogLevel.DEBUG
    exportSVDnew.logging.debug_level = 10
    exportSVDnew.param('rootFileName', setupFileName2s)
    exportSVDnew.param('stretchFactor', [0, 0])  # first for small quantiles, second for normal ones
    exportSVDnew.param('sampleThreshold', [2, 1000])  # first for small quantiles, second for normal ones
    exportSVDnew.param('sampleQuantiles', [0.001, 0.999])  # first for min, second for max
    exportSVDnew.param('smallSampleQuantiles', [0, 1])  # first for min, second for max
    exportSVDnew.param('filterRareCombinations', True)
    exportSVDnew.param('rarenessFilter', 0.005)

    exportVXDnew = register_module('RawSecMapMerger')
    exportVXDnew.logging.log_level = LogLevel.INFO
    exportVXDnew.logging.debug_level = 30
    exportVXDnew.param('rootFileName', setupFileName2)
    exportVXDnew.param('stretchFactor', [0, 0])  # first for small quantiles, second for normal ones
    exportVXDnew.param('sampleThreshold', [2, 1000])  # first for small quantiles, second for normal ones
    exportVXDnew.param('sampleQuantiles', [0.001, 0.999])  # first for min, second for max
    exportVXDnew.param('smallSampleQuantiles', [0, 1])  # first for min, second for max
    exportVXDnew.param('filterRareCombinations', True)
    exportVXDnew.param('rarenessFilter', 0.005)
else:
    print("doNew-parameter is set to illegal parameter (only 0 and 1 allowed): " + str(doNew) + ", aborting program.")
    raise SystemExit

# Create paths
main = create_path()
# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(progress)
main.add_module(gearbox)

if doNew is 1:
    main.add_module(exportVXDnew)
    main.add_module(exportSVDnew)
if doNew is 0:
    main.add_module(exportVXDstd)
    main.add_module(exportSVDstd)

# Process events
process(main)

print statistics
