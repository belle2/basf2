#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
    <output>CharmoniumPsi_Validation.root</output>
    <contact>jiasen@seu.edu.cn</contact>
</header>
"""

# NOTE: Unlike the other validation scripts this file is NOT auto-generated.
#       At the moment, it is created manually, but following the other scripts pattern.
#       A function that returns the recommended PID globaltag would be needed.

import basf2 as b2
import modularAnalysis as ma
from skim.WGs.quarkonium import CharmoniumPsi

path = b2.Path()
skim = CharmoniumPsi(
    validation=True,
    udstOutput=False,
    pidGlobaltag="chargedpidmva_rel6_v5",
)

ma.inputMdstList(
    b2.find_file(skim.validation_sample, data_type="validation"),
    path=path,
)
skim(path)

path.add_module('Progress')
b2.process(path, max_event=20000)
print(b2.statistics)
