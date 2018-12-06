#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2_mva
import basf2_mva_util
from basf2_mva_evaluation import plotting
import argparse
import tempfile

from ROOT import Belle2
import numpy as np
from B2Tools import b2latex

import os


def getCommandLineOptions():
    """ Parses the command line options of the fei and returns the corresponding arguments. """
    parser = argparse.ArgumentParser()
    parser.add_argument('-id', '--identifiers', dest='identifiers', type=str, required=True, action='append', nargs='+',
                        help='DB Identifier or weightfile. Does at the moment only work with one id.')
    parser.add_argument('-train', '--train_datafiles', dest='train_datafiles', type=str, required=True, action='append', nargs='+',
                        help='Data file containing ROOT TTree used during training')
    parser.add_argument('-test', '--test_datafiles', dest='test_datafiles', type=str, required=True, action='append', nargs='+',
                        help='Data file containing ROOT TTree with independent test data')
    parser.add_argument('-tree', '--treename', dest='treename', type=str, default='tree', help='Treename in data file')
    parser.add_argument('-out', '--outputfile', dest='outputfile', type=str, default='output.pdf',
                        help='Name of the outputted pdf file')
    parser.add_argument('-weightfile', '--weightfile', dest='weightfile', action='store_true',
                        help='Read feature importances from weightfile')
    parser.add_argument('-iterative', '--iterative', dest='iterative', action='store_true',
                        help='Improve the importance estimation by iteratively'
                             'leaving one variable out and retrain. Needs O(NFeatures) Trainings!')
    parser.add_argument('-recursive', '--recursive', dest='recursive', action='store_true',
                        help='Improve the importance estimation by recursively'
                             'remove the most important variable. Needs O(NFeatures**2) Trainings!')
    args = parser.parse_args()
    return args


def get_importances(method, train_datafiles, test_datafiles, treename, variables, global_auc):
    """
    Calculate the importance of the variables of a method by retraning the method without
    one of the variables at a time and comparing the auc to the global_auc
    @param method the method object
    @param train_datafiles data used to retrain the method
    @param test_datafiles data used to evaluate the method and calculate the new auc
    @param treename the name of the tree containing the data
    @param variables list of variables which are considered for the trainings
    @param global_auc the auc of the training with all variables
    """
    importances = {}
    classifiers = {}
    for variable in variables:
        general_options = method.general_options
        general_options.m_variables = basf2_mva.vector(*[v for v in variables if v != variable])
        m = method.train_teacher(train_datafiles, treename, general_options)
        auc = basf2_mva_util.calculate_roc_auc(*m.apply_expert(test_datafiles, treename))
        importances[variable] = global_auc - auc
        classifiers[variable] = m
    return importances, classifiers


def get_importances_recursive(method, train_datfiles, test_datafiles, treename, variables, global_auc):
    """
    Calculate the importance of the variables of a method by retraning the method without
    one of the variables at a time. Then the best variable (the one which leads to the lowest auc
    if it is left out) is removed and the importance of the remaining variables is calculated recursively
    @param method the method object
    @param train_datafiles data used to retrain the method
    @param test_datafiles data used to evaluate the method and calculate the new auc
    @param treename the name of the tree containing the data
    @param variables list of variables which are considered for the trainings
    @param global_auc the auc of the training with all variables
    """
    imp, cla = get_importances(method, train_datfiles, test_datafiles, treename, variables, global_auc)
    most_important = max(imp.keys(), key=lambda x: imp[x])
    remaining_variables = [v for v in variables if v != most_important]

    if len(remaining_variables) == 1:
        return imp, cla

    importances = {most_important: imp[most_important]}
    classifiers = {most_important: cla[most_important]}
    rest, subcla = get_importances_recursive(method, train_datfiles, test_datafiles, treename,
                                             remaining_variables, global_auc - imp[most_important])
    importances.update(rest)
    classifiers.update(subcla)
    return importances, classifiers


