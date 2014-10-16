#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <input>GenericB_GENSIMRECtoDST.dst.root</input>
  <output>DSTtoMDST.mdst.root</output>
  <contact>Luis Pesantez; pesantez@uni-bonn.de</contact>
</header>
"""

import sys
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
print statistics
