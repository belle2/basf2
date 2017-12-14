#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
from ROOT import Belle2

# set_random_seed(101)
set_log_level(LogLevel.INFO)

if len(sys.argv) < 5:
    print('Usage: basf2 0_generateReconstructSample.py experiment_number run_number num_events magnet=0|1')
    sys.exit(1)

experiment = int(sys.argv[1])
run = int(sys.argv[2])
nevents = int(sys.argv[3])
magnet = bool(int(sys.argv[4]))

main = create_path()
main.add_module('EventInfoSetter', expList=[experiment], runList=[run], evtNumList=[nevents])
main.add_module('Cosmics')
main.add_module('Gearbox')

if magnet:
    main.add_module('Geometry', components=['MagneticFieldConstant4LimitedRCDC', 'CDC'])
else:
    main.add_module('Geometry', components=['CDC'])

main.add_module('FullSim')
main.add_module('CDCDigitizer')
main.add_module('SetupGenfitExtrapolation')
main.add_module('TrackFinderMCTruthRecoTracks', WhichParticles='CDC')
main.add_module('DAFRecoFitter')
main.add_module('RootOutput', outputFileName='DST_exp{:d}_run{:d}.root'.format(experiment, run))

# main.add_module('Display')

main.add_module('Progress')
process(main)
print(statistics)
