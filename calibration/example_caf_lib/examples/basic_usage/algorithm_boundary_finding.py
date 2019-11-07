#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from ROOT import Belle2

algo = Belle2.TestCalibrationAlgorithm()

# Can use a Python list of input files/wildcards. It will resolve the existing files
inputFileNames = ["CollectorOutput.root"]
algo.setInputFileNames(inputFileNames)
all_runs = algo.getRunListFromAllData()
boundaries = algo.findPayloadBoundaries(all_runs)

print("Boundaries were:")
for exprun in boundaries:
    print(f"({exprun.first},{exprun.second})")
