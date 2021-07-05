##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import ROOT

import numpy as np

import tracking.validation.classification as classification
from tracking.root_utils import root_cd

import logging


def get_logger():
    return logging.getLogger(__name__)


def get_truth_name(variable_names):
    """Selects the shortest variable name form containing the 'truth'."""
    truth_names = [name for name in variable_names if "truth" in name]

    # select the shortest
    try:
        truth_name = min(truth_names, key=len)
    except ValueError:
        raise ValueError("variable_names='%s' does not contain a truth variable" % variable_names)
    else:
        return truth_name


class ClassificationOverview:
    """Class to generate overview plots for the classification power of various variables from a TTree.

    In order to get an overview, which variables from a diverse set, generated from a recording filter
    or some other sort of validation, perform well in classification task.

    """

    def __init__(self,
                 output_file_name,
                 truth_name=None,
                 select=[],
                 exclude=[],
                 groupbys=[],
                 auxiliaries=[],
                 filters=[]):
        """Constructor"""
        #: cached output filename
        self.output_file_name = output_file_name
        #: cached truth name
        self.truth_name = truth_name
        #: cached selection-specifier array
        self.select = select
        #: cached exclusion-specifier array
        self.exclude = exclude
        #: cached groupby-specifier array
        self.groupbys = groupbys
        #: cached auxiliary-specifier array
        self.auxiliaries = auxiliaries
        #: cached filter-specifier array
        self.filters = filters

        #: array of classification analyses
        self.classification_analyses = []

    def train(self, input_tree):
        """Main method feed with a TTree containing the truth variable and the variables to be investigated.

        Branches that contain "truth" in the name are considered to directly contain information about
        true classification target and are not analysed here.

        Args:
            input_tree (ROOT.TTree) : Tree containing the variables to be investigated
                as well as the classification target.
            truth_name (str, optional) : Name of the branch of the classification target.
                If not given the Branch with the shortest name containing "truth" is selected.
        """

        if isinstance(self.output_file_name, str):
            output_file = ROOT.TFile(self.output_file_name, "RECREATE")
        else:
            output_file = self.output_file_name

        output_file.cd()

        column_names = [leave.GetName() for leave in input_tree.GetListOfLeaves()]

        tree_name = input_tree.GetName()

        truth_name = self.truth_name

        if truth_name is None:
            truth_name = get_truth_name(column_names)

        if truth_name not in column_names:
            raise KeyError("Truth column {truth} not in tree {tree}".format(truth=truth_name,
                                                                            tree=tree_name))
        variable_names = [name for name in column_names if name != truth_name]

        exclude = self.exclude
        select = self.select
        groupbys = self.groupbys
        auxiliaries = self.auxiliaries
        filters = self.filters

        if select:
            variable_names = [name for name in variable_names if name in select]

        if exclude:
            variable_names = [name for name in variable_names if name not in exclude]

        if filters:
            variable_names = [name for name in variable_names if name not in filters]

        # Remove the variables that have Monte Carlo truth information unless explicitly selected
        variable_names = [name for name
                          in variable_names
                          if "truth" not in name or name in select]

        print("Truth name", truth_name)
        print("Variable names", variable_names)

        import root_numpy
        print("Loading tree")
        branch_names = {*variable_names, truth_name, *groupbys, *auxiliaries, *filters}
        branch_names = [name for name in branch_names if name]
        input_array = root_numpy.tree2array(input_tree, branches=branch_names)
        input_record_array = input_array.view(np.recarray)

        if filters:
            for filter in filters:
                filter_values = input_record_array[filter]
                input_record_array = input_record_array[np.nonzero(filter_values)]

        print("Loaded tree")
        truths = input_record_array[truth_name]

        if not groupbys:
            groupbys = [None]

        for groupby in groupbys:
            if groupby is None or groupby == "":
                groupby_parts = [(None, slice(None))]
            else:
                groupby_parts = []
                groupby_values = input_record_array[groupby]
                unique_values, indices = np.unique(groupby_values, return_inverse=True)
                for idx, value in enumerate(unique_values):
                    groupby_parts.append((value, indices == idx))

            for groupby_value, groupby_select in groupby_parts:
                if groupby is None:
                    groupby_folder_name = '.'
                else:
                    groupby_folder_name = "groupby_{name}_{value}".format(name=groupby, value=groupby_value)

                with root_cd(groupby_folder_name) as tdirectory:
                    for variable_name in variable_names:
                        print('Analyse', variable_name, 'groupby', groupby, '=', groupby_value)

                        if variable_name == groupby:
                            continue

                        # Get the truths as a numpy array
                        estimates = input_record_array[variable_name]
                        estimates[estimates == np.finfo(np.float32).max] = float("nan")
                        estimates[estimates == -np.finfo(np.float32).max] = -float("inf")
                        auxiliaries = {name: input_record_array[name][groupby_select] for name in self.auxiliaries}

                        classification_analysis = classification.ClassificationAnalysis(
                            contact="",
                            quantity_name=variable_name,
                            outlier_z_score=5.0,
                            allow_discrete=True,
                        )
                        classification_analysis.analyse(
                            estimates[groupby_select],
                            truths[groupby_select],
                            auxiliaries=auxiliaries
                        )

                        with root_cd(variable_name) as tdirectory:
                            classification_analysis.write(tdirectory)

                        self.classification_analyses.append(classification_analysis)

        if isinstance(self.output_file_name, str):
            output_file.Close()

        print("Saved overviews completely")
