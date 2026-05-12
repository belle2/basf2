#! /usr/bin/env python3
# Script name is hardcoded on SC GUI and can not be changed

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from hlt.hlt_execution import main

if __name__ == '__main__':
    exit(main("beam_reco_sel.py"))