if __name__ == '__main__':

    print("WARNING This tool is deprecated, use mva/examples/advanced/variable_importance.py instead and adapt it to your needs.")
    print("In fact adapting the example is easier than using this general tool, and it is also easier to automatise")
    print("Therefore this tool will be removed in the future")

    old_cwd = os.getcwd()
    args = getCommandLineOptions()

    identifiers = sum(args.identifiers, [])
    train_datafiles = sum(args.train_datafiles, [])
    test_datafiles = sum(args.test_datafiles, [])

    methods = [basf2_mva_util.Method(identifier) for identifier in identifiers]

    labels = []
    importances = []
    iterative_classifiers = []
    recursive_classifiers = []
    all_variables = []
    cla_dict = {}
    for method in methods:
        global_auc = basf2_mva_util.calculate_roc_auc(*method.apply_expert(test_datafiles, args.treename))
        print(" in method", method)
        for variable in method.variables:
            all_variables.append(variable)
        if args.recursive:
            imp, cla = get_importances_recursive(method, train_datafiles, test_datafiles,
                                                 args.treename, method.variables, global_auc)
            importances.append(imp)
            recursive_classifiers.append(cla)
            labels.append(method.identifier + '\n (recursive)')

            cla_dict[method.identifier + '_recursive'] = cla
        elif args.iterative:
            imp, cla = get_importances(method, train_datafiles, test_datafiles,
                                       args.treename, method.variables, global_auc)
            importances.append(imp)
            iterative_classifiers.append(cla)
            labels.append(method.identifier + '\n (iterative)')
            cla_dict[method.identifier + '_iterative'] = cla
        if args.weightfile:
            importances.append(method.importances)
            labels.append(method.identifier + '\n (weightfile)')

    all_variables = list(sorted(all_variables, key=lambda v: importances[0].get(v, 0.0)))

    importances_dict = {}
    for i, label in enumerate(labels):
        importances_dict[label] = np.array([importances[i].get(v, 0.0) for v in all_variables])

    # todo: distinguish between iterative & recursive
    print("Apply experts on independent data")
    test_probability = {}
    test_target = {}
    for method in methods:
        ps = {}
        ts = {}
        for classifier in cla:
            p, t = cla[classifier].apply_expert(test_datafiles, args.treename)
            ps[classifier] = p
            ts[classifier] = t
        test_probability[method.identifier] = ps
        test_target[method.identifier] = ts

    print("Apply experts on training data")
    train_probability = {}
    train_target = {}
    if args.train_datafiles is not None:
        train_datafiles = sum(args.train_datafiles, [])
        for method in methods:
            ps = {}
            ts = {}
            for classifier in cla:
                p, t = cla[classifier].apply_expert(train_datafiles, args.treename)
                ps[classifier] = p
                ts[classifier] = t
            train_probability[method.identifier] = ps
            train_target[method.identifier] = ts

    # Change working directory after experts run, because they might want to access
    # a locadb in the current working directory
    import shutil

    with tempfile.TemporaryDirectory() as tempdir:
        os.chdir(tempdir)

        o = b2latex.LatexFile()
        o += b2latex.TitlePage(title='Automatic Feature Importance Report',
                               authors=[r'Thomas Keck', 'Markus Prim', 'Moritz Gelb'],
                               abstract='Feature importance calculation by leaving one variable out and retrain.',
                               add_table_of_contents=False,
                               clearpage=False).finish()
        o += b2latex.Section("General Feature Importance")
        graphics = b2latex.Graphics()
        p = plotting.Importance()
        read_root_var = [Belle2.invertMakeROOTCompatible(v) for v in all_variables]
        print("\n")
        print(importances_dict)
        print(labels)
        print(read_root_var)
        print("\n")
        p.add(importances_dict, labels, read_root_var)
        p.finish()
        p.save('importance.png')
        graphics.add('importance.png', width=1.0)
        o += graphics.finish()

        for identifier in identifiers:
            if args.recursive:
                o += b2latex.Section("Recursive Feature Importance")
                o += b2latex.String("""
                            Calculate the importance of the variables of a method by retraining the method without
                            one of the variables at a time. Then the best variable (the one which leads to the lowest
                            area under the curve if it is left out) is removed and the importance of the remaining
                            variables is calculated recursively.
                            """)
            if args.iterative:
                o += b2latex.Section("Iterative Feature Importance")
                o += b2latex.String("""
                                    Calculate the importance of the variables of a method by retraining the method
                                    without one of the variables at a time and comparing the auc to the global
                                    area under the curve.
                                    """)
            if args.weightfile:
                o += b2latex.Section("Feature Importance")
                o += b2latex.String("""
                                    Read feature importances from weightfile.
                                    """)

            for variable in reversed(all_variables):
                if args.weightfile:
                    pass
                for classifier in cla:
                    if classifier == variable:
                        if args.recursive:
                            o += b2latex.SubSection(
                                "Without variable {} (and variables above)".format(Belle2.invertMakeROOTCompatible(classifier)))
                        elif args.iterative:
                            o += b2latex.SubSection(
                                "Without variable {}".format(Belle2.invertMakeROOTCompatible(classifier)))
                        probability = {classifier: np.r_[train_probability[identifier][classifier],
                                                         test_probability[identifier][classifier]]}
                        target = np.r_[train_target[identifier][classifier], test_target[identifier][classifier]]
                        train_mask = np.r_[np.ones(len(train_target[identifier][classifier])),
                                           np.zeros(len(test_target[identifier][classifier]))]
                        graphics = b2latex.Graphics()
                        p = plotting.Overtraining()
                        p.add(probability, classifier, train_mask == 1, train_mask == 0, target == 1, target == 0, )
                        p.finish()

                        p.axis.set_title(
                            "Overtraining check for {} without variable {} ".format(
                                identifier, Belle2.invertMakeROOTCompatible(classifier)))
                        p.save('overtraining_plot_{}_wo_{}.png'.format(hash(identifier), classifier))
                        graphics.add('overtraining_plot_{}_wo_{}.png'.format(hash(identifier), classifier), width=1.0)
                        o += graphics.finish()

            o.save('latex.tex', compile=True)
            os.chdir(old_cwd)
            shutil.copy(tempdir + '/latex.pdf', args.outputfile)
