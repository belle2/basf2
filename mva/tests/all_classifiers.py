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

import os
import tempfile
import subprocess
import shutil
from b2test_utils import skip_test

variables = ['p', 'pz', 'daughter(0, p)', 'daughter(0, pz)', 'daughter(1, p)', 'daughter(1, pz)',
             'chiProb', 'dr', 'dz', 'daughter(0, dr)', 'daughter(1, dr)', 'daughter(0, chiProb)', 'daughter(1, chiProb)',
             'daughter(0, kaonID)', 'daughter(0, pionID)', 'daughterAngle(0, 1)']

if __name__ == "__main__":

    import ROOT  # noqa

    # Skip test if files are not available
    if not (os.path.isfile('train.root') and os.path.isfile('test.root')):
        skip_test('Necessary files "train.root" and "test.root" not available.')

    general_options = ROOT.Belle2.MVA.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    methods = [
        ('Trivial.xml', ROOT.Belle2.MVA.TrivialOptions()),
        ('Python.xml', ROOT.Belle2.MVA.PythonOptions()),
        ('FastBDT.xml', ROOT.Belle2.MVA.FastBDTOptions()),
        ('TMVAClassification.xml', ROOT.Belle2.MVA.TMVAOptionsClassification()),
        ('FANN.xml', ROOT.Belle2.MVA.FANNOptions()),
    ]

    olddir = os.getcwd()
    with tempfile.TemporaryDirectory() as tempdir:
        os.symlink(os.path.abspath('train.root'), tempdir + '/' + os.path.basename('train.root'))
        os.symlink(os.path.abspath('test.root'), tempdir + '/' + os.path.basename('test.root'))
        os.chdir(tempdir)

        for identifier, specific_options in methods:
            general_options.m_identifier = identifier
            ROOT.Belle2.MVA.teacher(general_options, specific_options)

        ROOT.Belle2.MVA.expert(basf2_mva.vector(*[i for i, _ in methods]),
                               basf2_mva.vector('train.root'), 'tree', 'expert.root')

        subprocess.call('basf2_mva_evaluate.py -c -o latex.pdf -train train.root -data test.root -i ' +
                        ' '.join([i for i, _ in methods]), shell=True)

        os.chdir(olddir)
        shutil.copyfile(tempdir + '/latex.pdf', 'latex.pdf')
