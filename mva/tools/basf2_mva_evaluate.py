#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2_mva_util

from basf2_mva_evaluation import plotting
import argparse
import tempfile

import numpy as np
from B2Tools import b2latex, format

import ROOT

import os
import shutil
import collections


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
    parser.add_argument('-w', '--working_directory', dest='working_directory', type=str, default='',
                        help="""Working directory where the created images and root files are stored,
                              default is to create a temporary directory.""")
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


def create_abbreviations(names, length=5):
    count = dict()
    for name in names:
        abbreviation = name[:length]
        if abbreviation not in count:
            count[abbreviation] = 0
        count[abbreviation] += 1
    abbreviations = collections.OrderedDict()

    count2 = dict()
    for name in names:
        abbreviation = name[:length]
        abbreviations[name] = abbreviation
        if count[abbreviation] > 1:
            if abbreviation not in count2:
                count2[abbreviation] = 0
            count2[abbreviation] += 1
            abbreviations[name] += str(count2[abbreviation])
    return abbreviations


if __name__ == '__main__':

    ROOT.gROOT.SetBatch(True)

    old_cwd = os.getcwd()
    args = getCommandLineOptions()

    identifiers = sum(args.identifiers, [])
    identifier_abbreviations = create_abbreviations(identifiers)

    datafiles = sum(args.datafiles, [])

    print("Load methods")
    methods = [basf2_mva_util.Method(identifier) for identifier in identifiers]

    print("Apply experts on independent data")
    test_probability = {}
    test_target = {}
    for method in methods:
        p, t = method.apply_expert(datafiles, args.treename)
        test_probability[identifier_abbreviations[method.identifier]] = p
        test_target[identifier_abbreviations[method.identifier]] = t

    print("Apply experts on training data")
    train_probability = {}
    train_target = {}
    if args.train_datafiles is not None:
        train_datafiles = sum(args.train_datafiles, [])
        for method in methods:
            p, t = method.apply_expert(train_datafiles, args.treename)
            train_probability[identifier_abbreviations[method.identifier]] = p
            train_target[identifier_abbreviations[method.identifier]] = t

    variables = unique(v for method in methods for v in method.variables)
    variable_abbreviations = create_abbreviations(variables)
    root_variables = unique(v for method in methods for v in method.root_variables)

    spectators = unique(v for method in methods for v in method.spectators)
    spectator_abbreviations = create_abbreviations(spectators)
    root_spectators = unique(v for method in methods for v in method.root_spectators)

    print("Load variables array")
    rootchain = ROOT.TChain(args.treename)
    for datafile in datafiles:
        rootchain.Add(datafile)

    variables_data = basf2_mva_util.tree2dict(rootchain, root_variables, list(variable_abbreviations.values()))
    spectators_data = basf2_mva_util.tree2dict(rootchain, root_spectators, list(spectator_abbreviations.values()))

    print("Create latex file")
    # Change working directory after experts run, because they might want to access
    # a locadb in the current working directory
    with tempfile.TemporaryDirectory() as tempdir:
        if args.working_directory == '':
            os.chdir(tempdir)
        else:
            os.chdir(args.working_directory)

        o = b2latex.LatexFile()
        o += b2latex.TitlePage(title='Automatic MVA Evaluation',
                               authors=[r'Thomas Keck\\ Moritz Gelb\\ Nils Braun'],
                               abstract='Evaluation plots',
                               add_table_of_contents=True).finish()

        o += b2latex.Section("Classifiers")
        o += b2latex.String(r"""
            This section contains the GeneralOptions and SpecificOptions of all classifiers represented by an XML tree.
            The same information can be retreived using the basf2\_mva\_info tool.
        """)

        table = b2latex.LongTable(r"ll", "Abbreviations of identifiers", "{name} & {abbr}", r"Identifier & Abbreviation")
        for identifier in identifiers:
            table.add(name=format.string(identifier), abbr=format.string(identifier_abbreviations[identifier]))
        o += table.finish()

        for method in methods:
            o += b2latex.SubSection(format.string(method.identifier))
            o += b2latex.Listing(language='XML').add(method.description).finish()

        o += b2latex.Section("Variables")
        o += b2latex.String("""
            This section contains an overview of the importance and correlation of the variables used by the classifiers.
            And distribution plots of the variables on the independent dataset. The distributions are normed for signal and
            background separately, and only the region +- 3 sigma around the mean is shown.
        """)

        table = b2latex.LongTable(r"ll", "Abbreviations of variables", "{name} & {abbr}", r"Variable & Abbreviation")
        for v in variables:
            table.add(name=format.string(v), abbr=format.string(variable_abbreviations[v]))
        o += table.finish()

        o += b2latex.SubSection("Importance")
        graphics = b2latex.Graphics()
        p = plotting.Importance()
        p.add({identifier_abbreviations[i.identifier]: np.array([i.importances.get(v, 0.0) for v in variables]) for i in methods},
              identifier_abbreviations.values(), variable_abbreviations.values())
        p.finish()
        p.save('importance.png')
        graphics.add('importance.png', width=1.0)
        o += graphics.finish()

        o += b2latex.SubSection("Correlation")
        first_identifier_abbr = list(identifier_abbreviations.values())[0]
        graphics = b2latex.Graphics()
        p = plotting.CorrelationMatrix()
        p.add(variables_data, variable_abbreviations.values(),
              test_target[first_identifier_abbr] == 1,
              test_target[first_identifier_abbr] == 0)
        p.finish()
        p.save('correlation_plot.png')
        graphics.add('correlation_plot.png', width=1.0)
        o += graphics.finish()

        if False:
            graphics = b2latex.Graphics()
            p = plotting.TSNE()
            p.add(variables_data, variable_abbreviations.values(),
                  test_target[first_identifier_abbr] == 1,
                  test_target[first_identifier_abbr] == 0)
            p.finish()
            p.save('tsne_plot.png')
            graphics.add('tsne_plot.png', width=1.0)
            o += graphics.finish()

        for v in variables:
            variable_abbr = variable_abbreviations[v]
            o += b2latex.SubSection(format.string(v))
            graphics = b2latex.Graphics()
            p = plotting.VerboseDistribution(normed=True, range_in_std=3)
            p.add(variables_data, variable_abbr, test_target[first_identifier_abbr] == 1, label="Signal")
            p.add(variables_data, variable_abbr, test_target[first_identifier_abbr] == 0, label="Background")
            p.finish()
            p.save('variable_{}.png'.format(hash(v)))
            graphics.add('variable_{}.png'.format(hash(v)), width=1.0)
            o += graphics.finish()

        o += b2latex.Section("Classifier Plot")
        o += b2latex.String("This section contains the receiver operating characteristics (ROC), purity projection, ..."
                            "of the classifiers on training and independent data."
                            "The legend of each plot contains the shortened identifier and the area under the ROC curve"
                            "in parenthesis.")

        o += b2latex.Section("ROC Plot")
        graphics = b2latex.Graphics()
        p = plotting.RejectionOverEfficiency()
        for identifier in identifier_abbreviations.values():
            p.add(test_probability, identifier, test_target[identifier] == 1, test_target[identifier] == 0)
        p.finish()
        p.axis.set_title("ROC Rejection Plot on independent data")
        p.save('roc_plot_test.png')
        graphics.add('roc_plot_test.png', width=1.0)
        o += graphics.finish()

        if train_probability:
            for i, identifier in enumerate(identifiers):
                graphics = b2latex.Graphics()
                p = plotting.RejectionOverEfficiency()
                identifier_abbr = identifier_abbreviations[identifier]
                p.add(train_probability, identifier_abbr, train_target[identifier_abbr] == 1,
                      train_target[identifier_abbr] == 0, label='Train')
                p.add(test_probability, identifier_abbr, test_target[identifier_abbr] == 1,
                      test_target[identifier_abbr] == 0, label='Test')
                p.finish()
                p.axis.set_title(identifier)
                p.save('roc_test_{}.png'.format(hash(identifier)))
                graphics.add('roc_test_{}.png'.format(hash(identifier)), width=1.0)
                o += graphics.finish()

        o += b2latex.Section("Classification Results")

        for identifier in identifiers:
            identifier_abbr = identifier_abbreviations[identifier]
            o += b2latex.SubSection(format.string(identifier_abbr))
            graphics = b2latex.Graphics()
            p = plotting.Multiplot(plotting.PurityAndEfficiencyOverCut, 2)
            p.add(0, test_probability, identifier_abbr, test_target[identifier_abbr] == 1,
                  test_target[identifier_abbr] == 0, normed=True)
            p.sub_plots[0].axis.set_title("Classification result in test data for {identifier}".format(identifier=identifier))

            p.add(1, test_probability, identifier_abbr, test_target[identifier_abbr] == 1,
                  test_target[identifier_abbr] == 0, normed=False)
            p.sub_plots[1].axis.set_title("Classification result in test data for {identifier}".format(identifier=identifier))
            p.finish()

            p.save('classification_result_{identifier}.png'.format(identifier=hash(identifier)))
            graphics.add('classification_result_{identifier}.png'.format(identifier=hash(identifier)), width=1)
            o += graphics.finish()

        o += b2latex.Section("Diagonal Plot")
        graphics = b2latex.Graphics()
        p = plotting.Diagonal()
        for identifier in identifiers:
            o += b2latex.SubSection(format.string(identifier_abbr))
            identifier_abbr = identifier_abbreviations[identifier]
            p.add(test_probability, identifier_abbr, test_target[identifier_abbr] == 1, test_target[identifier_abbr] == 0)
        p.finish()
        p.axis.set_title("Diagonal plot on independent data")
        p.save('diagonal_plot_test.png')
        graphics.add('diagonal_plot_test.png', width=1.0)
        o += graphics.finish()

        if train_probability:
            o += b2latex.SubSection("Overtraining Plot")
            for identifier in identifiers:
                identifier_abbr = identifier_abbreviations[identifier]
                probability = {identifier_abbr: np.r_[train_probability[identifier_abbr], test_probability[identifier_abbr]]}
                target = np.r_[train_target[identifier_abbr], test_target[identifier_abbr]]
                train_mask = np.r_[np.ones(len(train_target[identifier_abbr])), np.zeros(len(test_target[identifier_abbr]))]
                graphics = b2latex.Graphics()
                p = plotting.Overtraining()
                p.add(probability, identifier_abbr,
                      train_mask == 1, train_mask == 0,
                      target == 1, target == 0, )
                p.finish()
                p.axis.set_title("Overtraining check for {}".format(identifier))
                p.save('overtraining_plot_{}.png'.format(hash(identifier)))
                graphics.add('overtraining_plot_{}.png'.format(hash(identifier)), width=1.0)
                o += graphics.finish()

        o += b2latex.Section("Spectators")
        o += b2latex.String("This section contains the distribution and dependence on the"
                            "classifier outputs of all spectator variables.")

        table = b2latex.LongTable(r"ll", "Abbreviations of spectators", "{name} & {abbr}", r"Spectator & Abbreviation")
        for s in spectators:
            table.add(name=format.string(s), abbr=format.string(spectator_abbreviations[s]))
        o += table.finish()

        for spectator in spectators:
            spectator_abbr = spectator_abbreviations[spectator]
            o += b2latex.SubSection(format.string(spectator))
            graphics = b2latex.Graphics()
            p = plotting.VerboseDistribution()
            p.add(spectators_data, spectator_abbr, test_target[first_identifier_abbr] == 1, label="Signal")
            p.add(spectators_data, spectator_abbr, test_target[first_identifier_abbr] == 0, label="Background")
            p.finish()
            p.save('spectator_{}.png'.format(hash(spectator)))
            graphics.add('spectator_{}.png'.format(hash(spectator)), width=1.0)
            o += graphics.finish()

            for identifier in identifiers:
                o += b2latex.SubSubSection(format.string(spectator) + " with classifier " + format.string(identifier))
                identifier_abbr = identifier_abbreviations[identifier]
                data = {identifier_abbr: test_probability[identifier_abbr], spectator_abbr: spectators_data[spectator_abbr]}
                graphics = b2latex.Graphics()
                p = plotting.Correlation()
                p.add(data, spectator_abbr, identifier_abbr, list(range(10, 100, 10)),
                      test_target[identifier_abbr] == 1,
                      test_target[identifier_abbr] == 0)
                p.finish()
                p.save('correlation_plot_{}_{}.png'.format(hash(spectator), hash(identifier)))
                graphics.add('correlation_plot_{}_{}.png'.format(hash(spectator), hash(identifier)), width=1.0)
                o += graphics.finish()

        o.save('latex.tex', compile=True)
        os.chdir(old_cwd)
        if args.working_directory == '':
            shutil.copy(tempdir + '/latex.pdf', args.outputfile)
        else:
            shutil.copy(args.working_directory + '/latex.pdf', args.outputfile)
