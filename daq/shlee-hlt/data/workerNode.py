#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
from basf2 import *

# Register modules
hltInput = fw.register_module('HLTInput')
paramLoader = fw.register_module('ParamLoaderXML')
geoBuilder = fw.register_module('GeoBuilder')
fullSim = fw.register_module('FullSim')
hltOutput = fw.register_module('HLTOutput')
simpleOutput = fw.register_module('SimpleOutput')
pRootOutput = fw.register_module('pRootOutput')
seqRootOutput = fw.register_module('SeqRootOutput')
perfTest = fw.register_module('PerfTest')

# Parameter settings
paramLoader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))

hltInput.param('nodeType', 'WN')
hltOutput.param('nodeType', 'WN')
perfTest.param('overallOutputFileName', 'performance.overall.WN.txt')
perfTest.param('eventsOutputFileName', 'performance.events.WN.txt')

main = fw.create_path()

main.add_module(hltInput)
main.add_module(paramLoader)
main.add_module(geoBuilder)
main.add_module(fullSim)
main.add_module(hltOutput)
main.add_module(simpleOutput)
# main.add_module (pRootOutput)
# main.add_module (seqRootOutput)
main.add_module(perfTest)

fw.process(main)
