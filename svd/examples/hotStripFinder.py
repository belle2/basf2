#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import *
from svd import *
import glob

#########
#
# SVD HOT STRIP FINDER SCRIPT
#
# usage
# > basf2 hotStripFinder.py -i INPUT FILE
#
#######

# set this string to identify the output rootfiles
outputfile = "SVDHotStripFinder.root"

use_database_chain()
use_central_database("data_reprocessing_prompt")
use_central_database("svd_basic_20190213")
# use_central_database("svd_loadedOnFADC_20190227")
# use_central_database("svd_offlineCalibrations_20190307")
use_central_database("svd_onlySVDinGeoConfiguration_20190427")

main = create_path()

set_random_seed(1)

main.add_module('RootInput', branchNames=['SVDShaperDigits'])

main.add_module('Gearbox')
main.add_module('Geometry')

# default parameters
main.add_module('SVDHotStripFinder', outputFileName=outputfile, searchBase=2, threshold=0.75)

main.add_module('Progress')

print_path(main)

process(main)

print(statistics)
