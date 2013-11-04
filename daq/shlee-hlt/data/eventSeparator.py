#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
from basf2 import *

# Register modules
evtMetaGen = fw.register_module('EventInfoSetter')
particleGun = fw.register_module('PGunInput')
hltOutput = fw.register_module('HLTOutput')
simpleOutput = fw.register_module('SimpleOutput')
seqRootOutput = fw.register_module('SeqRootOutput')
perfTest = fw.register_module('PerfTest')

# Parameter settings
evtMetaGen.param('EvtNumList', [10])
particleGun.param('nTracks', 10)
particleGun.param('PIDcodes', [-11, 11])
particleGun.param('pPar1', 0.05)
particleGun.param('pPar2', 3)

hltOutput.param('nodeType', 'ES')
perfTest.param('overallOutputFileName', 'performance.overall.ES.txt')
perfTest.param('eventsOutputFileName', 'performance.events.ES.txt')

main = fw.create_path()

main.add_module(evtMetaGen)
main.add_module(particleGun)
main.add_module(hltOutput)
# main.add_module(simpleOutput)
# main.add_module (seqRootOutput)
main.add_module(perfTest)

fw.process(main)
