#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will simulate several testbeam events, reconstruct
# and fit tracks, and display each event.
import os
from basf2 import *
from subprocess import call
from time import time
from sys import argv

secSetup = 'testBeamSTD2'  # ,'evtgenHIGH' 'evtgenSTD', 'evtgenLOW'] # when using the same secSetup several times, use this for setting Value
qiType = 'circleFit'
filterOverlaps = 'hopfield'
detectType = 'VXD'
seed = 1
numEvents = 1

if len(argv) is 1:
    print ' no arguments given, using standard values'
elif len(argv) is 2:
    seed = int(argv[1])
elif len(argv) is 3:
    seed = int(argv[2])
    numEvents = int(argv[1])

set_log_level(LogLevel.ERROR)
set_random_seed(seed)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

gearbox = register_module('Gearbox')
# use simple testbeam geometry
gearbox.param('fileName', 'testbeam/vxd/VXD.xml')

# geometry = register_module('Geometry')
# only the tracking detectors will be simulated. Makes this example much faster
# geometry.param('Components', ['MagneticField', 'TB'])

exportXML = register_module('ExportSectorMap')
exportXML.logging.log_level = LogLevel.DEBUG
exportXML.logging.debug_level = 1
exportXML.param('detectorType', detectType)
exportXML.param('sectorSetup', secSetup)

# Create paths
main = create_path()
# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
# main.add_module(geometry)
main.add_module(exportXML)
# Process events
process(main)

print statistics

print 'Event Statistics for exportXML:'
print statistics([exportXML])

print 'Memory statistics'
for stats in statistics.modules:
    print 'Module %s:' % stats.name
    print ' -> initialize(): %10d KB' % stats.memory(statistics.INIT)
    print ' -> beginRun():   %10d KB' % stats.memory(statistics.BEGIN_RUN)
    print ' -> event():      %10d KB' % stats.memory()
    print ' -> endRun():     %10d KB' % stats.memory(statistics.END_RUN)
    print ' -> terminate():  %10d KB' % stats.memory(statistics.TERM)

print 'Event Statistics detailed:'
print statistics(statistics.TOTAL)

