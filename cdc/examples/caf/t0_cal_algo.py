#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import datetime
from basf2 import *
import ROOT
from ROOT import Belle2
from caf.utils import IoV

iov = Belle2.IntervalOfValidity.always()
# reset_database()
use_local_database("localDB/database.txt", "localDB")

main = create_path()
main.add_module('EventInfoSetter',
                expList=[0],
                evtNumList=[1],
                runList=[1630])
main.add_module('Gearbox')
main.add_module('Geometry', components=['CDC'])
process(main, 1)


algo = Belle2.CDC.T0CalibrationAlgorithm()
algo.setInputFileNames(['rootfile/*/CollectorOutput.root'])
# algo.setInputFileNames(['5/output_cdc.*.root'])
algo.storeHisto(True)
algo.setDebug(True)
print("Result of calibration =", algo.execute([], 0, iov))
algo.commit()
