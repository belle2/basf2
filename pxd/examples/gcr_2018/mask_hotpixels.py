#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
import ROOT
from ROOT import Belle2
from ROOT.Belle2 import IntervalOfValidity

# Only initialize RootInput, as we do not loop over events,
# only load persistent objects stored during data collection
hotpixelkiller = Belle2.PXDHotPixelMaskCalibrationAlgorithm()

# Set the prefix manually if you want to use the hotpixelkiller for a specific collector
hotpixelkiller.setPrefix("PXDRawHotPixelMaskCollector")

# Can use a Python list of input files/wildcards. It will resolve the existing files
hotpixelkiller.setInputFileNames(["RawCollectorOutput.root"])

# Try raising/lowering with the minEvents to force or solve a not enough data return value
hotpixelkiller.minEvents = 1000000
hotpixelkiller.minHits = 5
hotpixelkiller.maxOccupancy = 0.000001

# Could also define an IoV for your calibrations at the start of execution
iov = IntervalOfValidity.always()
print("Result of calibration =", hotpixelkiller.execute([], 0, iov))
# IoV is applied to payloads during execution and gets used during commit
hotpixelkiller.commit()
