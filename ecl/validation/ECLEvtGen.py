#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<input>EvtGenSimRec.root</input>
<output>ECLEvtGenOutput.root</output>
<contact>Elisa Manoni, elisa.manoni@pg.infn.it</contact>
</header>
"""

import os
import glob
import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction

# Create paths
main = b2.create_path()

# Fixed random seed
b2.set_random_seed(123456)

inputFile = '../EvtGenSimRec.root'
main.add_module('RootInput', inputFileName=inputFile)

# eclDataAnalysis module
ecldataanalysis = b2.register_module('ECLDataAnalysis')
ecldataanalysis.param('rootFileName', '../ECLEvtGenOutput.root')
ecldataanalysis.param('doTracking', 1)
main.add_module(ecldataanalysis)

b2.process(main)
# print(statistics)
