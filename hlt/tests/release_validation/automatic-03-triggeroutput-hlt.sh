#!/bin/bash

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

b2hlt_print_result.py --local-db-path cdb ${OUTPUT_ROOT_DIR}/beam_reco_monitor-hlt-out.root
