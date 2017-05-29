#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

set_log_level(LogLevel.INFO)

reset_database()
use_local_database('calibration_results/TestCalibration/outputdb/database.txt',
                   directory='calibration_results/TestCalibration/outputdb')

main = create_path()
main.add_module('EventInfoSetter', expList=[1, 1, 1, 1], runList=[1, 2, 3, 4], evtNumList=[1, 1, 1, 1])
main.add_module('TestCalibDBAccess')
main.add_module('Progress')
process(main)
print(statistics)
