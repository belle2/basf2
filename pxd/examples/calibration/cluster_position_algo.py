#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This steering file to create an localdb containing PXDClusterPosition payloads
# in a localdb
#
# Execute as: basf2 cluster_position_algo.py

from ROOT import Belle2
from ROOT.Belle2 import IntervalOfValidity

# Only initialize RootInput, as we do not loop over events,
# only load persistent objects stored during data collection
clusteralgo = Belle2.PXDClusterPositionCalibrationAlgorithm()

# Set the prefix manually if you want to use the hotpixelkiller for a specific collector
clusteralgo.setPrefix("PXDClusterPositionCollector")

# Can use a Python list of input files/wildcards. It will resolve the existing files
clusteralgo.setInputFileNames(["PXDClusterPositionCollectorOutput_kind_0.root", "PXDClusterPositionCollectorOutput_kind_1.root",
                               "PXDClusterPositionCollectorOutput_kind_2.root", "PXDClusterPositionCollectorOutput_kind_3.root"])

# Here, we can play around with algo parameters
# Minimum number of collected clusters for estimating shape likelyhood
clusteralgo.minClusterForShapeLikelyhood = 250
# Minimum number of collected clusters for estimating cluster position offsets
clusteralgo.minClusterForPositionOffset = 1000
# Maximum number of eta bins for estimating cluster position offsets
clusteralgo.maxEtaBins = 10
# Select the clusterkinds to calibrate. This should match with the supplied source files.
clusteralgo.clusterKinds.push_back(0)
clusteralgo.clusterKinds.push_back(1)
clusteralgo.clusterKinds.push_back(2)
clusteralgo.clusterKinds.push_back(3)

# Could also define an IoV for your calibrations at the start of execution
iov = IntervalOfValidity.always()
print("Result of calibration =", clusteralgo.execute([], 0, iov))
# IoV is applied to payloads during execution and gets used during commit
clusteralgo.commit()
