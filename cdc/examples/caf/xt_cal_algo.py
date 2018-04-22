#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import datetime
from basf2 import *
import ROOT
from ROOT import Belle2
from caf.utils import IoV


reset_database()
use_database_chain()
use_central_database("332_COPY-OF_GT_gen_prod_004.11_Master-20171213-230000", LogLevel.INFO)
use_central_database("MagneticFieldPhase2QCSoff")
use_local_database("localDB/database.txt", "localDB")
# use_local_database("/home/belle/muchida/basf2/work/caf/gcr2/test7/localDB/database.txt")

algo = Belle2.CDC.XTCalibrationAlgorithm()
algo.setInputFileNames(['rootfile/*/CollectorOutput.root'])

# Ture, f you set IOV for whole exp and runs.
# iov = Belle2.IntervalOfValidity.always()
# print("Result of calibration =", algo.execute([], 0, iov))
print("Result of calibration =", algo.execute())
algo.commit()
