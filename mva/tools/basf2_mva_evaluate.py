#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2_mva
import basf2_mva_util

from basf2_mva_evaluation import plotting
import argparse
import tempfile

import numpy as np
from B2Tools import b2latex, format

import ROOT

import os
import shutil
import sys
import hashlib


def getCommandLineOptions():
    """ Parses the command line options of the fei and returns the corresponding arguments. """
    parser = argparse.ArgumentParser()
    parser.add_argument('-id', '--identifiers', dest='identifiers', type=str, required=True, action='append', nargs='+',
                        help='DB Identifier or weightfile')
    parser.add_argument('-train', '--train_datafiles', dest='train_datafiles', type=str, required=False, action='append', nargs='+',
                        help='Data file containing ROOT TTree used during training')
    parser.add_argument('-data', '--datafiles', dest='datafiles', type=str, required=True, action='append', nargs='+',
                        help='Data file containing ROOT TTree with independent test data')
    parser.add_argument('-tree', '--treename', dest='treename', type=str, default='tree', help='Treename in data file')
    parser.add_argument('-out', '--outputfile', dest='outputfile', type=str, default='output.pdf',
                        help='Name of the outputted pdf file')
    parser.add_argument('-plot', '--plot-variables', dest='plot_variables', action='store_true',
                        help='Plot the variables e.g. Correlation Matrix this can take very long')
    args = parser.parse_args()
    return args


def unique(input):
    """
    Returns a list containing only unique elements, keeps the original order of the list
    @param input list containing the elements
    """
    output = []
    for x in input:
        if x not in output:
            output.append(x)
    return output


if __name__ == '__main__':

    ROOT.gROOT.SetBatch(True)

    old_cwd = os.getcwd()
    args = getCommandLineOptions()

    identifiers = sum(args.identifiers, [])
    datafiles = sum(args.datafiles, [])

    print("Load methods")
    methods = [basf2_mva_util.Method(identifier) for identifier in identifiers]

    print("Apply experts on independent data")
    test_probability = {}
    test_target = {}
    for method in methods:
        p, t = method.apply_expert(datafiles, args.treename)
        test_probability[method.identifier] = p
        test_target[method.identifier] = t

    print("Apply experts on training data")
    train_probability = {}
    train_target = {}
    if args.train_datafiles is not None:
        train_datafiles = sum(args.train_datafiles, [])
        for method in methods:
            p, t = method.apply_expert(train_datafiles, args.treename)
            train_probability[method.identifier] = p
            train_target[method.identifier] = t

    variables = unique(v for method in methods for v in method.variables)
    root_variables = unique(v for method in methods for v in method.root_variables)

    if args.plot_variables:
        print("Load variables array")
        rootchain = ROOT.TChain(args.treename)
        for datafile in datafiles:
            rootchain.Add(datafile)

        variables_data = basf2_mva_util.tree2dict(rootchain, root_variables, variables)

    print("Create latex file")
    # Change working directory after experts run, because they might want to access
    # a locadb in the current working directory
    tempdir = tempfile.mkdtemp()
    os.chdir(tempdir)

    o = b2latex.LatexFile()
    o += b2latex.TitlePage(title='Automatic MVA Evaluation',
                           authors=[r'Thomas Keck\\ Moritz Gelb'],
                           abstract='Evaluation plots',
                           add_table_of_contents=True).finish()

    o += b2latex.Section("Classifiers")
    o += b2latex.String(r"""
        This section contains the GeneralOptions and SpecificOptions of all classifiers represented by an XML tree.
        The same information can be retreived using the basf2\_mva\_info tool
    """)

    for method in methods:
        o += b2latex.SubSection(format.string(method.identifier))
        o += b2latex.Listing(language='XML').add(method.description).finish()

    o += b2latex.Section("Variables")
    o += b2latex.String("""
        This section contains an overview of the importance and correlation of the variables used by the classifiers
    """)
    graphics = b2latex.Graphics()
    p = plotting.Importance()
    p.add({i.identifier: np.array([i.importances.get(v, 0.0) for v in variables]) for i in methods},
          identifiers, variables)
    p.finish()
    p.save('importance.png')
    graphics.add('importance.png', width=1.0)
    o += graphics.finish()

    if args.plot_variables:
        graphics = b2latex.Graphics()
        p = plotting.CorrelationMatrix()
        p.add(variables_data, variables,
              test_target[identifiers[0]] == 1,
              test_target[identifiers[0]] == 0)
        p.finish()
        p.save('correlation_signal.png')
        graphics.add('correlation_signal.png', width=1.0)
        o += graphics.finish()

    o += b2latex.Section("ROC Plot")
    o += b2latex.String("""
        This section contains the receiver operating characteristics (ROC) of the classifiers on training and independent data.
        The legend of each plot contains the shortened identifier and the area under the ROC curve in parenthesis.
    """)
    graphics = b2latex.Graphics()
    p = plotting.RejectionOverEfficiency()
    for identifier in identifiers:
        p.add(test_probability, identifier, test_target[identifier] == 1, test_target[identifier] == 0)
    p.finish()
    p.axis.set_title("ROC Rejection Plot on independent data")
    p.save('roc_plot_test.png')
    graphics.add('roc_plot_test.png', width=1.0)
    o += graphics.finish()

    if train_probability:
        graphics = b2latex.Graphics()
        p = plotting.Multiplot(plotting.RejectionOverEfficiency, len(identifiers))
        for i, identifier in enumerate(identifiers):
            p.add(i, train_probability, identifier, train_target[identifier] == 1,
                  train_target[identifier] == 0, label=identifier + ' (train)')
            p.add(i, test_probability, identifier, test_target[identifier] == 1,
                  test_target[identifier] == 0, label=identifier + ' (test)')
        p.finish()
        p.save('roc_multiplot_test.png')
        graphics.add('roc_multiplot_test.png', width=1.0)
        o += graphics.finish()

    o += b2latex.Section("Diagonal Plot")
    graphics = b2latex.Graphics()
    p = plotting.Diagonal()
    for identifier in identifiers:
        p.add(test_probability, identifier, test_target[identifier] == 1, test_target[identifier] == 0)
    p.finish()
    p.axis.set_title("Diagonal plot on independent data")
    p.save('diagonal_plot_test.png')
    graphics.add('diagonal_plot_test.png', width=1.0)
    o += graphics.finish()

    if train_probability:
        o += b2latex.Section("Overtraining Plot")
        for identifier in identifiers:
            probability = {identifier: np.r_[train_probability[identifier], test_probability[identifier]]}
            target = np.r_[train_target[identifier], test_target[identifier]]
            train_mask = np.r_[np.ones(len(train_target[identifier])), np.zeros(len(test_target[identifier]))]
            graphics = b2latex.Graphics()
            p = plotting.Overtraining()
            p.add(probability, identifier,
                  train_mask == 1, train_mask == 0,
                  target == 1, target == 0, )
            p.finish()
            p.axis.set_title("Overtraining check for {}".format(identifier))
            p.save('overtraining_plot_{}.png'.format(hash(identifier)))
            graphics.add('overtraining_plot_{}.png'.format(hash(identifier)), width=1.0)
            o += graphics.finish()

    o.save('latex.tex', compile=True)
    os.chdir(old_cwd)
    shutil.copy(tempdir + '/latex.pdf', args.outputfile)
