#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2_mva
from basf2_mva_evaluation import roc_plot, diag_plot, distribution_plot, overtraining_plot
import argparse
import tempfile

import numpy as np
from B2Tools import b2latex, format
import ROOT
from ROOT import Belle2
from ROOT import gSystem
gSystem.Load('libanalysis.so')

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
    parser.add_argument('-test', '--test_datafiles', dest='test_datafiles', type=str, required=False, action='append', nargs='+',
                        help='Data file containing ROOT TTree with independent test data')
    parser.add_argument('-tree', '--treename', dest='treename', type=str, default='tree', help='Treename in data file')
    parser.add_argument('-out', '--outputfile', dest='outputfile', type=str, default='output.pdf',
                        help='Name of the outputted pdf file')
    args = parser.parse_args()
    return args


class WeightfileInformation:
    """
    Extract information about an expert from the database using the given identifiers
    """

    def __init__(self, identifier):
        """
        Construct new Weightfile Information object
        """
        #: Label of this weightfile
        self.label = identifier
        weightfile = basf2_mva.Weightfile.load(identifier)
        #: General options
        self.general_options = basf2_mva.GeneralOptions()
        weightfile.getOptions(self.general_options)
        variables = [Belle2.makeROOTCompatible(v) for v in self.general_options.m_variables]
        importances = weightfile.getFeatureImportance()
        #: Variables and importanceses
        self.variables = {k: importances[k] for k in variables}

        #: Branch names as written by the basf2_mva.expert
        self.branch_probability = Belle2.makeROOTCompatible(identifier)
        self.branch_target = Belle2.makeROOTCompatible(identifier + '_' + self.general_options.m_target_variable)


if __name__ == '__main__':
    old_cwd = os.getcwd()
    ROOT.gROOT.SetBatch(True)
    args = getCommandLineOptions()

    tempdir = tempfile.mkdtemp()
    identifiers = sum(args.identifiers, [])
    train_datafiles = sum(args.train_datafiles, [])
    if args.test_datafiles is not None:
        test_datafiles = sum(args.test_datafiles, [])
    else:
        test_datafiles = []

    train_rootfilename = tempdir + '/train_expert.root'
    basf2_mva.expert(basf2_mva.vector(*identifiers), basf2_mva.vector(*train_datafiles), args.treename, train_rootfilename)
    train_rootfile = ROOT.TFile(train_rootfilename, "UPDATE")

    test_rootfile = None
    if len(test_datafiles) > 0:
        test_rootfilename = tempdir + '/test_expert.root'
        basf2_mva.expert(basf2_mva.vector(*identifiers), basf2_mva.vector(*test_datafiles), args.treename, test_rootfilename)
        test_rootfile = ROOT.TFile(test_rootfilename, "UPDATE")

    informations = [WeightfileInformation(identifier) for identifier in identifiers]
    branches_target = list(map(lambda x: x.branch_target, informations))
    branches_probability = list(map(lambda x: x.branch_probability, informations))
    labels = list(map(lambda x: x.label, informations))
    variables = set([v for information in informations for v in information.variables.keys()])

    # Change working directory after experts run, because they might want to access
    # a locadb in the current working directory
    os.chdir(tempdir)

    o = b2latex.LatexFile()
    o += b2latex.TitlePage(title='Automatic MVA Evaluation',
                           authors=['Thomas Keck, Moritz Gelb'],
                           abstract='Evaluation plots',
                           add_table_of_contents=True).finish()

    o += b2latex.Section("Variables")
    table = b2latex.LongTable(columnspecs=r'p{5cm}' + r'|rr' * len(informations),
                              caption='List of variables used in the training: ' + ', '.join(map(format.string, labels)),
                              head=r'Name ' + ' & Rank / Importance ' * len(informations) + r' \\',
                              format_string=r'{} ' + ' & ${} / {:.4f}$ ' * len(informations))
    sorted_var = {}
    for v in variables:
        l = []
        for information in informations:
            if v in information.variables:
                l += [list(map(lambda x: x[0], reversed(sorted(information.variables.items(), key=lambda x: x[1])))).index(v),
                      information.variables[v]]
            else:
                l += [-1, 0]
            sorted_var[v] = l

    rank = 0
    for variable in sorted_var.items():
        for var, list in sorted_var.items():
            if list[0] == rank:
                rank += 1
                table.add(format.variable(Belle2.invertMakeROOTCompatible(var)), *list)

    for var, list in sorted_var.items():
        if list[0] == -1:
            table.add(format.variable(Belle2.invertMakeROOTCompatible(var)), *list)

    o += table.finish()

    o += b2latex.Section("ROC Plot")
    graphics = b2latex.Graphics()
    roc_plot.from_file(train_rootfile, branches_probability, branches_target, labels, 'roc_plot.png')
    graphics.add('roc_plot.png', width=1.0)
    o += graphics.finish()

    o += b2latex.Section("Diagonal Plot")
    graphics = b2latex.Graphics()
    diag_plot.from_file(train_rootfile, branches_probability, branches_target, labels, 'diagonal_plot.png')
    graphics.add('diagonal_plot.png', width=1.0)
    o += graphics.finish()

    o += b2latex.Section("Distribution Plot")
    graphics = b2latex.Graphics()
    distribution_plot.from_file(train_rootfile, branches_probability, branches_target, labels, 'distribution_plot.png')
    graphics.add('distribution_plot.png', width=1.0)
    o += graphics.finish()

    o += b2latex.Section("Overtraining Plot")
    if test_rootfile is not None:
        for branch_probability, branch_target, label in zip(branches_probability, branches_target, labels):
            graphics = b2latex.Graphics()
            filename = 'overtraining_plot_{}.png'.format(hashlib.md5(label.encode('utf-8')).hexdigest())
            overtraining_plot.from_file(train_rootfile, test_rootfile, [branch_probability],
                                        [branch_target], [label], filename)
            graphics.add(filename, width=1.0)
            o += graphics.finish()

    o.finish()
    o.save('latex.tex', compile=True)
    os.chdir(old_cwd)
    shutil.copy(tempdir + '/latex.pdf', args.outputfile)
