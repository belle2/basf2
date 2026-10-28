#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Write out the location and direction of all ECL crystals. Uses information
from the payload ECLCrystalsPositionAndShape for the experiment and run of
set via EventInfoSetter
"""

import basf2 as b2
from ecl_utils import ECLDumpGeometry

main = b2.Path()

main.add_module('EventInfoSetter')
main.add_module('Gearbox')
main.add_module('Geometry')
main.add_module(ECLDumpGeometry())

b2.process(main, 1)
