#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
from ROOT import Belle2

set_random_seed(100)

reset_database()
use_local_database()

main = create_path()


main.add_module('RootInput')
main.add_module('Gearbox')
main.add_module('Geometry', components=['BeamPipe', 'MagneticFieldConstant4LimitedRCDC', 'PXD', 'SVD'])
main.add_module('MagnetSwitcher')

main.add_module('GBLfit', UseClusters=True)

main.add_module('MillepedeCollector')
main.add_module('RootOutput', branchNames=['EventMetaData'])
main.add_module('Progress')
# main.add_module('GBLdiagnostics')
process(main)

print(statistics)
