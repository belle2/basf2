#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

nevents = 10000000
set_log_level(LogLevel.INFO)

main = create_path()

main.add_module('EventInfoSetter', evtNumList=nevents, logLevel=LogLevel.DEBUG)

# DQM Histo Module
histo = register_module('DqmHistoManager')
histo.param('HostName', 'localhost')
histo.param('Port', 9991)
histo.param('DumpInterval', 10000)
main.add_module(histo)

dqmtest = register_module('DAQDummyGaus')
main.add_module(dqmtest)

progress = register_module('Progress')
main.add_module(progress)

process(main)

print(statistics)
