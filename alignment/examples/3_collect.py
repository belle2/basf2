#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
from ROOT import Belle2

# -------------------------------------------------------------------------------------------------------
# Script options

if len(sys.argv) < 2:
    print('Usage: basf2 3_collect.py magnet|no-magnet')
    print('Always specify if magnetic field has ON or OFF')
    sys.exit(1)

magnet_options = {'magnet': True, 'no-magnet': False}
magnet_option = str(sys.argv[1])
magnet = False

if magnet_option not in magnet_options:
    print('Invalid option for magnet configuration: ', magnet, ' Allowed options are: magnet | no-magnet')
    sys.exit(1)
else:
    magnet = magnet_options[magnet_option]

# -------------------------------------------------------------------------------------------------------


set_random_seed(100)

main = create_path()

main.add_module('RootInput')
main.add_module('Gearbox')

if magnet:
    print('Using magnetic field')
    main.add_module('Geometry', components=['BeamPipe', 'MagneticFieldConstant4LimitedRCDC', 'PXD', 'SVD', 'CDC'])
else:
    print('Using NO magnetic field')
    main.add_module('Geometry', components=['BeamPipe', 'PXD', 'SVD', 'CDC'])

main.add_module('SetupGenfitExtrapolation', noiseBetheBloch=False, noiseCoulomb=False, noiseBrems=False)
main.add_module('MillepedeCollector')
main.add_module('RootOutput', branchNames=['EventMetaData'])
main.add_module('Progress')
# main.add_module('GBLdiagnostics')
process(main)

print(statistics)
