#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from tracking import *
from svd import *
from rawdata import *
from ROOT import Belle2
import os.path
import sys

fileIN = sys.argv[1]
dirOUT = sys.argv[2]

# setup database
reset_database()
use_database_chain()
use_central_database("data_reprocessing_prod5", LogLevel.WARNING)
use_central_database("svdonly_phase2analysis_with_master", LogLevel.WARNING)

main = create_path()
main.add_module('RootInput', inputFileName=str(fileIN))
main.add_module("Gearbox")
main.add_module('Geometry', useDB=True)

main.add_module('SVDChargeSharingAnalysis', outputDirName=str(dirOUT), outputRootFileName='test.root',
                useTrackInfo=True, is2017TBanalysis=False)
main.add_module('Progress')
print_path(main)
process(main)
print(statistics)
