#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2_mva_util

from basf2_mva_evaluation import plotting
from basf2 import conditions
import argparse
import tempfile

import numpy as np
from B2Tools import b2latex, format
from basf2 import B2INFO

import os
import shutil
import collections
from typing import List, Any


def get_argument_parser() -> argparse.ArgumentParser:
    """ Parses the command line options of the fei and returns the corresponding arguments. """
    parser = argparse.ArgumentParser()
    parser.add_argument('-id', '--identifiers', dest='identifiers', type=str, required=True, action='append', nargs='+',
                        help='DB Identifier or weightfile')
    parser.add_argument('-train', '--train_datafiles', dest='train_datafiles', type=str, required=False, action='append', nargs='+',
                        help='Data file containing ROOT TTree used during training')
    parser.add_argument('-data', '--datafiles', dest='datafiles', type=str, required=True, action='append', nargs='+',
                        help='Data file containing ROOT TTree with independent test data')
    parser.add_argument('-tree', '--treename', dest='treename', type=str, default='tree', help='Treename in data file')
    parser.add_argument('-out', '--outputfile', dest='outputfile', type=str, default='output.zip',
                        help='Name of the created .zip archive file if not compiling or a pdf file if compilation is successful.')
    parser.add_argument('-w', '--working_directory', dest='working_directory', type=str, default='',
                        help="""Working directory where the created images and root files are stored,
                              default is to create a temporary directory.""")
    parser.add_argument('-l', '--localdb', dest='localdb', type=str, action='append', nargs='+', required=False,
                        help="""path or list of paths to local database(s) containing the mvas of interest.
                                The testing payloads are preprended and take precedence over payloads in global tags.""")
    parser.add_argument('-g', '--globaltag', dest='globaltag', type=str, action='append', nargs='+', required=False,
                        help='globaltag or list of globaltags containing the mvas of interest. The globaltags are prepended.')
    parser.add_argument('-n', '--fillnan', dest='fillnan', action='store_true',
                        help='Fill nan and inf values with actual numbers')
    parser.add_argument('-c', '--compile', dest='compile', action='store_true',
                        help='Compile latex to pdf directly')
    parser.add_argument('-a', '--abbreviation_length', dest='abbreviation_length',
                        action='store', type=int, default=5,
                        help='Number of characters to which variable names are abbreviated.')
    return parser


def unique(input_list: List[Any]) -> List[Any]:
    """
    Returns a list containing only unique elements, keeps the original order of the list
    @param input_list list containing the elements
    """
    output = []
    for x in input_list:
        if x not in output:
            output.append(x)
    return output


