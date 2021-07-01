#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Doxygen should skip this script
# @cond

"""
This script loads specified ROOT file and prints the content of the DataStore
for each event. To be used for debugging.

Execute script with:
  $> basf2 printDataStore.py -i [input_ROOT_file]
"""

import basf2
import modularAnalysis as ma

path = basf2.Path()
ma.inputMdst(environmentType='default',
             filename=basf2.find_file('mdst14.root', 'validation', True),
             path=path)
ma.printDataStore(path=path)
basf2.process(path)

# @endcond
