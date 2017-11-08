#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

import basf2_mva

import os
import tempfile
import subprocess
import sys
import shutil

variables = ['M', 'p', 'pt', 'pz',
             'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)',
             'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)',
             'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)',
             'chiProb', 'dr', 'dz',
             'daughter(0, dr)', 'daughter(1, dr)',
             'daughter(0, dz)', 'daughter(1, dz)',
             'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)',
             'daughter(0, kaonID)', 'daughter(0, pionID)',
             'daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)', 'daughterInvariantMass(1, 2)']


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
    if not (os.path.isfile('train.root') and os.path.isfile('test.root')):
        print("TEST SKIPPED: Not runnable on build bot", file=sys.stderr)
        sys.exit(1)

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"
    general_options.m_identifier = "Python.xml"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_training_fraction = 0.9
    specific_options.m_nIterations = 2
    specific_options.m_mini_batch_size = 10000
    specific_options.m_framework = 'test'

    olddir = os.getcwd()
    with tempfile.TemporaryDirectory() as tempdir:
        os.symlink(os.path.abspath('train.root'), tempdir + '/' + os.path.basename('train.root'))
        os.symlink(os.path.abspath('test.root'), tempdir + '/' + os.path.basename('test.root'))
        os.chdir(tempdir)

        basf2_mva.teacher(general_options, specific_options)

        basf2_mva.expert(basf2_mva.vector("Python.xml"),
                         basf2_mva.vector('train.root'), 'tree', 'expert.root')

        specific_options.m_steering_file = 'mva/tests/python.py'
        basf2_mva.teacher(general_options, specific_options)

        basf2_mva.expert(basf2_mva.vector("Python.xml"),
                         basf2_mva.vector('train.root'), 'tree', 'expert.root')

        os.chdir(olddir)
