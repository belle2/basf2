#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file to create an localdb containing PXDClusterPositionAndAngle payloads
# in a localdb
#
# Execute as: basf2 cluster_position_angle_algo.py
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
clusteralgo = Belle2.PXDClusterPositionAndAngleCalibrationAlgorithm()

# Set the prefix manually if you want to use the hotpixelkiller for a specific collector
clusteralgo.setPrefix("PXDClusterPositionAndAngleCollector")

# Can use a Python list of input files/wildcards. It will resolve the existing files
clusteralgo.setInputFileNames(["PXDClusterPositionAndAngleCollectorOutput_kind_0.root", ])

# Here, we can play around with algo parameters
# Minimum number of collected clusters for estimating shape likelyhood
clusteralgo.minClusterForShapeLikelyhood = 250
# Minimum number of collected clusters for estimating cluster position offsets
clusteralgo.minClusterForPositionOffset = 1000
# Maximum number of eta bins for estimating cluster position offsets
clusteralgo.maxEtaBins = 1
# Select the clusterkinds to calibrate. This should match with the supplied source files.
clusteralgo.clusterKinds.push_back(0)
# clusteralgo.clusterKinds.push_back(1)
# clusteralgo.clusterKinds.push_back(2)
# clusteralgo.clusterKinds.push_back(3)

# Could also define an IoV for your calibrations at the start of execution
iov = IntervalOfValidity.always()
print("Result of calibration =", clusteralgo.execute([], 0, iov))
# IoV is applied to payloads during execution and gets used during commit
clusteralgo.commit()
