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

# set here the correct exp and run number you like to process
exp = 8
run = 2308  # 2265
runTpe = ""

if run == 2308:
    runType = "cosmic"
elif run == 2265:
    runType = "physics"
else:
    print("Check the run number!!")

# set this string to identify the output rootfiles
outputfile = "SVDHotStripFinderZS5_exp" + str(exp) + "run" + str(run) + "_V2.root"

use_database_chain()
use_central_database("data_reprocessing_prompt_bucket6")
# use_central_database("svd_basic")
use_local_database("/home/belle2/casarosa/master/hotStrips/centraldb/database_ok.txt",
                   "/home/belle2/casarosa/master/hotStrips/centraldb")
use_central_database("svd_onlySVDinGeoConfiguration")

use_local_database("localDB_HSF/database.txt", "localDB_HSF")

main = create_path()

set_random_seed(1)

main.add_module(
    'RootInput',
    #    inputFileName="~/svd/BII-3221-svd-hot-strip-masking/buc6/r03844/all/cdst/sub00/cdst.physics.0007.03844.HLT*",
    # luminosity run 2265, exp8, raw data
    # cosmic run 2308, exp8, raw data
    inputFileName="/ghi/fs01/belle2/bdata/Data/Raw/e000" + \
    str(exp) + "/r0" + str(run) + "/sub00/" + str(runType) + ".000" + str(exp) + ".0" + str(run) + ".HLT1.f00000.root",
    branchNames=['RawSVDs'])

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
                threshold=1.0, absOccThreshold=0.20, relOccPrec=5, useHSFinderV1=False)

main.add_module('Progress')

print_path(main)

process(main)

print(statistics)
