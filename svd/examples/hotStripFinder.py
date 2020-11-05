#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import *
from svd import *
import glob
from basf2 import conditions as b2conditions

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
run = 2265  # 2308  # 2265
useLauraAlg = True
searchbaseValue = -1  # default value, use all sensor strips
setLauraThreshold = 5

runType = ""
myflag = ""

if run == 2308:
    runType = "cosmic"
elif run == 2265:
    runType = "physics"
else:
    print("Check the run number!!")

if searchbaseValue != -1:
    myflag = "searchBase" + str(searchbaseValue) + "_thr" + str(setLauraThreshold)

# set this string to identify the output rootfiles
outputfile = "SVDHotStripFinderZS5_exp" + str(exp) + "run" + str(run) + "_V1_" + str(myflag) + ".root"
if useLauraAlg:
    outputfile = "SVDHotStripFinderZS5_exp" + str(exp) + "run" + str(run) + "_V2_" + str(myflag) + ".root"

b2conditions.prepend_globaltag("svd_onlySVDinGeoConfiguration")
b2conditions.prepend_globaltag("data_reprocessing_prompt")
b2conditions.prepend_globaltag("svd_basic")

main = create_path()

set_random_seed(1)

main.add_module(
    'RootInput',
    #    inputFileName="~/svd/BII-3221-svd-hot-strip-masking/buc6/r03844/all/cdst/sub00/cdst.physics.0007.03844.HLT*",
    # luminosity run 2265, exp8, raw data
    # cosmic run 2308, exp8, raw data
    inputFileName="/ghi/fs01/belle2/bdata/Data/Raw/e000" + \
    str(exp) + "/r0" + str(run) + "/sub00/" + str(runType) + ".000" + str(exp) + ".0" + str(run) + ".HLT*",
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
main.add_module('SVDHotStripFinder', ShaperDigits='SVDShaperDigitsZS5', outputFileName=outputfile, searchBase=searchbaseValue,
                threshold=1, absOccThreshold=0.20, relOccPrec=setLauraThreshold, useHSFinderV1=not useLauraAlg)

main.add_module('Progress')

print_path(main)

process(main)

print(statistics)