def flatten(input_list: List[List[Any]]) -> List[Any]:
    """
    Flattens a list of lists
    @param input_list list of lists to be flattened
    """
    return [item for sublist in input_list for item in sublist]


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

    import ROOT  # noqa
    ROOT.PyConfig.IgnoreCommandLineOptions = True
    ROOT.PyConfig.StartGuiThread = False
    ROOT.gROOT.SetBatch(True)

    old_cwd = os.getcwd()
    parser = get_argument_parser()
    args = parser.parse_args()

    identifiers = flatten(args.identifiers)
    identifier_abbreviations = create_abbreviations(identifiers, args.abbreviation_length)

    datafiles = flatten(args.datafiles)
    if args.localdb is not None:
        for localdb in flatten(args.localdb):
            conditions.prepend_testing_payloads(localdb)

    if args.globaltag is not None:
        for tag in flatten(args.globaltag):
            conditions.prepend_globaltag(tag)

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
    variable_abbreviations = create_abbreviations(variables, args.abbreviation_length)
    root_variables = unique(v for method in methods for v in method.root_variables)

    spectators = unique(v for method in methods for v in method.spectators)
    spectator_abbreviations = create_abbreviations(spectators, args.abbreviation_length)
    root_spectators = unique(v for method in methods for v in method.root_spectators)

    print("Load variables array")
    rootchain = ROOT.TChain(args.treename)
    for datafile in datafiles:
        rootchain.Add(datafile)

    variables_data = basf2_mva_util.chain2dict(rootchain, root_variables, list(variable_abbreviations.values()))
    spectators_data = basf2_mva_util.chain2dict(rootchain, root_spectators, list(spectator_abbreviations.values()))

    if args.fillnan:
        for column in variable_abbreviations.values():
            np.nan_to_num(variables_data[column], copy=False)

        for column in spectator_abbreviations.values():
            np.nan_to_num(spectators_data[column], copy=False)

    print("Create latex file")
    # Change working directory after experts run, because they might want to access
    # a localdb in the current working directory.
    with tempfile.TemporaryDirectory() as tempdir:
        if args.working_directory == '':
            os.chdir(tempdir)
        else:
            os.chdir(args.working_directory)

        with open('abbreviations.txt', 'w') as f:
            f.write('Identifier Abbreviation : Identifier \n')
            for name, abbrev in identifier_abbreviations.items():
                f.write(f'\t{abbrev} : {name}\n')
            f.write('\n\n\nVariable Abbreviation : Variable \n')
            for name, abbrev in variable_abbreviations.items():
                f.write(f'\t{abbrev} : {name}\n')
            f.write('\n\n\nSpectator Abbreviation : Spectator \n')
            for name, abbrev in spectator_abbreviations.items():
                f.write(f'\t{abbrev} : {name}\n')

        o = b2latex.LatexFile()
        o += b2latex.TitlePage(title='Automatic MVA Evaluation',
                               authors=[r'Thomas Keck\\ Moritz Gelb\\ Nils Braun'],
                               abstract='Evaluation plots',
                               add_table_of_contents=True).finish()

        o += b2latex.Section("Classifiers")
        o += b2latex.String(r"""
            This section contains the GeneralOptions and SpecificOptions of all classifiers represented by an XML tree.
            The same information can be retrieved using the basf2\_mva\_info tool.
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

            The importance scores shown are based on the variable importance as estimated by each MVA method internally.
            This means the variable with the lowest importance will have score 0, and the variable
            with the highest importance will have score 100. If the method does not provide such a ranking, all
            importances will be 0.
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
        p.save('importance.pdf')
        graphics.add('importance.pdf', width=1.0)
        o += graphics.finish()

        o += b2latex.SubSection("Correlation")
        first_identifier_abbr = list(identifier_abbreviations.values())[0]
        graphics = b2latex.Graphics()
        p = plotting.CorrelationMatrix()
        p.add(variables_data, variable_abbreviations.values(),
              test_target[first_identifier_abbr] == 1,
              test_target[first_identifier_abbr] == 0)
        p.finish()
        p.save('correlation_plot.pdf')
        graphics.add('correlation_plot.pdf', width=1.0)
        o += graphics.finish()

        for v in variables:
            variable_abbr = variable_abbreviations[v]
            o += b2latex.SubSection(format.string(v))
            graphics = b2latex.Graphics()
            p = plotting.VerboseDistribution(normed=True, range_in_std=3)
            p.add(variables_data, variable_abbr, test_target[first_identifier_abbr] == 1, label="Signal")
            p.add(variables_data, variable_abbr, test_target[first_identifier_abbr] == 0, label="Background")
            p.finish()
            p.save(f'variable_{hash(v)}.pdf')
            graphics.add(f'variable_{hash(v)}.pdf', width=1.0)
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
        p.save('roc_plot_test.pdf')
        graphics.add('roc_plot_test.pdf', width=1.0)
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
                p.save(f'roc_test_{hash(identifier)}.pdf')
                graphics.add(f'roc_test_{hash(identifier)}.pdf', width=1.0)
                o += graphics.finish()

        o += b2latex.Section("Classification Results")

        for identifier in identifiers:
            identifier_abbr = identifier_abbreviations[identifier]
            o += b2latex.SubSection(format.string(identifier_abbr))
            graphics = b2latex.Graphics()
            p = plotting.Multiplot(plotting.PurityAndEfficiencyOverCut, 2)
            p.add(0, test_probability, identifier_abbr, test_target[identifier_abbr] == 1,
                  test_target[identifier_abbr] == 0, normed=True)
            p.sub_plots[0].axis.set_title(f"Classification result in test data for {identifier}")

            p.add(1, test_probability, identifier_abbr, test_target[identifier_abbr] == 1,
                  test_target[identifier_abbr] == 0, normed=False)
            p.sub_plots[1].axis.set_title(f"Classification result in test data for {identifier}")
            p.finish()

            p.save(f'classification_result_{hash(identifier)}.pdf')
            graphics.add(f'classification_result_{hash(identifier)}.pdf', width=1)
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
        p.save('diagonal_plot_test.pdf')
        graphics.add('diagonal_plot_test.pdf', width=1.0)
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
                p.axis.set_title(f"Overtraining check for {identifier}")
                p.save(f'overtraining_plot_{hash(identifier)}.pdf')
                graphics.add(f'overtraining_plot_{hash(identifier)}.pdf', width=1.0)
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
            p.save(f'spectator_{hash(spectator)}.pdf')
            graphics.add(f'spectator_{hash(spectator)}.pdf', width=1.0)
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
                p.save(f'correlation_plot_{hash(spectator)}_{hash(identifier)}.pdf')
                graphics.add(f'correlation_plot_{hash(spectator)}_{hash(identifier)}.pdf', width=1.0)
                o += graphics.finish()

        if args.compile:
            B2INFO(f"Creating a PDF file at {args.outputfile}. Please remove the '-c' switch if this fails.")
            o.save('latex.tex', compile=True)
        else:
            B2INFO(f"Creating a .zip archive containing plots and a TeX file at {args.outputfile}."
                   f"Please unpack the archive and compile the latex.tex file with pdflatex.")
            o.save('latex.tex', compile=False)

        os.chdir(old_cwd)
        if args.working_directory == '':
            working_directory = tempdir
        else:
            working_directory = args.working_directory

        if args.compile:
            shutil.copy(os.path.join(working_directory, 'latex.pdf'), args.outputfile)
        else:
            base_name = os.path.join(old_cwd, args.outputfile.rsplit('.', 1)[0])
            shutil.make_archive(base_name, 'zip', working_directory)
