#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ------------------------------------------------------------------------------------------------------
#           TSim-ecl example code.
# ------------------------------------------------------------------------------------------------------
#       In order to test Tsim-ecl code, you need a root file which has ECLHit table.(after Gsim)
#       ex)
#       commend > basf2 TrgEcl_pgun.py [Name of output root file]
# ------------------------------------------------------------------------------------------------------
import os
from basf2 import *
from simulation import add_simulation
from gdltrigger import add_tsim
from modularAnalysis import *

import glob

main = create_path()
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

babayaganlo = register_module('BabayagaNLOInput')
babayaganlo.param('FMax', 7.5e4)
babayaganlo.param('FinalState', 'ee')
babayaganlo.param('MaxAcollinearity', 180.0)
babayaganlo.param('ScatteringAngleRange', [20.0, 150.0])
# babayaganlo.param('VacuumPolarization', 'hadr5')
babayaganlo.param('VacuumPolarization', 'hlmnt')
babayaganlo.param('SearchMax', 10000)
babayaganlo.param('VPUncertainty', True)
main.add_module(babayaganlo)

# generateY4S(10, "mixed.dec", main)

add_simulation(main)

# add trigger
add_tsim(main)

# output
rootoutput = register_module('RootOutput')
rootoutput.param('outputFileName', "BBmixed.root")
main.add_module(rootoutput)


# main
process(main)
print(statistics)
# ===<END>
