#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2_mva

import numpy as np
import os
import tempfile
from b2test_utils import skip_test

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


def apply(state, X):
    """
    Test apply function
    """
    print(X.mean(axis=0))
    print(X.std(axis=0))
    p = np.zeros(len(X))
    return np.require(p, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


def begin_fit(state, Xtest, Stest, ytest, wtest):
    """
    Test begin_fit function
    """
    print(Xtest.mean(axis=0))
    print(Xtest.std(axis=0))
    return state


def partial_fit(state, X, S, y, w, epoch):
    """
    Test partial_fit function
    """
    print(X.mean(axis=0))
    print(X.std(axis=0))
    return True


print("Executed python script")

if __name__ == "__main__":

    # Skip test if files are not available
    if not (os.path.isfile('train.root') and os.path.isfile('test.root')):
        skip_test('Necessary files "train.root" and "test.root" not available.')

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"
    general_options.m_identifier = "Python.xml"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_training_fraction = 0.9
    specific_options.m_nIterations = 1
    specific_options.m_mini_batch_size = 0
    specific_options.m_framework = 'test'
    specific_options.m_steering_file = 'mva/tests/python_normalization.py'
    specific_options.m_normalize = False

    olddir = os.getcwd()
    with tempfile.TemporaryDirectory() as tempdir:
        os.symlink(os.path.abspath('train.root'), tempdir + '/' + os.path.basename('train.root'))
        os.chdir(tempdir)

        basf2_mva.teacher(general_options, specific_options)
        basf2_mva.expert(basf2_mva.vector("Python.xml"),
                         basf2_mva.vector('train.root'), 'tree', 'expert.root')
        specific_options.m_normalize = True
        basf2_mva.teacher(general_options, specific_options)
        basf2_mva.expert(basf2_mva.vector("Python.xml"),
                         basf2_mva.vector('train.root'), 'tree', 'expert.root')
        os.chdir(olddir)
