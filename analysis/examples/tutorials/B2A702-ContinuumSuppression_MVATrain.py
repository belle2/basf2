#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial runs over flat NTuples of reconstructed B->KsPi0 decays,
# which were created running B2A701. The training and test datasets consist of
# a mixture of reconstructed Bd->KsPi0 and qqbqr MC. The apply datasets are
# pure signal and qqbar for running the expert as explained below.
#
# Usage:
#   basf2 B2A702-ContinuumSuppression_MVATrain.py
#
# Contributors: P. Goldenzweig (October 2016)
#
################################################################################

import basf2_mva
import subprocess

if __name__ == "__main__":

    # Note that the target variable 'isNotContinuum' needs to be
    # saved in your train.root and test.root files, along with the
    # trainingVars, listed again here (see B2A701).

    # Use this path to run over Bd_KsPi0 reconstructed signal and qqbar skims.
    path = '/group/belle2/tutorial/release_01-00-00/inputForCSTutorial/'

    train_data = path + 'train.root'
    test_data = path + 'test.root'
    apply_signal_data = path + 'apply_signal.root'
    apply_qqbar_data = path + 'apply_qqbar.root'

    # Define the variables for training.
    #  For details, please see: https://confluence.desy.de/display/BI/Continuum+Suppression+Framework
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
        'CleoCone(1)',
        'CleoCone(2)',
        'CleoCone(3)',
        'CleoCone(4)',
        'CleoCone(5)',
        'CleoCone(6)',
        'CleoCone(7)',
        'CleoCone(8)',
        'CleoCone(9)'
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
    # basf2_mva.expert(basf2_mva.vector('MVAFastBDT.root'), basf2_mva.vector(apply_signal_data), 'tree', 'MVAExpert_signal.root')
    # basf2_mva.expert(basf2_mva.vector('MVAFastBDT.root'), basf2_mva.vector(apply_qqbar_data), 'tree', 'MVAExpert_qqbar.root')
