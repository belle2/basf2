#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import calibration_algorithms as calalgs

algo = calalgs.CalibrationAlgorithmNew("Tester")
# Input Python list object works
inputFileNames = ["collector_output/*"]
algo.setInputFileNames(inputFileNames)
# Python list object of files returned after expanding shell search (currently buggy)
[print(x) for x in algo.getInputFileNames()]
