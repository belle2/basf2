#!/bin/bash

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Check the size of the DQM histograms produced by ExpressReco
hlt-check-dqm-size ${OUTPUT_ROOT_DIR}/beam_reco_monitor-expressreco-dqm.root
