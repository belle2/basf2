#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# The example of running grl and gdl of L1 trigger, conjunction with tsim_subdetectors.py
# basf2 tsim_grl_gdl.py -i inputrootfile -o outputrootfile
# the inputrootfile should be the output file of tsim_subdetectors.py

import os
from basf2 import *
from L1trigger import *
import glob

main = create_path()
main.add_module('RootInput')
# add trigger simlation of sub detectors (CDC, ECL, and bKLM are included currently)
add_grl_gdl_tsim(main)

# output
main.add_module('RootOutput')


# main
process(main)
print(statistics)
# ===<END>
