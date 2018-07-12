#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../GenericB_GENSIMRECtoDST.dst.root</input>
  <output>../DSTtoMDST.mdst.root</output>
  <contact>Jorge Martinez-Ortega; jmartinez@fis.cinvestav.mx</contact>
</header>
"""

import sys

rootFileName = '../DSTtoMDST'
logFileName = rootFileName + '.log'
sys.stdout = open(logFileName, 'w')

import os
from basf2 import *
from reconstruction import *

main = create_path()
input = register_module('RootInput')
input.param('inputFileName', '../GenericB_GENSIMRECtoDST.dst.root')
main.add_module(input)
add_mdst_output(main, True, '../DSTtoMDST.mdst.root')

process(main)

# Print call statistics
print(statistics)
