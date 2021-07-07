#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
import modularAnalysis as ma

main = basf2.create_path()
ma.inputMdst("default", basf2.find_file("analysis/tests/mdst.root"), path=main)

# load the pions and write a something to extra info
ma.fillParticleList('pi+:example', '', path=main)
ma.variablesToExtraInfo("pi+:example", {"random": "someOtherExtraInfoName"}, path=main)

# print the extra info associated to the pions
particleWisePrinter = basf2.register_module("ExtraInfoPrinter")
particleWisePrinter.set_name("ExtraInfoPrinter_Particles")
particleWisePrinter.param("particleList", 'pi+:example')
main.add_module(particleWisePrinter)

# make some nonsense cuts just to write something to EventExtraInfo
# (eventrandom writes to EventExtraInfo)
ma.cutAndCopyList("pi+:filtered", "pi+:example", "eventRandom > 0.5", path=main)

# print the extra info associated to this event
eventWisePrinter = basf2.register_module("ExtraInfoPrinter")
eventWisePrinter.set_name("ExtraInfoPrinter_Event")
eventWisePrinter.param('particleList', '')
main.add_module(eventWisePrinter)

basf2.process(main)
print(basf2.statistics)
