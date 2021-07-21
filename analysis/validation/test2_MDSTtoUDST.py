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
<input>../DSTtoMDST.mdst.root</input>
<output>../MDSTtoUDST.udst.root</output>
<contact>Frank Meier; frank.meier@duke.edu</contact>
</header>
"""

import sys
import basf2
from modularAnalysis import outputUdst


rootFileName = '../MDSTtoUDST'
logFileName = rootFileName + '.log'
sys.stdout = open(logFileName, 'w')

main = basf2.create_path()
input = basf2.register_module('RootInput')
input.param('inputFileName', '../DSTtoMDST.mdst.root')
main.add_module(input)
outputUdst('../MDSTtoUDST.udst.root', path=main)

basf2.process(main)

# Print call statistics
print(basf2.statistics)
