#!/bin/bash

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

b2code-memoryusage -m plot --types=vps --no-legend -p ${OUTPUT_DIR}/beam_reco_monitor-hlt-memory.npz -o ${OUTPUT_DIR}/beam_reco_monitor-hlt-memory.pdf
