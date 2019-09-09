#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# EKLM time calibration.

import sys
import basf2
from ROOT import Belle2

# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

gearbox = basf2.register_module('Gearbox')
gearbox.initialize()

algo = Belle2.EKLMTimeCalibrationAlgorithm()
algo.setInputFileNames(sys.argv[1:])
# Uncomment the following line to draw histograms.
# algo.setDebug()
algo.execute()
algo.commit()
