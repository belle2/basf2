#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import datetime
from basf2 import *
import ROOT
from ROOT import Belle2
from caf.utils import IoV
d = datetime.datetime.today()
print(d.strftime('This Calculution is done at : %d-%m-%y %H:%M:%S\n'))

iov_to_calibrate = IoV(exp_low=1, run_low=1, exp_high=-1, run_high=-1)
# reset_database()
# use_local_database()
# use_local_database("cdc_crt/database.txt", "cdc_crt")

main = create_path()
# main.add_module('RootInput',inputFileNames = ['rootfile/CAF_cdc.0000.001630.root','rootfile/CAF_cdc.0000.001631.root'])
main.add_module('RootInput', inputFileNames=['rootfile/CAF_all.root'])
main.add_module('Gearbox')
main.add_module('CDCJobCntlParModifier',
                T0File='re201702d/t0.dat',
                T0InputType=False
                )
main.add_module('Geometry', components=['CDC'])
process(main)

algo = Belle2.CDC.T0Correction4CAF()
algo.storeHisto(True)
algo.setDebug(False)
algo.setUseDB(False)

# algo.setPrefix("T0CalibCollector");
exprun_vector = ROOT.vector("std::pair<int,int>")()  # empty runs vector means use everything
alg_result = algo.execute(exprun_vector, 0)
print("result was", alg_result)
dd = datetime.datetime.today()
print(dd.strftime('This Calculution is done at : %d-%m-%y %H:%M:%S\n'))

# algo.execute(iov_to_calibrate)
