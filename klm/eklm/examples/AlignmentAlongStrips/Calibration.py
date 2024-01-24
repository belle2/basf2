#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# EKLM alingment along strips.

import basf2
from ROOT import Belle2
import sys

# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

gearbox = basf2.register_module('Gearbox')
gearbox.initialize()

algo = Belle2.EKLMAlignmentAlongStripsAlgorithm()
algo.setInputFileNames(sys.argv[1:])
algo.setOutputFile('alignment_along_strips.root')
algo.execute()
algo.commit()
