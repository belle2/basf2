#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import sys

import basf2 as b2

# command arguments
#   argvs[1] = input ring buffer name
#   argvs[2] = output ring buffer name
#   argvs[3] = port number of hserver
#   argvs[4] = number of cores for parallel processing
argvs = sys.argv
argc = len(argvs)

# set_log_level(LogLevel.INFO)
b2.set_log_level(b2.LogLevel.ERROR)

# to avoid undefined symbol
geom = b2.register_module("Geometry")

# path create
main = b2.create_path()


# Rbuf2Ds as input module
# rbuf2ds = register_module("Rbuf2Ds")
# rbuf2ds.param("RingBufferName", argvs[1])
# main.add_module(rbuf2ds)

# Raw2Ds as input module
raw2ds = b2.register_module("Raw2Ds")
raw2ds.param("RingBufferName", argvs[1])
main.add_module(raw2ds)

# Histo Module
# histo = register_module('HistoManager')
# main.add_module (histo)
histo = b2.register_module('DqmHistoManager')
histo.param("Port", 9991)
histo.param("DumpInterval", 10000)
main.add_module(histo)

# Monitor module
monitor = b2.register_module('MonitorData')
main.add_module(monitor)

# RoI Related codes here
roiGen = b2.register_module('ROIGenerator')
param_roiGen = {'ROIListName': 'ROIs',
                'nROIs': 8, 'Layer': 1, 'Ladder': 1, 'Sensor': 1}
roiGen.param(param_roiGen)


roiPayloadAssembler = b2.register_module('ROIPayloadAssembler')
param_roiPayloadAssembler = {'ROIListName': 'ROIs',
                             'ROIpayloadName': 'ROIpayload'}
roiPayloadAssembler.param(param_roiPayloadAssembler)

roiReadTest = b2.register_module('ROIReadTest')
param_roiReadTest = {'outfileName': 'ROIoutHLT.txt',
                     'ROIpayloadName': 'ROIpayload'}
roiReadTest.param(param_roiReadTest)

main.add_module(roiGen)
main.add_module(roiPayloadAssembler)
# main.add_module(roiReadTest)


# Ds2Rbuf as output module
ds2rbuf = b2.register_module("Ds2Rbuf")
ds2rbuf.param("RingBufferName", argvs[2])
main.add_module(ds2rbuf)

# Progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Debug modules
elapsed = b2.register_module('ElapsedTime')
elapsed.param('EventInterval', 10000)
main.add_module(elapsed)

# Process events
b2.set_nprocesses(int(argvs[4]))
b2.process(main)
