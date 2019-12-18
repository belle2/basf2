#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

from basf2 import *
from b2biiConversion import convertBelleMdstToBelleIIMdst, setupB2BIIDatabase

if len(sys.argv) != 4:
    sys.exit('Must provide two input parameters: [mc|data] [input_Belle_MDST_file][output_BelleII_ROOT_file].\n'
             'A small example Belle MDST file can be downloaded from '
             'http://www-f9.ijs.si/~zupanc/evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst')

mc_or_data = sys.argv[1].lower()
isMC = {"mc": True, "data": False}.get(mc_or_data, None)
if isMC is None:
    sys.exit('First parameter must be "mc" or "data" to indicate whether we run on MC or real data')

inputBelleMDSTFile = sys.argv[2]
outputBelle2ROOTFile = sys.argv[3]

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
