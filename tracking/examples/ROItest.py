#!/usr/bin/env python
# -*- coding: utf-8 -*-

#############################################################################
#
# This steering file
#
##############################################################################

import os
from basf2 import *

numEvents = 10

##first register the modules

evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('expList', [0])
evtmetagen.param('runList', [1])
evtmetagen.param('evtNumList', [numEvents])

##first register the modules
evtinfo = register_module('EventInfo')

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)

roiGen = register_module('ROIGenerator')
param_roiGen = {'ROIListName': 'ROIs', 'nROIs': 4}
roiGen.param(param_roiGen)

roiToOnsen = register_module('ROItoOnsen')
param_roiToOnsen = {'ROIListName': 'ROIs', 'ROIpayloadName': 'ROIpayload'}
roiToOnsen.param(param_roiToOnsen)

roiReadTest = register_module('ROIReadTest')
param_roiReadTest = {'outfileName': 'ROIout.txt',
                     'ROIpayloadName': 'ROIpayload'}
roiReadTest.param(param_roiReadTest)

rootOutput = register_module('RootOutput')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(eventCounter)
main.add_module(evtmetagen)
main.add_module(evtinfo)
main.add_module(roiGen)
main.add_module(roiToOnsen)
main.add_module(roiReadTest)
main.add_module(rootOutput)

# Process events
process(main)

print statistics
