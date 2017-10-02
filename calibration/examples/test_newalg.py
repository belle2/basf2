#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
import ROOT
from ROOT import Belle2
import time

set_log_level(LogLevel.DEBUG)

algo = Belle2.TestCalibrationAlgorithm()
# Input Python list object works
inputFileNames = ["./collector_output/*"]
algo.setInputFileNames(inputFileNames)
# Python list object of files returned after expanding shell search
d = algo.getInputFileNames()
for name in d:
    print(name)
print(algo.execute([(1, 1), (1, 2)]))
