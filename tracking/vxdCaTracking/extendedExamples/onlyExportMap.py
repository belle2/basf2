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
pTcuts = [0.05, 0.5]
setupFileName = 'secMapEvtGenOnR10454May2014'

set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

gearbox = register_module('Gearbox')

# geometry.set_log_level(LogLevel.INFO) INFO if set to true, complete list of components can be found...

# Show progress of processing
progress = register_module('Progress')

# using one export module only
exportXML = register_module('ExportSectorMap')
exportXML.logging.log_level = LogLevel.DEBUG
exportXML.logging.debug_level = 30
exportXML.param('rootFileName', setupFileName)
exportXML.param('stretchFactor', [0, 0])  # first for small quantiles, second for normal ones
exportXML.param('sampleThreshold', [2, 1000])  # first for small quantiles, second for normal ones
exportXML.param('sampleQuantiles', [0.001, 0.999])  # first for min, second for max
exportXML.param('smallSampleQuantiles', [0, 1])  # first for min, second for max

# Create paths
main = create_path()
# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(progress)
main.add_module(gearbox)

main.add_module(exportXML)

# Process events
process(main)

print statistics
