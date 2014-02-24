#!/usr/bin/env python
# -*- coding: utf-8 -*-

#############################################################################
#
# This steering file
#
##############################################################################

import os
from basf2 import *

numEvents = 100

##first register the modules

evtnumber = register_module('EventInfoSetter')
evtnumber.param('expList', [0])
evtnumber.param('runList', [1])
evtnumber.param('evtNumList', [numEvents])

##first register the modules
evtinfo = register_module('EventInfoPrinter')

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)

roiGen = register_module('ROIGenerator')
param_roiGen = {'ROIListName': 'ROIs', 'nROIs': 8}
roiGen.param(param_roiGen)

roiPayloadAssembler = register_module('ROIPayloadAssembler')
param_roiPayloadAssembler = {'ROIListName': 'ROIs',
                             'ROIpayloadName': 'ROIpayload'}
roiPayloadAssembler.param(param_roiPayloadAssembler)

roiReadTest = register_module('ROIReadTest')
param_roiReadTest = {'outfileName': 'ROIout.txt',
                     'ROIpayloadName': 'ROIpayload'}
roiReadTest.param(param_roiReadTest)

roiSender = register_module('ROISender')
param_roiSender = {
    'MessageQueueName': '/roi',
    'ROIpayloadName': 'ROIpayload',
    'MessageQueueDepth': 10,
    'MessageSize': 8192,
    }
roiSender.param(param_roiSender)

rootOutput = register_module('RootOutput')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(eventCounter)
main.add_module(evtnumber)
main.add_module(evtinfo)
main.add_module(roiGen)
main.add_module(roiPayloadAssembler)
# main.add_module(roiReadTest)
main.add_module(roiSender)
main.add_module(rootOutput)

# Process events
process(main)

print statistics
