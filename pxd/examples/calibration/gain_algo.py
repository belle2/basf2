#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file to create an localdb containing PXDGain payloads
# in a localdb
#
# Execute as: basf2 gain_algo.py
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de

import os
import sys
from basf2 import *
import ROOT
from ROOT import Belle2
from ROOT.Belle2 import IntervalOfValidity

# Only initialize RootInput, as we do not loop over events,
# only load persistent objects stored during data collection
algo = Belle2.PXDGainCalibrationAlgorithm()

# Set the prefix manually if you want to use the hotpixelkiller for a specific collector
algo.setPrefix("PXDGainCollector")

# Can use a Python list of input files/wildcards. It will resolve the existing files
algo.setInputFileNames(['PXDGainCollectorOutput.root'])

# Here, we can play around with algo parameters
# Minimum number of collected clusters for estimating shape likelyhood
# algo.minClusterForShapeLikelyhood = 250
# Minimum number of collected clusters for estimating cluster position offsets
# algo.minClusterForPositionOffset = 1000


# Could also define an IoV for your calibrations at the start of execution
iov = IntervalOfValidity.always()
print("Result of calibration =", algo.execute([], 0, iov))
# IoV is applied to payloads during execution and gets used during commit
# clusteralgo.commit()
