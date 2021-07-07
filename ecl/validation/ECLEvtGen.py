#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
<input>EvtGenSimRec.root</input>
<output>ECLEvtGenOutput.root</output>
<contact>Elisa Manoni, elisa.manoni@pg.infn.it</contact>
</header>
"""

import basf2 as b2

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
