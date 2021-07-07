#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import Belle2

algo = Belle2.TestCalibrationAlgorithm()
# Set the prefix manually if you want to use the algorithm for a specific collector
# algo.setPrefix("CaTest")
algo.setDebugHisto(True)

# Can use a Python list of input files/wildcards. It will resolve the existing files
inputFileNames = ["CollectorOutput.root"]
algo.setInputFileNames(inputFileNames)
# Python list object of files returned
d = algo.getInputFileNames()
for name in d:
    print("Resolved input file to algorithm:", name)

# Can use python lists/tuples to define the runs whose data you want to use [(exp,run), ...]
# IoV of the payloads will be calculated from this input list
print("Result of calibration =", algo.execute([(1, 1), (1, 2)]))
output_json_string = algo.dumpOutputJson()
print(f"Output JSON was {output_json_string}")
# localdb isn't updated until you call this
# algo.commit()

# Can run over all collected data and auto define the IoV of the payloads
print("Result of calibration =", algo.execute())
output_json_string = algo.dumpOutputJson()
print(f"Output JSON was {output_json_string}")
# Can commit multiple times in one process since the payloads are cleared at the start
# of each execution
# algo.commit()

# Can set the input JSON from a previous execution's output JSON
algo.loadInputJson(output_json_string)
# Could also define an IoV for your calibrations at the start of execution
iov = Belle2.IntervalOfValidity.always()
print("Result of calibration =", algo.execute([], 0, iov))
output_json_string = algo.dumpOutputJson()
print(f"Output JSON was {output_json_string}")
# IoV is applied to payloads during execution and gets used during commit
algo.commit()
