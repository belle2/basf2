#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <output>BtoD0h_Kspipipi0_Validation.root</output>
    <contact>yi.zhang2@desy.de</contact>
</header>
"""

# NOTE: This file is auto-generated by b2skim-generate-validation.
#       Do not make changes here, as your changes may be overwritten.
#       Instead, make changes to the validation_histograms method of
#       BtoD0h_Kspipipi0.

import basf2 as b2
import modularAnalysis as ma
from skim.btocharm import BtoD0h_Kspipipi0

path = b2.Path()
skim = BtoD0h_Kspipipi0(validation=True, udstOutput=False)

ma.inputMdstList(
    "default",
    b2.find_file(skim.validation_sample, data_type="validation"),
    path=path,
)
skim(path)

b2.process(path)