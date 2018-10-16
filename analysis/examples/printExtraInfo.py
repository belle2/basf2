#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from ROOT import Belle2
import basf2 as b2
import modularAnalysis as ma

main = b2.create_path()
ma.inputMdst("MC10", Belle2.FileSystem.findFile("analysis/tests/mdst.root"), path=main)

# load the pions and write a something to extra info
ma.fillParticleList('pi+:example', '', path=main)
ma.variablesToExtraInfo("pi+:example", {"random": "someOtherExtraInfoName"}, path=main)

# print the extra infor associated to the pions
particleWisePrinter = b2.register_module("ExtraInfoPrinter")
particleWisePrinter.set_name("ExtraInfoPrinter_Particles")
particleWisePrinter.param("particleList", 'pi+:example')
main.add_module(particleWisePrinter)

# make some nonsense cuts just to write something to EventExtraInfo
# (eventrandom writes to EventExtraInfo)
ma.cutAndCopyList("pi+:filtered", "pi+:example", "eventRandom > 0.5", path=main)

# print the extra info associated to this event
eventWisePrinter = b2.register_module("ExtraInfoPrinter")
eventWisePrinter.set_name("ExtraInfoPrinter_Event")
eventWisePrinter.param('particleList', '')
main.add_module(eventWisePrinter)

b2.process(main)
print(b2.statistics)
