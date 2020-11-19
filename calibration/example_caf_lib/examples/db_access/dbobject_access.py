#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2

b2.set_log_level(b2.LogLevel.INFO)

b2.reset_database()
b2.use_local_database('calibration_results/TestCalibration/outputdb/database.txt',
                      directory='calibration_results/TestCalibration/outputdb')

main = b2.create_path()
main.add_module('EventInfoSetter', expList=[1, 1, 1, 1], runList=[1, 2, 3, 4], evtNumList=[1, 1, 1, 1])
main.add_module('TestCalibDBAccess')
main.add_module('Progress')
b2.process(main)
print(b2.statistics)
