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
use_central_database("data_reprocessing_prompt_bucket6")
use_central_database("svd_onlySVDinGeoConfiguration")

main = create_path()

set_random_seed(1)

main.add_module(
    'RootInput',
    inputFileName="~/svd/BII-3221-svd-hot-strip-masking/buc6/r03844/all/cdst/sub00/cdst.physics.0007.03844.HLT*",
    branchNames=['SVDShaperDigits'])

main.add_module('Gearbox')
main.add_module('Geometry')

# default parameters
main.add_module('SVDHotStripFinder', outputFileName=outputfile, searchBase=0, threshold=4.0)

main.add_module('Progress')

print_path(main)

process(main)

print(statistics)
