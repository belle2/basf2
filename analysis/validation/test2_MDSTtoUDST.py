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

rootFileName = '../MDSTtoUDST'
logFileName = rootFileName + '.log'
sys.stdout = open(logFileName, 'w')

import os
from basf2 import *
from modularAnalysis import *

main = create_path()
input = register_module('RootInput')
input.param('inputFileName', '../DSTtoMDST.mdst.root')
main.add_module(input)
outputUdst('../MDSTtoUDST.udst.root', path=main)

process(main)

# Print call statistics
print(statistics)
