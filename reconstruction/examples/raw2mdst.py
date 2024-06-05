#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from mdst import add_mdst_output
from softwaretrigger.constants import SoftwareTriggerModes
from softwaretrigger.processing import add_hlt_processing

# create path
main = b2.create_path()

# input
main.add_module('RootInput')

# gearbox and geometry
main.add_module('Gearbox')
main.add_module('Geometry')

# reconstruction
# No filter is applied in monitoring mode. To enable filtering, use, for example,
# softwaretrigger_mode=SoftwareTriggerModes.filter
add_hlt_processing(main, softwaretrigger_mode=SoftwareTriggerModes.monitor, prune_output=False, calcROIs=False)

# mdst output
add_mdst_output(main)

# process events and print call statistics
b2.process(main)
print(b2.statistics)
