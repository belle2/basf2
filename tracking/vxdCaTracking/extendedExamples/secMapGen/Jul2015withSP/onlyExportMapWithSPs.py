#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
from subprocess import call
from sys import argv

#################

generateSecMap = True  # <----------- hier umschalten zwischen secMapGenerierung und VXDTF!
useSimpleClusterizer = True
useEvtGen = True
useEDeposit = True  # <----- damit False funzt, pxd/data/PXD.xml und svd/data/SVD.xml see neutrons auf true setzen

#################
# Important parameters:

numTracks = 10
numEvents = 1
initialValue = 1  # want random events, if set to 0

# parameters for the secMap-calculation:
pTcuts = [0.025, 0.1, 0.4]


setupFileName = 'evtGenPGunR19500Jul2015Old'
setupFileName2 = 'evtGenPGunR19500Jul2015New'


setupFileNamesvdStd = setupFileName + 'SVD' + 'Std'
setupFileNamevxdStd = setupFileName + 'VXD' + 'Std'

setupFileName2s = setupFileName2 + 'SVD' + 'Std'
setupFileName2 = setupFileName2 + 'VXD' + 'Std'

set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)


eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])


eventinfoprinter = register_module('EventInfoPrinter')
0

gearbox = register_module('Gearbox')

# geometry.set_log_level(LogLevel.INFO) INFO if set to true, complete list of components can be found...

# Show progress of processing
progress = register_module('Progress')


# using one export module only

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


# Create paths
main = create_path()
# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(progress)
main.add_module(gearbox)

main.add_module(exportVXDnew)
main.add_module(exportSVDnew)
main.add_module(exportVXDstd)
main.add_module(exportSVDstd)

# Process events
process(main)

print statistics
