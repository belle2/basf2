#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

from basf2 import *
from b2biiConversion import convertBelleMdstToBelleIIMdst, setupB2BIIDatabase

if len(sys.argv) != 3:
    sys.exit('Must provide two input parameters: [mc|data] [input_Belle_MDST_file].\n'
             'A small example Belle MDST file can be downloaded from '
             'http://www-f9.ijs.si/~zupanc/evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst')

mc_or_data = sys.argv[1].lower()
isMC = {"mc": True, "data": False}.get(mc_or_data, None)
if isMC is None:
    sys.exit('First parameter must be "mc" or "data" to indicate whether we run on MC or real data')

setupB2BIIDatabase(isMC)

inputBelleMDSTFile = sys.argv[2]

main = create_path()

# add all modules necessary to read and convert the mdst file
convertBelleMdstToBelleIIMdst(inputBelleMDSTFile, applyHadronBJSkim=True, path=main)

# Print out the contents of the DataStore
dump = register_module('PrintCollections')
main.add_module(dump)

# progress
progress = register_module('Progress')
main.add_module(progress)

process(main)

# Print call statistics
print(statistics)
