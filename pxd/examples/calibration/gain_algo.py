#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file to create an localdb containing PXDGainMapPar payloads
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

# set some random seed
set_random_seed(10346)


# Only initialize RootInput, as we do not loop over events,
# only load persistent objects stored during data collection
algo = Belle2.PXDGainCalibrationAlgorithm()

# Set the prefix manually if you want to use the hotpixelkiller for a specific collector
algo.setPrefix("PXDGainCollector")

# Can use a Python list of input files/wildcards. It will resolve the existing files
algo.setInputFileNames(['PXDGainCollectorOutput_MC.root', 'PXDGainCollectorOutput_Data.root'])


# Could also define an IoV for your calibrations at the start of execution
iov = IntervalOfValidity.always()
print("Result of calibration =", algo.execute([], 0, iov))
# IoV is applied to payloads during execution and gets used during commit
# clusteralgo.commit()
