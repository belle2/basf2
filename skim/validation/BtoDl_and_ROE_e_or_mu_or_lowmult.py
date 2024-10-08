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
    <output>BtoDl_and_ROE_e_or_mu_or_lowmult_Validation.root</output>
    <contact>shanette.delamotte@adelaide.edu.au</contact>
</header>
"""

# NOTE: This file is auto-generated by b2skim-generate-validation.
#       Do not make changes here, as your changes may be overwritten.
#       Instead, make changes to the validation_histograms method of
#       BtoDl_and_ROE_e_or_mu_or_lowmult.

import basf2 as b2
import modularAnalysis as ma
from skim.WGs.semileptonic import BtoDl_and_ROE_e_or_mu_or_lowmult

path = b2.Path()
skim = BtoDl_and_ROE_e_or_mu_or_lowmult(
    validation=True,
    udstOutput=False,
)

ma.inputMdstList(
    b2.find_file(skim.validation_sample, data_type="validation"),
    path=path,
)
skim(path)

path.add_module('Progress')
b2.process(path)
