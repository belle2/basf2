#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

# Register modules
hltInput = fw.register_module('HLTInput')
perfTest = fw.register_module('PerfTest')
simpleOutput = fw.register_module('SimpleOutput')
pRootOutput = fw.register_module('pRootOutput')

# Parameter settings
hltInput.param('nodeType', 'EM')
hltInput.param('dataSources', 4)
perfTest.param('overallOutputFileName', 'performance.overall.WN.txt')
perfTest.param('eventsOutputFileName', 'performance.events.WN.txt')

main = fw.create_path()

main.add_module(hltInput)
main.add_module(simpleOutput)
# main.add_module (pRootOutput)
main.add_module(perfTest)

fw.process(main)
