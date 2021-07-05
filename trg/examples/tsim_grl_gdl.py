#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# The example of running grl and gdl of L1 trigger, conjunction with tsim_subdetectors.py
# basf2 tsim_grl_gdl.py -i inputrootfile -o outputrootfile
# the inputrootfile should be the output file of tsim_subdetectors.py

import basf2 as b2
from L1trigger import add_grl_gdl_tsim

main = b2.create_path()
main.add_module('RootInput')
# add trigger simlation of sub detectors (CDC, ECL, and bKLM are included currently)
add_grl_gdl_tsim(main)

# output
main.add_module('RootOutput')


# main
b2.process(main)
print(b2.statistics)
# ===<END>
