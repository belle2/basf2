#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will
import os
from sys import argv
from basf2 import *
from time import time

numEvents = 25
initialValue = 42


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

spCreatorSingle = register_module('SpacePointCreator')
spCreatorSingle.logging.log_level = LogLevel.DEBUG
spCreatorSingle.logging.debug_level = 1
spCreatorSingle.param('OnlySingleClusterSpacePoints', True)
spCreatorSingle.param('NameOfInstance', 'singlesOnly')

spCreatorCombi = register_module('SpacePointCreator')
spCreatorCombi.logging.log_level = LogLevel.DEBUG
spCreatorCombi.logging.debug_level = 1
spCreatorCombi.param('OnlySingleClusterSpacePoints', False)
spCreatorCombi.param('NameOfInstance', 'couplesAllowed')

spCreatorTest = register_module('SpacePointCreatorTest')
spCreatorTest.logging.log_level = LogLevel.DEBUG
spCreatorTest.logging.debug_level = 20
spCreatorTest.param('NameOfInstance', 'SPTester')

# Create paths
main = create_path()
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
main.add_module(spCreatorSingle)
#main.add_module(spCreatorCombi)
main.add_module(spCreatorTest)


# Process events
process(main)

print 'Event Statistics :'
print statistics
