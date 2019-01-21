#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

from basf2 import *

# command arguments
#   argvs[1] = input ring buffer name
#   argvs[2] = output ring buffer name
#   argvs[3] = port number of hserver
#   argvs[4] = number of cores for parallel processing
argvs = sys.argv
argc = len(argvs)

# set_log_level(LogLevel.INFO)
set_log_level(LogLevel.ERROR)

# to avoid undefined symbol
geom = register_module("Geometry")

# path create
main = create_path()


# Rbuf2Ds as input module
# rbuf2ds = register_module("Rbuf2Ds")
# rbuf2ds.param("RingBufferName", argvs[1])
# main.add_module(rbuf2ds)

# Raw2Ds as input module
raw2ds = register_module("Raw2Ds")
raw2ds.param("RingBufferName", argvs[1])
main.add_module(raw2ds)

# Histo Module
# histo = register_module('HistoManager')
# main.add_module (histo)
histo = register_module('DqmHistoManager')
histo.param("Port", 9991)
histo.param("DumpInterval", 10000)
main.add_module(histo)

# Monitor module
monitor = register_module('MonitorData')
main.add_module(monitor)

# Ds2Rbuf as output module
ds2rbuf = register_module("Ds2Rbuf")
ds2rbuf.param("RingBufferName", argvs[2])
main.add_module(ds2rbuf)

# Progress
progress = register_module('Progress')
main.add_module(progress)

# Debug modules
elapsed = register_module('ElapsedTime')
elapsed.param('EventInterval', 10000)
main.add_module(elapsed)

# Process events
set_nprocesses(int(argvs[4]))
process(main)
