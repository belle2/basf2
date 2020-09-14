#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<input>../DSTtoMDST.mdst.root</input>
<output>../MDSTtoUDST.udst.root</output>
<contact>Luis Pesantez; pesantez@uni-bonn.de</contact>
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
