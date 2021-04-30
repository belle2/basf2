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

import basf2
from reconstruction import add_mdst_output

rootFileName = '../DSTtoMDST'
logFileName = rootFileName + '.log'
sys.stdout = open(logFileName, 'w')

main = basf2.create_path()
input = basf2.register_module('RootInput')
input.param('inputFileName', '../GenericB_GENSIMRECtoDST.dst.root')
main.add_module(input)
add_mdst_output(main, True, '../DSTtoMDST.mdst.root')

basf2.process(main)

# Print call statistics
print(basf2.statistics)
