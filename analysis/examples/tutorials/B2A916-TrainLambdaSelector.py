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
# This tutorial demonstrates how to train weight file for LambdaSelector #
# Data sample for this training is created by                            #
# B2A916-CreateTrainData_Lambda.py                                       #
#                                                                        #
##########################################################################

import basf2 as b2
import basf2_mva
import LambdaSelector as LambdaSelector

# Train weight file for V0 Selection.
# name of output file is specified by the argument 'mva_identifier'.
LambdaSelector.V0Selector_Training(
    train_data=b2.find_file('LambdaSelector_train_V0Selector.root', 'examples', False),
    tree_name="tree",
    mva_identifier="MVA_LGBM_V0Selector.root",
    target_variable="isSignal"
)

# Train weight file for V0 Selection.
# name of output file is specified by the argument 'mva_identifier'.
LambdaSelector.KsVeto_Training(
    train_data=b2.find_file('LambdaSelector_train_KsVeto.root', 'examples', False),
    tree_name="tree",
    mva_identifier="MVA_LGBM_KsVeto.root",
    target_variable="isSignal"
)

# apply mva to the training data
basf2_mva.expert(basf2_mva.vector('MVA_LGBM_V0Selector.root'),
                 basf2_mva.vector(b2.find_file('LambdaSelector_train_V0Selector.root', 'examples', False)),
                 'tree', 'MVAExpert_V0Selector.root')
basf2_mva.expert(basf2_mva.vector('MVA_LGBM_KsVeto.root'),
                 basf2_mva.vector(b2.find_file('LambdaSelector_train_KsVeto.root', 'examples', False)),
                 'tree', 'MVAExpert_KsVeto.root')
