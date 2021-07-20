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
  <input>../GenericB_GENSIMRECtoDST.dst.root</input>
  <output>../DSTtoMDST.mdst.root</output>
  <contact>Frank Meier; frank.meier@duke.edu</contact>
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
