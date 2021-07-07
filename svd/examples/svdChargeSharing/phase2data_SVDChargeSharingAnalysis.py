#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import sys
from basf2 import conditions as b2conditions

fileIN = sys.argv[1]
dirOUT = sys.argv[2]

# setup database
b2conditions.prepend_globaltag("data_reprocessing_prompt")

main = b2.create_path()
main.add_module('RootInput', inputFileName=str(fileIN))
main.add_module("Gearbox")
main.add_module('Geometry', useDB=True)

main.add_module('SVDChargeSharingAnalysis', outputDirName=str(dirOUT), outputRootFileName='test.root',
                useTrackInfo=True, is2017TBanalysis=False)
main.add_module('Progress')
b2.print_path(main)
b2.process(main)
print(b2.statistics)
