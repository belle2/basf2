#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
from ROOT import Belle2

set_log_level(LogLevel.INFO)

if len(sys.argv) < 4:
    print('Usage: basf2 1_generate.py experiment_number run_number num_events')
    sys.exit(1)

experiment = int(sys.argv[1])
run = int(sys.argv[2])
nevents = int(sys.argv[3])

main = create_path()
main.add_module('EventInfoSetter', expList=[experiment], runList=[run], evtNumList=[nevents])
main.add_module('Cosmics')
main.add_module('Gearbox')
main.add_module('Geometry', components=['BeamPipe', 'SVD'])
main.add_module('FullSim')
main.add_module('PXDDigitizer')
main.add_module('SVDDigitizer')
main.add_module('PXDClusterizer')
main.add_module('SVDClusterizer')
main.add_module('RootOutput', outputFileName='DST_exp{:d}_run{:d}.root'.format(experiment, run))
main.add_module('Progress')
process(main)
print(statistics)
