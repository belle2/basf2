#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from subprocess import call
from sys import argv


numEvents = 1
initialValue = 1  # want random events, if set to 0

# parameters for the secMap-calculation:

setupFileName = 'TESTSecMaps'

setupFileNameSVD = setupFileName + 'SVD' + 'Std'
setupFileNameVXD = setupFileName + 'VXD' + 'Std'


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

exportSVDnew = register_module('RawSecMapMergerWithSpacePoints')
exportSVDnew.logging.log_level = LogLevel.DEBUG
exportSVDnew.logging.debug_level = 10
exportSVDnew.param('rootFileName', setupFileNameSVD)
exportSVDnew.param('stretchFactor', [0, 0])  # first for small quantiles, second for normal ones
exportSVDnew.param('sampleThreshold', [2, 1000])  # first for small quantiles, second for normal ones
exportSVDnew.param('sampleQuantiles', [0.001, 0.999])  # first for min, second for max
exportSVDnew.param('smallSampleQuantiles', [0, 1])  # first for min, second for max
exportSVDnew.param('filterRareCombinations', True)
exportSVDnew.param('rarenessFilter', 0.005)

exportVXDnew = register_module('RawSecMapMergerWithSpacePoints')
exportVXDnew.logging.log_level = LogLevel.INFO
exportVXDnew.logging.debug_level = 30
exportVXDnew.param('rootFileName', setupFileNameVXD)
exportVXDnew.param('stretchFactor', [0, 0])  # first for small quantiles, second for normal ones
exportVXDnew.param('sampleThreshold', [2, 1000])  # first for small quantiles, second for normal ones
exportVXDnew.param('sampleQuantiles', [0.001, 0.999])  # first for min, second for max
exportVXDnew.param('smallSampleQuantiles', [0, 1])  # first for min, second for max
exportVXDnew.param('filterRareCombinations', True)
exportVXDnew.param('rarenessFilter', 0.005)
log_to_file('exportSecMapLog.txt', append=False)

# Create paths
main = create_path()
# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(progress)
main.add_module(gearbox)

main.add_module(exportVXDnew)
main.add_module(exportSVDnew)


# Process events
process(main)

print(statistics)
