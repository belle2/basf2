#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2 as b2
import sys

argvs = sys.argv
argc = len(argvs)


# set_log_level(LogLevel.ERROR)

main = b2.create_path()

# File input module
input = b2.register_module("RootInput")
main.add_module(input)

# DQM Histogram Module
histo = b2.register_module('DqmHistoManager')
histo.param('histoFileName', 'dqmtest.root')
histo.param('Port', int(argvs[1]))
histo.param('DumpInterval', 10)
main.add_module(histo)

# monitor = register_module('MonitorData')
# main.add_module(monitor)

# progress
progress = b2.register_module("Progress")
main.add_module(progress)

# Start processing
b2.process(main)
