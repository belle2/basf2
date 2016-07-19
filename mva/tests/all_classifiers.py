#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

import basf2_mva

import os
import tempfile
import subprocess
import sys


if __name__ == "__main__":

    # Skip test if files are not available
    if not (os.path.isfile('train.root') and os.path.isfile('test.root')):
        sys.exit(0)

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector('p', 'pz', 'daughter(0, Kid)')
    general_options.m_target_variable = "isSignal"

    methods = [
            ('Trivial.xml', basf2_mva.TrivialOptions()),
            ('FastBDT.xml', basf2_mva.FastBDTOptions()),
            ('TMVAClassification.xml', basf2_mva.TMVAOptionsClassification()),
            # ('FANN.xml', basf2_mva.FANNOptions()),
            ]

    olddir = os.getcwd()
    tempdir = tempfile.mkdtemp()
    os.symlink(os.path.abspath('train.root'), tempdir + '/' + os.path.basename('train.root'))
    os.symlink(os.path.abspath('test.root'), tempdir + '/' + os.path.basename('test.root'))
    os.chdir(tempdir)

    for identifier, specific_options in methods:
        general_options.m_weightfile = identifier
        basf2_mva.teacher(general_options, specific_options)

    basf2_mva.expert(basf2_mva.vector(*[i for i, _ in methods]),
                     basf2_mva.vector('train.root'), 'tree', 'expert.root')

    subprocess.call('basf2_mva_evaluate.py -train train.root -test test.root -i ' + ' '.join([i for i, _ in methods]), shell=True)
