#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2_mva
import b2test_utils
import basf2

variables = ['M', 'p', 'pt', 'pz',
             'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)',
             'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)',
             'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)',
             'chiProb', 'dr', 'dz',
             'daughter(0, dr)', 'daughter(1, dr)',
             'daughter(0, dz)', 'daughter(1, dz)',
             'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)',
             'daughter(0, kaonID)', 'daughter(0, pionID)',
             'daughterInvM(0, 1)', 'daughterInvM(0, 2)', 'daughterInvM(1, 2)']


def feature_importance(state):
    """
    Return a list containing the feature importances
    """
    print("Called overwritten feature importance")
    return []


def load(obj):
    """
    Test load function
    """
    print("Called overwritten load")
    print(obj)
    return None


print("Executed python script")

if __name__ == "__main__":

    # Skip test if files are not available
    try:
        train_file = basf2.find_file('mva/train_D0toKpipi.root', 'examples', False)
        test_file = basf2.find_file('mva/test_D0toKpipi.root', 'examples', False)
    except BaseException:
        b2test_utils.skip_test('Necessary files "train.root" and "test.root" not available.')

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector(train_file)
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"
    general_options.m_identifier = "Python.xml"
    general_options.m_max_events = 200

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_training_fraction = 0.9
    specific_options.m_nIterations = 2
    specific_options.m_mini_batch_size = 10000
    specific_options.m_framework = 'test'

    # we create payloads so let's switch to an empty, temporary directory
    with b2test_utils.clean_working_directory():

        basf2_mva.teacher(general_options, specific_options)

        basf2_mva.expert(basf2_mva.vector("Python.xml"),
                         basf2_mva.vector(test_file), 'tree', 'expert.root')

        specific_options.m_steering_file = 'mva/tests/python.py'
        basf2_mva.teacher(general_options, specific_options)

        basf2_mva.expert(basf2_mva.vector("Python.xml"),
                         basf2_mva.vector(test_file), 'tree', 'expert.root')
