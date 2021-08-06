#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Example showing the use of the RetentionCheck module

To run the example, simply do

    basf2 exampleOfRetentionCheck.py -n 100
"""

import basf2 as b2
import modularAnalysis as ma
from skimExpertFunctions import get_test_file
from skimRetentionCheck import RetentionCheck, pathWithRetentionCheck
from skim.fei import runFEIforB0SL

# Create a basf2 path.
path = b2.create_path()

# Input a test mdst file.
ma.inputMdstList(get_test_file("MC12_mixedBGx1"), path=path)

# Run pre-selection cuts and FEI.
runFEIforB0SL(path)
# Apply *arbitrary* cuts on the B tag list.
ma.applyCuts('B0:semileptonic', 'sigProb>0.0001', path=path)
ma.applyCuts('B0:semileptonic', '-4.0<cosThetaBY<3.0', path=path)
ma.applyCuts('B0:semileptonic', 'dmID<8', path=path)
ma.applyCuts('B0:semileptonic', '[[dmID<4 and d1_p_CMSframe>1.0] or [dmID>=4 and d2_p_CMSframe>1.0]]', path=path)

# Add the retention tracker.
path = pathWithRetentionCheck(['B0:semileptonic'], path)

# Process the basf2 path.
b2.process(path)

# Print out the retention tracker results and create an associated plot.
RetentionCheck.print_results()
RetentionCheck.plot_retention('B0:semileptonic', save_as='retention_rate.pdf')
