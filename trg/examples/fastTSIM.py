#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# The example of running simulation of L1 trigger

import basf2 as b2
from simulation import add_simulation
from L1trigger import add_trigger_simulation

# b2.set_log_level(b2.LogLevel.DEBUG)

main = b2.create_path()
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

babayaganlo = b2.register_module('BabayagaNLOInput')
babayaganlo.param('FMax', 7.5e4)
babayaganlo.param('FinalState', 'ee')
babayaganlo.param('MaxAcollinearity', 180.0)
babayaganlo.param('ScatteringAngleRange', [15, 165])
# babayaganlo.param('VacuumPolarization', 'hadr5')
babayaganlo.param('VacuumPolarization', 'hlmnt')
babayaganlo.param('SearchMax', 10000)
babayaganlo.param('VPUncertainty', True)
main.add_module(babayaganlo)


# trigger simulation is included by default
add_simulation(main, simulateT0jitter=True, usePXDDataReduction=False, forceSetPXDDataReduction=True)

for m in main.modules():
    if m.name() == "TRGGDL":
        m.set_log_level(b2.LogLevel.DEBUG)
#        m.set_debug_level(25)

# output
rootoutput = b2.register_module('RootOutput')
rootoutput.param('outputFileName', "test.root")
main.add_module(rootoutput)


# main
b2.process(main)
print(b2.statistics)
# ===<END>
