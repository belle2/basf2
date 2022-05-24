#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# This tutorial demonstrates how to train weight file for KsSelector     #
# Data sample for this trainig is created by                             #
# B2A911-CreateTrainData.py                                              #
#                                                                        #
##########################################################################

import basf2_mva
import ksSelector as ksSelector

# Train weight file for V0 Selection.
# name of output file is specified by the argument 'mva_identifier'.
ksSelector.V0Selector_Training(
    train_data="KsSelector_train.root",
    tree_name="V0Selector",
    mva_identifier="MVAFastBDT_V0Selector.root",
    target_variable="isSignal"
)

# Train weight file for V0 Selection.
# name of output file is specified by the argument 'mva_identifier'.
ksSelector.V0Selector_Training(
    train_data="KsSelector_train.root",
    tree_name="LambdaVeto",
    mva_identifier="MVAFastBDT_LambdaVeto.root",
    target_variable="isSignal"
)

# apply mva to the training data
basf2_mva.expert(basf2_mva.vector('MVAFastBDT_V0Selector.root'), basf2_mva.vector(
    "KsSelector_train.root"), 'V0Selector', 'MVAExpert_V0Selector.root')
basf2_mva.expert(basf2_mva.vector('MVAFastBDT_LambdaVeto.root'), basf2_mva.vector(
    "KsSelector_train.root"), 'LambdaVeto', 'MVAExpert_LambdaVeto.root')
