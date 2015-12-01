#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

from basf2 import *
from b2biiConversion import convertBelleMdstToBelleIIMdst

if len(sys.argv) != 3:
    sys.exit('Must provide two input parameters: [input_Belle_MDST_file][output_BelleII_ROOT_file].\n'
             'A small example Belle MDST file can be downloaded from '
             'http://www-f9.ijs.si/~zupanc/evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst')

inputBelleMDSTFile = sys.argv[1]
outputBelle2ROOTFile = sys.argv[2]

main = create_path()

# add all modules necessary to read and convert the mdst file
convertBelleMdstToBelleIIMdst(inputBelleMDSTFile, applyHadronBJSkim=True, path=main)

# Store the converted Belle II dataobjects in ROOT
output = register_module('RootOutput')
output.param('outputFileName', outputBelle2ROOTFile)
main.add_module(output)

# progress
progress = register_module('Progress')
main.add_module(progress)

process(main)

# Print call statistics
print(statistics)
