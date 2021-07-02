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
# Stuck? Ask for help at questions.belle2.org                            #
#                                                                        #
# This tutorial runs over flat NTuples of reconstructed B->KsPi0 decays, #
# which were created running B2A701. The training and test datasets      #
# consist of a mixture of reconstructed Bd->KsPi0 and qqbar MC. The      #
# apply datasets are pure signal and qqbar for running the expert as     #
# explained below.                                                       #
#                                                                        #
# Usage:                                                                 #
#   basf2 B2A702-ContinuumSuppression_MVATrain.py                        #
#                                                                        #
##########################################################################

import basf2 as b2
import basf2_mva
import subprocess

if __name__ == "__main__":

    # Note that the target variable 'isNotContinuum' needs to be
    # saved in your train.root and test.root files, along with the
    # trainingVars, listed again here (see B2A701).
    import os
    if not os.getenv('BELLE2_EXAMPLES_DATA_DIR'):
        b2.B2FATAL("You need the example data installed. Run `b2install-data example` in terminal for it.")

    # Use this path to run over Bd_KsPi0 reconstructed signal and qqbar skims.
    path = os.getenv('BELLE2_EXAMPLES_DATA_DIR')+'/mva/'

    train_data = path + 'train.root'
    test_data = path + 'test.root'
    apply_signal_data = path + 'apply_signal.root'
    apply_qqbar_data = path + 'apply_qqbar.root'

    # Define the variables for training.
    #  For details, please see the Continuum suppression section at https://software.belle2.org
    #  Note that KSFWVariables takes the optional additional argument FS1, to return the variables calculated from the
    #  signal-B final state particles.
    #  CleoCone also takes the optional additional argument ROE, to return the cones calculated from ROE particles only.
    trainVars = [
        'R2',
        'thrustBm',
        'thrustOm',
        'cosTBTO',
        'cosTBz',
        'KSFWVariables(et)',
        'KSFWVariables(mm2)',
        'KSFWVariables(hso00)',
        'KSFWVariables(hso02)',
        'KSFWVariables(hso04)',
        'KSFWVariables(hso10)',
        'KSFWVariables(hso12)',
        'KSFWVariables(hso14)',
        'KSFWVariables(hso20)',
        'KSFWVariables(hso22)',
        'KSFWVariables(hso24)',
        'KSFWVariables(hoo0)',
        'KSFWVariables(hoo1)',
        'KSFWVariables(hoo2)',
        'KSFWVariables(hoo3)',
        'KSFWVariables(hoo4)',
        'CleoConeCS(1)',
        'CleoConeCS(2)',
        'CleoConeCS(3)',
        'CleoConeCS(4)',
        'CleoConeCS(5)',
        'CleoConeCS(6)',
        'CleoConeCS(7)',
        'CleoConeCS(8)',
        'CleoConeCS(9)'
    ]

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector(train_data)
    general_options.m_treename = "tree"
    general_options.m_identifier = "MVAFastBDT.root"
    general_options.m_variables = basf2_mva.vector(*trainVars)
    general_options.m_target_variable = "isNotContinuumEvent"
    fastbdt_options = basf2_mva.FastBDTOptions()

    # Train a MVA method and store the weightfile (MVAFastBDT.root) locally.
    basf2_mva.teacher(general_options, fastbdt_options)

    # Evaluate training.
    subprocess.call('basf2_mva_evaluate.py '
                    ' -train ' + train_data +
                    ' -data ' + test_data +
                    ' -id ' + 'MVAFastBDT.root' +
                    ' --output qqbarSuppressionEvaluation.pdf',
                    shell=True
                    )

    # If you're only interested in the network output distribution, then
    # comment these in to apply the trained methods on an independent sample
    # (produced in B2A701 if you ran with the `apply_signal` and `apply_qqbar` options).
    #
    basf2_mva.expert(basf2_mva.vector('MVAFastBDT.root'), basf2_mva.vector(apply_signal_data), 'tree', 'MVAExpert_signal.root')
    basf2_mva.expert(basf2_mva.vector('MVAFastBDT.root'), basf2_mva.vector(apply_qqbar_data), 'tree', 'MVAExpert_qqbar.root')
