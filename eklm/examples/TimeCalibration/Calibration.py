#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# EKLM time calibration.

from basf2 import *
from reconstruction import *
from ROOT import Belle2
import sys

# Set the global log level
set_log_level(LogLevel.INFO)

inputFileNames = [sys.argv[1]]

gearbox = register_module('Gearbox')
gearbox.initialize()

algo = Belle2.EKLMTimeCalibrationAlgorithm()
algo.setInputFileNames(inputFileNames)
# Uncomment the following line to draw histograms.
# algo.setDebug()
algo.execute()
algo.commit()
