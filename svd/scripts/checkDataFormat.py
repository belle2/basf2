#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import basf2
from basf2 import*
import ROOT
from ROOT import Belle2
import os.path
import sys
from basf2 import conditions as b2conditions

##################################################################################
#
# SVD Data Format Check
#
# this script
# usage: basf2 checkDataFormat.py -i INPUT_FILE
###################################################################################


print('***')
print('*** this is the data format check script used:')
with open(sys.argv[0], 'r') as fin:
    print(fin.read(), end="")
print('*** end of the script')
print('***')

b2conditions.prepend_globaltag("svd_basic")

# Create path 1
main = create_path()

main.add_module('RootInput', branchNames=['RawSVDs'])

main.add_module("Gearbox")
main.add_module('Geometry')

# unpack SVD raw data
main.add_module('SVDUnpacker')

main.add_module('SVDDataFormatCheck', ShaperDigits='SVDShaperDigits')

main.add_module('Progress')

print_path(main)
process(main)

print(statistics)
