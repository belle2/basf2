#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This steering file to create an localdb containing VTXClusterPosition payloads
# in a localdb
#
# Execute as: basf2 cluster_position_algo.py

import os
import sys
import glob
import basf2 as b2
import ROOT
from ROOT import Belle2
from ROOT.Belle2 import IntervalOfValidity

# Only initialize RootInput, as we do not loop over events,
# only load persistent objects stored during data collection
clusteralgo = Belle2.VTXClusterPositionCalibrationAlgorithm()

# Set the prefix manually if you want to use the hotpixelkiller for a specific collector
clusteralgo.setPrefix("VTXClusterPositionCollector")

# Can use a Python list of input files/wildcards. It will resolve the existing files
# We have one type of pixel for VTX, that is one cluster kind.
clusteralgo.setInputFileNames(list(glob.glob("VTXClusterPositionCollectorOutput_kind_0_*.root")))

# Here, we can play around with algo parameters
# Minimum number of collected clusters for estimating shape likelyhood
clusteralgo.minClusterForShapeLikelyhood = 250
# Minimum number of collected clusters for estimating cluster position offsets
clusteralgo.minClusterForPositionOffset = 1000
# Maximum number of eta bins for estimating cluster position offsets
clusteralgo.maxEtaBins = 10
# Select the clusterkinds to calibrate. This should match with the supplied source files.
clusteralgo.clusterKinds.push_back(0)

# Could also define an IoV for your calibrations at the start of execution
iov = IntervalOfValidity.always()
print("Result of calibration =", clusteralgo.execute([], 0, iov))
# IoV is applied to payloads during execution and gets used during commit
clusteralgo.commit()
