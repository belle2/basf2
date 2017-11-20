#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

argvs = sys.argv
argc = len(argvs)

from basf2 import *

# set_log_level(LogLevel.ERROR)

main = create_path()

# File input module
input = register_module("RootInput")
main.add_module(input)

# DQM Histogram Module
histo = register_module('DqmHistoManager')
histo.param('histoFileName', 'dqmtest.root')
histo.param('Port', int(argvs[1]))
histo.param('DumpInterval', 10)
main.add_module(histo)

# monitor = register_module('MonitorData')
# main.add_module(monitor)

# progress
progress = register_module("Progress")
main.add_module(progress)

# Start processing
process(main)
