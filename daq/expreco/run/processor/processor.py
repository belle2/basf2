#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
rbuf2ds = b2.register_module("Rbuf2Ds")
rbuf2ds.param("RingBufferName", argvs[1])
main.add_module(rbuf2ds)

# Raw2Ds as input module
# raw2ds = register_module("Raw2Ds")
# raw2ds.param("RingBufferName", argvs[1])
# main.add_module(raw2ds)

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
