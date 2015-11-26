#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
from ROOT import Belle2

reset_database()
use_local_database()

main = create_path()


main.add_module('RootInput')
main.add_module('Gearbox')
main.add_module('Geometry', components=['BeamPipe', 'MagneticFieldConstant4LimitedRCDC', 'PXD', 'SVD'])
main.add_module('MagnetSwitcher')
"""
misalignment = cal.VXDMisalignment('VXDMisalignment')
# misalign everything
misalignment.genSensorU('0.0.0', 0.01)
misalignment.genSensorV('0.0.0', 0.01)
misalignment.genSensorW('0.0.0', 0.01)
misalignment.genSensorAlpha('0.0.0', 0.001)
misalignment.genSensorBeta('0.0.0', 0.001)
misalignment.genSensorGamma('0.0.0', 0.001)

# Reset misalignment to zero for sensor we fix
# in PedeSteering (see below)
misalignment.setSensorAll('4.0.1', 0.0)
misalignment.setSensorAll('5.0.1', 0.0)
misalignment.setSensorAll('6.0.0', 0.0)

main.add_module(
    'MisalignmentGenerator',
    experimentLow=0,
    runLow=0,
    experimentHigh=-1,
    runHigh=-1,
    name=misalignment.getName(),
    data=misalignment.getMisalignment())
"""
main.add_module('GBLfit', UseClusters=False)

main.add_module('MillepedeCollector')
main.add_module('RootOutput', branchNames=['EventMetaData'])
main.add_module('Progress')
process(main)

print(statistics)
