#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2_mva
import basf2_mva_util
from basf2_mva_evaluation import plotting
import argparse
import tempfile

import numpy as np
from B2Tools import b2latex, format

import os
import shutil
import sys
import hashlib


def getCommandLineOptions():
    """ Parses the command line options of the fei and returns the corresponding arguments. """
    parser = argparse.ArgumentParser()
    parser.add_argument('-id', '--identifiers', dest='identifiers', type=str, required=True, action='append', nargs='+',
                        help='DB Identifier or weightfile')
    parser.add_argument('-train', '--train_datafiles', dest='train_datafiles', type=str, required=True, action='append', nargs='+',
                        help='Data file containing ROOT TTree used during training')
    parser.add_argument('-test', '--test_datafiles', dest='test_datafiles', type=str, required=True, action='append', nargs='+',
                        help='Data file containing ROOT TTree with independent test data')
    parser.add_argument('-tree', '--treename', dest='treename', type=str, default='tree', help='Treename in data file')
    parser.add_argument('-out', '--outputfile', dest='outputfile', type=str, default='output.pdf',
                        help='Name of the outputted pdf file')
    parser.add_argument('-iterative', '--iterative', dest='iterative', action='store_true',
                        help='Improve the importance estimation by iteratively'
                             'remove the most important variable. Needs O(NFeatures**2) Trainings!')
    args = parser.parse_args()
    return args


def get_importances(method, train_datfiles, test_datafiles, treename, variables, global_auc):
    importances = {}
    for variable in variables:
        general_options = method.general_options
        general_options.m_variables = basf2_mva.vector(*[v for v in variables if v != variable])
        m = method.train_teacher(train_datafiles, treename, general_options)
        auc = basf2_mva_util.calculate_roc_auc(*m.apply_expert(test_datafiles, treename))
        importances[variable] = global_auc - auc
    return importances


def get_importances_iterative(method, train_datfiles, test_datafiles, treename, variables, global_auc):
    imp = get_importances(method, train_datfiles, test_datafiles, treename, variables, global_auc)
    most_important = max(imp.keys(), key=lambda x: imp[x])
    print(most_important, imp[most_important])

    remaining_variables = [v for v in variables if v != most_important]
    if len(remaining_variables) == 1:
        return imp

    importances = {most_important: imp[most_important]}
    rest = get_importances_iterative(method, train_datfiles, test_datafiles, treename, remaining_variables, global_auc)
    importances.update(rest)
    return importances


if __name__ == '__main__':

    old_cwd = os.getcwd()
    args = getCommandLineOptions()

    identifiers = sum(args.identifiers, [])
    train_datafiles = sum(args.train_datafiles, [])
    test_datafiles = sum(args.test_datafiles, [])

    methods = [basf2_mva_util.Method(identifier) for identifier in identifiers]

    importances = []
    all_variables = []
    for method in methods:
        global_auc = basf2_mva_util.calculate_roc_auc(*method.apply_expert(test_datafiles, args.treename))
        for variable in method.variables:
            all_variables.append(variable)
        if args.iterative:
            imp = get_importances_iterative(method, train_datafiles, test_datafiles, args.treename, method.variables, global_auc)
            for key in imp.keys():
                imp[key] = global_auc - imp[key]
        else:
            imp = get_importances(method, train_datafiles, test_datafiles, args.treename, method.variables, global_auc)
        importances.append(imp)

    all_variables = list(sorted(all_variables, key=lambda v: importances[0].get(v, 0.0)))

    importances_dict = {}
    for i, method in enumerate(methods):
        importances_dict[method.identifier] = np.array([importances[i].get(v, 0.0) for v in all_variables])

    # Change working directory after experts run, because they might want to access
    # a locadb in the current working directory
    tempdir = tempfile.mkdtemp()
    os.chdir(tempdir)

    o = b2latex.LatexFile()
    o += b2latex.TitlePage(title='Automatic Feature Importance Report',
                           authors=[r'Thomas Keck'],
                           abstract='Feature importance calculated by leaving one variable out and retrain',
                           add_table_of_contents=False,
                           clearpage=False).finish()

    graphics = b2latex.Graphics()
    p = plotting.Importance()
    p.add(importances_dict, identifiers, all_variables)
    p.finish()
    p.save('importance.png')
    graphics.add('importance.png', width=1.0)
    o += graphics.finish()

    o.save('latex.tex', compile=True)
    os.chdir(old_cwd)
    shutil.copy(tempdir + '/latex.pdf', args.outputfile)
