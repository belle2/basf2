#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2
import hbasf2
from softwaretrigger import constants
from softwaretrigger.processing import finalize_zmq_path, setup_basf2_and_db, start_zmq_path
from pxd import add_roi_payload_assembler

args = setup_basf2_and_db(zmq=True)

path, reco_path = start_zmq_path(args, location=constants.Location.hlt)

# Monitor module
reco_path.add_module('MonitorData')

# ROI Payload related
reco_path.add_module('ROIGenerator', ROIListName='ROIs', nROIs=8, Layer=1, Ladder=1, Sensor=1)
add_roi_payload_assembler(reco_path, ignore_hlt_decision=True)
# reco_path.add_module('ROIReadTest', outfileName='ROIoutHLT.txt',ROIpayloadName='ROIpayload')

# Debug modules
reco_path.add_module('ElapsedTime', EventInterval=10000)

finalize_zmq_path(path, args, location=constants.Location.hlt)

basf2.print_path(path)
hbasf2.process(path, [args.dqm, args.output], True)
