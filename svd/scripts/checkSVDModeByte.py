#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import basf2
from basf2 import*
from svd import add_svd_reconstruction
from svd.skim_utils import skimOutRNDTrgModule
import ROOT
from ROOT import Belle2
import os.path
import sys
import argparse

##################################################################################
#
# Phase3 SVDModeByte check
#
# this script
# usage: basf2 checkSVDModeByte -- --input FILE_IN
###################################################################################


# argument parser
parser = argparse.ArgumentParser(description="Phase3 SVD installed in BelleII: SVDModeByte check script")
parser.add_argument('--input', dest='input', help='Input root/sroot file')

parser.print_help()
print('')
args = parser.parse_args()

fileIN = args.input

if not os.path.isfile(fileIN):
    print('')
    print('!! ERROR !!')
    print('the input file ' + str(fileIN) + ' does not exist, check and retry.')
    print('')
    sys.exit(1)

print('***')
print('*** this is the unpacking script used:')
with open(sys.argv[0], 'r') as fin:
    print(fin.read(), end="")
print('*** end of unpacking script')
print('***')


# setup database
reset_database()
# phase2
use_central_database("data_reprocessing_proc8")
# phase3
# use_database_chain()
# use_central_database("svd_basic_20190213")
# use_central_database("svd_loadedOnFADC_20190227")
# use_central_database("svd_offlineCalibrations_20190307")

# Create path 1
main = create_path()

# main.add_module('SeqRootInput', inputFileNames=[fileIN])
# main.add_module('RootInput', inputFileNames=[fileIN],branchNames=['RawSVDs'])
main.add_module('RootInput', inputFileNames=[fileIN], branchNames=['SVDShaperDigits'])

main.add_module("Gearbox")
main.add_module('Geometry')

# unpack SVD raw data
# main.add_module('SVDUnpacker', svdShaperDigitListName='SVDShaperDigitsToFilter')
# main.add_module('SVDDataFormatCheck',ShaperDigits='SVDShaperDigitsToFilter')

main.add_module('SVDDataFormatCheck', ShaperDigits='SVDShaperDigits')

main.add_module('Progress')

print_path(main)
process(main)
print(statistics)
