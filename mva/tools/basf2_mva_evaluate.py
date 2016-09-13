#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2_mva
from basf2_mva_evaluation import plotting, histogram
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
    parser.add_argument('-train', '--train_datafiles', dest='train_datafiles', type=str, required=False, action='append', nargs='+',
                        help='Data file containing ROOT TTree used during training')
    parser.add_argument('-data', '--datafiles', dest='datafiles', type=str, required=True, action='append', nargs='+',
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
        self.description = str(basf2_mva.info(identifier))

        #: Branch names as written by the basf2_mva.expert
        self.branch_probability = Belle2.makeROOTCompatible(identifier)
        self.branch_target = Belle2.makeROOTCompatible(identifier + '_' + self.general_options.m_target_variable)


def tree2dict(tree, tree_columns, dict_columns=None):
    if dict_columns is None:
        dict_columns = tree_columns
    d = {column: np.zeros((tree.GetEntries(),)) for column in dict_columns}
    for iEvent, event in enumerate(tree):
        for dict_column, tree_column in zip(dict_columns, tree_columns):
            d[dict_column][iEvent] = getattr(event, tree_column)
    return d


def apply_experts(informations, datafiles, treename):
    tempdir = tempfile.mkdtemp()
    datafiles = sum(datafiles, [])
    rootfilename = tempdir + '/expert.root'
    basf2_mva.expert(basf2_mva.vector(*[i.label for i in informations]),
                     basf2_mva.vector(*datafiles),
                     treename,
                     rootfilename)
    rootfile = ROOT.TFile(rootfilename, "UPDATE")
    roottree = rootfile.Get("variables")
    probabilities = tree2dict(roottree, [i.branch_probability for i in informations], [i.label for i in informations])
    targets = tree2dict(roottree, [i.branch_target for i in informations], [i.label for i in informations])
    shutil.rmtree(tempdir)
    return probabilities, targets


if __name__ == '__main__':

    old_cwd = os.getcwd()
    ROOT.gROOT.SetBatch(True)
    args = getCommandLineOptions()

    identifiers = sum(args.identifiers, [])
    informations = [WeightfileInformation(identifier) for identifier in identifiers]

    test_probability, test_target = apply_experts(informations, args.datafiles, args.treename)
    if args.train_datafiles is not None:
        train_probability, train_target = apply_experts(informations, args.train_datafiles, args.treename)
    else:
        train_probability, train_target = None, None

    labels = list(map(lambda x: x.label, informations))
    variables = sorted(set([v for information in informations for v in information.variables.keys()]),
                       key=lambda v: informations[0].variables.get(v, 0.0))

    rootchain = ROOT.TChain(args.treename)
    for variable_datafile in sum(args.datafiles, []):
        rootchain.Add(variable_datafile)
    variables_data = tree2dict(rootchain, variables, [Belle2.invertMakeROOTCompatible(var) for var in variables])

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
    o += b2latex.String("""
        This section contains the GeneralOptions and SpecificOptions of all classifiers represented by an XML tree
    """)

    for information in informations:
        o += b2latex.SubSection(information.label)
        o += b2latex.Listing(language='XML').add(information.description).finish()

    o += b2latex.Section("Variables")
    o += b2latex.String("""
        This section contains an overview of the importance and correlation of the variables used by the classifiers
    """)
    graphics = b2latex.Graphics()
    p = plotting.Importance()
    p.add({i.label: np.array([i.variables.get(v, 0.0) for v in variables]) for i in informations},
          identifiers, [Belle2.invertMakeROOTCompatible(var) for var in variables])
    p.finish()
    p.save('importance.png')
    graphics.add('importance.png', width=1.0)
    o += graphics.finish()

    graphics = b2latex.Graphics()
    p = plotting.CorrelationMatrix()
    p.add(variables_data, [Belle2.invertMakeROOTCompatible(var) for var in variables],
          test_target[identifiers[0]] == 1,
          test_target[identifiers[0]] == 0)
    p.finish()
    p.save('correlation_signal.png')
    graphics.add('correlation_signal.png', width=1.0)
    o += graphics.finish()

    o += b2latex.Section("ROC Plot")
    o += b2latex.String("""
        This section contains the receiver operating characteristics (ROC) of the classifiers on training and independent data
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

    if train_probability is not None:
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

    if train_probability is not None:
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
            p.save('overtraining_plot.png')
            graphics.add('overtraining_plot.png', width=1.0)
            o += graphics.finish()

    o.save('latex.tex', compile=True)
    os.chdir(old_cwd)
    shutil.copy(tempdir + '/latex.pdf', args.outputfile)
