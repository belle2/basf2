#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import basf2
from b2biiConversion import convertBelleMdstToBelleIIMdst

os.environ['PGUSER'] = 'g0db'

main = basf2.create_path()

# add all modules necessary to read and convert the mdst file
inputfile = basf2.find_file('b2bii_input_evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst', 'examples', False)
convertBelleMdstToBelleIIMdst(inputfile, applySkim=True, path=main)

# Store the converted Belle II dataobjects in ROOT
output = basf2.register_module('RootOutput')
output.param('outputFileName', 'B2BII_Convert_Example.mdst.root')
main.add_module(output)

# progress
main.add_module('Progress')

basf2.process(main)

# Print call statistics
print(basf2.statistics)
