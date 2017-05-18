#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
import ROOT
from ROOT import Belle2
import numpy as np

set_debug_level(1000)

algo = Belle2.TestCalibrationAlgorithm()
main = create_path()
main.add_module("RootInput", inputFileNames=['CollectorOutput_*.root'])
process(main)


# Example how to access collected data (but you need exp and run number)
hist1 = Belle2.PyStoreObj('CaTest_histogram1', 1).obj().getObject('1.1')
hist2 = Belle2.PyStoreObj('CaTest_histogram1', 1).obj().getObject('1.2')
hist3 = Belle2.PyStoreObj('CaTest_histogram1', 1).obj().getObject('1.3')
hist4 = Belle2.PyStoreObj('CaTest_histogram1', 1).obj().getObject('1.4')
hist1.Print()
hist2.Print()
hist3.Print()
hist4.Print()

algo.execute()

# for obj in Belle2.PyStoreObj.list(Belle2.DataStore.c_Persistent):
#    print(obj)

# algo.commit()
