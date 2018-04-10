#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<output>ECLEvtGenOutput.root</output>
<contact>Benjamin Oberhof, ecl2ml@bpost.kek.jp</contact>
</header>
"""

import os
import glob
from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

# Create paths
main = create_path()

# random number for generation
# import random
# intseed = random.randint(1, 10000000)

# Fixed random seed
set_random_seed(123456)

inputFile = '../EvtGenSimRec.root'
main.add_module('RootInput', inputFileName=inputFile)

# eclDataAnalysis module
ecldataanalysis = register_module('ECLDataAnalysis')
ecldataanalysis.param('rootFileName', '../ECLEvtGenOutput.root')
ecldataanalysis.param('doTracking', 1)
ecldataanalysis.param('doSimulation', 0)
main.add_module(ecldataanalysis)

process(main)
# print(statistics)
