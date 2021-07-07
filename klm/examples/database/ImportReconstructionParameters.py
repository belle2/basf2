#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import KLM scintillator digitization parameters payloads.

import sys
import basf2
import math
from ROOT.Belle2 import KLMDatabaseImporter, KLMReconstructionParameters

basf2.set_log_level(basf2.LogLevel.INFO)

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True

dbImporter = KLMDatabaseImporter()

recPar = KLMReconstructionParameters()
recPar.setMultiStripDigitsEnabled(False)

if (mc):
    dbImporter.setIOV(1002, 0, 1002, -1)
    dbImporter.importReconstructionParameters(recPar)

    dbImporter.setIOV(1003, 0, 1003, -1)
    dbImporter.importReconstructionParameters(recPar)

    dbImporter.setIOV(0, 0, 0, -1)
    dbImporter.importReconstructionParameters(recPar)

else:
    dbImporter.setIOV(0, 0, 15, -1)
    dbImporter.importReconstructionParameters(recPar)

    recPar.setMultiStripDigitsEnabled(True)
    dbImporter.setIOV(16, 0, -1, -1)
    dbImporter.importReconstructionParameters(recPar)
