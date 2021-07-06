#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from ROOT import Belle2


b2.reset_database()
b2.use_database_chain()
b2.use_central_database("Calibration_Offline_Development", b2.LogLevel.INFO)
b2.use_central_database("MagneticFieldPhase2QCSoff")
# use_local_database("localDB/database.txt", "localDB")
b2.use_local_database("/home/belle/muchida/basf2/release/cdc/examples/caf/localDB/database.txt")
# use_local_database("/home/belle/muchida/basf2/work/caf/gcr2/test7/localDB/database.txt")

algo = Belle2.CDC.T0CalibrationAlgorithm()
# algo.setInputFileNames(['../test5/calib_result/8/rootfile/*/CollectorOutput.root'])
algo.setInputFileNames(['rootfile/*/CollectorOutput.root'])
algo.storeHisto(True)
algo.setDebug(True)
algo.setMinimumNDF(20)
# algo.setMinimumPval(0.00001)
# algo.enableTextOutput(True)

# Ture, f you set IOV for whole exp and runs.
# iov = Belle2.IntervalOfValidity.always()
# print("Result of calibration =", algo.execute([], 0, iov))
print("Result of calibration =", algo.execute())
algo.commit()
