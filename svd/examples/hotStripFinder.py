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
outputfile = "SVDHotStripFinderZS5_exp8run2265.root"

use_database_chain()
use_central_database("data_reprocessing_prompt_bucket6")
use_central_database("svd_basic")
use_central_database("svd_onlySVDinGeoConfiguration")

main = create_path()

set_random_seed(1)

main.add_module(
    'RootInput',
    #    inputFileName="~/svd/BII-3221-svd-hot-strip-masking/buc6/r03844/all/cdst/sub00/cdst.physics.0007.03844.HLT*",
    inputFileName="/ghi/fs01/belle2/bdata/Data/Raw/e0008/r02265/sub00/physics.0008.02265.HLT*",
    # luminosity run 2265, exp8, raw data:add unpacker
    # inputFileName="/ghi/fs01/belle2/bdata/Data/Raw/e0008/r02308/sub00/cosmic.0008.02308.HLT*",
    # # cosmic run 2308, exp8, raw data:add unpacker
    branchNames=['SVDShaperDigits'])

main.add_module('Gearbox')
main.add_module('Geometry')

# svd reconstruction, excluding SP creator
main.add_module('SVDUnpacker', svdShaperDigitListName='SVDShaperDigits')
main.add_module(
    'SVDZeroSuppressionEmulator',
    SNthreshold=5,
    ShaperDigits='SVDShaperDigits',
    ShaperDigitsIN='SVDShaperDigitsZS5',
    FADCmode=True)

# default parameters
main.add_module('SVDHotStripFinder', ShaperDigits='SVDShaperDigitsZS5', outputFileName=outputfile, searchBase=0,
                threshold=1.0, absOccThreshold=0.20, relOccPrec=5, firstExp=8, firstRun=2265, useHSFinderV1=True)

main.add_module('Progress')

print_path(main)

process(main)

print(statistics)
