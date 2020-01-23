#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from ROOT import Belle2
from caf.utils import runs_from_vector, find_run_lists_from_boundaries

algo = Belle2.TestCalibrationAlgorithm()

# Can use a Python list of input files/wildcards. It will resolve the existing files
inputFileNames = ["CollectorOutput.root"]
algo.setInputFileNames(inputFileNames)
all_runs = algo.getRunListFromAllData()
boundaries = runs_from_vector(algo.findPayloadBoundaries(all_runs))

print("Boundaries were:")
for exprun in boundaries:
    print(exprun)

print("Run lists associated to boundary IoVs")

for iov, runs in find_run_lists_from_boundaries(boundaries, runs_from_vector(all_runs)).items():
    print(iov, runs)
