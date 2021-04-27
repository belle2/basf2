#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
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
main = b2.create_path()

main.add_module('RootInput', branchNames=['RawSVDs'])

main.add_module("Gearbox")
main.add_module('Geometry')

# unpack SVD raw data
main.add_module('SVDUnpacker')

main.add_module('SVDDataFormatCheck', ShaperDigits='SVDShaperDigits')

main.add_module('Progress')

b2.print_path(main)
b2.process(main)

print(b2.statistics)
