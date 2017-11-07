#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

import basf2_mva

import os
import tempfile
import subprocess
import sys
import shutil

variables = ['p', 'pz', 'daughter(0, p)', 'daughter(0, pz)', 'daughter(1, p)', 'daughter(1, pz)',
             'chiProb', 'dr', 'dz', 'daughter(0, dr)', 'daughter(1, dr)', 'daughter(0, chiProb)', 'daughter(1, chiProb)',
             'daughter(0, kaonID)', 'daughter(0, pionID)', 'daughterAngle(0, 1)']

if __name__ == "__main__":

    # Skip test if files are not available
    if not (os.path.isfile('train.root') and os.path.isfile('test.root')):
        sys.exit(0)

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    methods = [
        ('Trivial.xml', basf2_mva.TrivialOptions()),
        ('Python.xml', basf2_mva.PythonOptions()),
        ('FastBDT.xml', basf2_mva.FastBDTOptions()),
        ('TMVAClassification.xml', basf2_mva.TMVAOptionsClassification()),
        ('FANN.xml', basf2_mva.FANNOptions()),
    ]

    olddir = os.getcwd()
    with tempfile.TemporaryDirectory() as tempdir:
        os.symlink(os.path.abspath('train.root'), tempdir + '/' + os.path.basename('train.root'))
        os.symlink(os.path.abspath('test.root'), tempdir + '/' + os.path.basename('test.root'))
        os.chdir(tempdir)

        for identifier, specific_options in methods:
            general_options.m_identifier = identifier
            basf2_mva.teacher(general_options, specific_options)

        basf2_mva.expert(basf2_mva.vector(*[i for i, _ in methods]),
                         basf2_mva.vector('train.root'), 'tree', 'expert.root')

        subprocess.call('basf2_mva_evaluate.py -o latex.pdf -train train.root -data test.root -i ' +
                        ' '.join([i for i, _ in methods]), shell=True)

        os.chdir(olddir)
        shutil.copyfile(tempdir + '/latex.pdf', 'latex.pdf')
