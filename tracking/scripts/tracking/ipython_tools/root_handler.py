# Plotting imports
import matplotlib.pyplot as plt
import seaborn as sb
sb.set()
import pandas as pd
import numpy as np
import os

from root_numpy import root2array, list_trees, list_branches, list_structures


def _get_pandas_branches(filename, tree, branches):
    """
    Get all branches which are int or float from the given tree in the given filename.
    If branches is given, use only those branches.
    Filter out any moredimensional branches.
    """
    # TODO: others also??
    structure = list_structures(filename, tree)
    if branches is None or len(branches) == 0:
        all_numerical_branches = filter(
            lambda branch_name: structure[branch_name][0][1] in [
                u"int",
                u"float",
                u"double"],
            structure)
    else:
        all_numerical_branches = filter(lambda branch_name: structure[branch_name][0][1] in [u"int", u"float", u"double"], branches)
    if len(all_numerical_branches) == 0:
        raise ValueError("Could not find any numerical branch in the tree %s. Can not export this tree." % tree)

    # Filter out the branches which are not one dimensional in the first column
    first_row = root2array(filenames=filename, treename=tree, branches=all_numerical_branches, start=0, stop=1, step=1)[0]
    good_indices = map(lambda x: x[0], filter(lambda element_with_index: element_with_index[1].size == 1, enumerate(first_row)))
    good_branches = [all_numerical_branches[b] for b in good_indices]
    if len(good_branches) == 0:
        raise ValueError("Could not find any useable branch in the tree %s. Can not export this tree." % tree)
    return good_branches


class RootReader():

    """ An interator class to read out root files in chunks of data. See read_root for usage"""

    def __init__(self, filename, tree, branches, chunksize):
        self.filename = filename
        self.tree = tree
        self.branches = branches
        self.chunksize = chunksize
        self.current_chunk = 0

    def __iter__(self):
        """ Iterator """
        return self

    def __next__(self):
        """ Python 3 iterator """
        return self.next()

    def next(self):
        """ The next function """
        root_array = root2array(self.filename, self.tree, self.branches,
                                start=self.current_chunk * self.chunksize, stop=(self.current_chunk + 1) * self.chunksize, step=1)
        if len(root_array) == 0:
            raise StopIteration()
        self.current_chunk += 1
        dataframe = pd.DataFrame(root_array)
        return dataframe


def read_root(filename, branches=None, tree_key=None, chunksize=None):
    """
    Export the given root file to a pandas dataframe.
    Coded after the root_pandas project on github.

    Arguments
    ---------
    If tree_key is not None use only the given tree -
       else use all trees and return a dict tree_name->dataframe if the number of trees os bigger than one.
    If branches is not None use only the given branches (if they are numerical and one dimensional), else use all branches.
    If chunksize is not None return a ReadRoot-object - an iterator - which can be uses to receive the data in chunks.
    """

    if not os.path.exists(filename):
        raise IOError("File %s does not exists." % filename)

    if tree_key is not None:
        trees = [tree_key]
    else:
        trees = list_trees(filename)
    result_dataframes = {}
    for tree in trees:
        good_branches = _get_pandas_branches(filename=filename, tree=tree, branches=branches)

        if chunksize is not None:
            result_dataframes.update({tree: RootReader(filename, tree, good_branches, chunksize)})
        else:
            root_array = root2array(filename, tree, good_branches)
            dataframe = pd.DataFrame(root_array)
            result_dataframes.update({tree: dataframe})

    if len(result_dataframes) == 1:
        return result_dataframes.values()[0]
    else:
        return result_dataframes


class TrackingValidationResult:
    #: This class represents a loaded validation root file.
    #: It has methods for plotting the mostly needed graphs

    def __init__(self, filename, label=None, color_index=0, additional_information=None):
        self.filename = filename
        self.pr_data = read_root(self.filename, tree_key="pr_tree/pr_tree")
        self.pr_data["is_prompt"] = (
            np.sqrt(self.pr_data.x_truth ** 2 + self.pr_data.y_truth ** 2) < 0.5) & (self.pr_data.is_primary == 1)
        self.mc_data = read_root(self.filename, tree_key="mc_tree/mc_tree")
        self.mc_data["is_prompt"] = (
            np.sqrt(self.mc_data.x_truth ** 2 + self.mc_data.y_truth ** 2) < 0.5) & (self.mc_data.is_primary == 1)

        self.mc_prompts = self.mc_data[self.mc_data.is_prompt == 1]
        self.pr_prompts = self.pr_data[self.pr_data.is_prompt == 1]

        colors = [(.64, .69, .83), (1, 199.0 / 256, 128.0 / 256),
                  (255.0 / 256, 128.0 / 256, 128.0 / 256), "red", "green", "blue", "black", "gray", "lightgreen"]

        self.label = label
        self.color = colors[color_index % len(colors)]

        self.finding_efficiency = None
        self.hit_efficiency = None
        self.fake_rate = None
        self.clone_rate = None
        self.get_figure_of_merits()

        self.additional_information = additional_information

    def get_figure_of_merits(self):
        if self.finding_efficiency is None:
            overview = read_root(self.filename, tree_key="ExpertMCSideTrackingValidationModule_overview_figures_of_merit")
            self.finding_efficiency = overview.finding_efficiency[0]
            self.hit_efficiency = overview.hit_efficiency[0]

            overview = read_root(self.filename, tree_key="ExpertPRSideTrackingValidationModule_overview_figures_of_merit")
            self.clone_rate = overview.clone_rate[0]
            self.fake_rate = overview.fake_rate[0]

        return dict(finding_efficiency=self.finding_efficiency,
                    hit_efficiency=self.hit_efficiency,
                    clone_rate=self.clone_rate,
                    fake_rate=self.fake_rate)

    def get_figures_of_merit_latex(self):
        results = self.get_figure_of_merits()

        latex_string = r'\begin{table}' + "\n"
        latex_string += r'  \begin{tabular}{cc} \toprule' + "\n"
        latex_string += r'    & \\ \midrule' + "\n"
        latex_string += r'    Finding Efficiency & ' + "%.2f" % (100 * results["finding_efficiency"]) + r' \% \\' + "\n"
        latex_string += r'    Hit Efficiency & ' + "%.2f" % (100 * results["hit_efficiency"]) + r' \% \\' + "\n"
        latex_string += r'    Fake Rate & ' + "%.2f" % (100 * results["fake_rate"]) + r' \% \\' + "\n"
        latex_string += r'    Clone Rate & ' + "%.2f" % (100 * results["clone_rate"]) + r' \% \\ \bottomrule' + "\n"
        latex_string += r'  \end{tabular}' + "\n"
        latex_string += r'\end{table}'

        return latex_string

    def plot_efficiency_point(self):
        self.plot(100 * self.finding_efficiency, 100 * self.hit_efficiency, loc=3)
        plt.xlabel("finding efficiency")
        plt.ylabel("hit efficiency")

    def grouped_by_pt_data(self, mc_data=None):
        if mc_data is None:
            mc_data = self.mc_data

        pt_values = pd.cut(mc_data.pt_truth, np.linspace(mc_data.pt_truth.min(), mc_data.pt_truth.max(), 10))
        grouped = mc_data.groupby(pt_values)

        return grouped

    def plot(self, data_x, data_y, loc=4, yerr=None):
        if yerr is not None:
            plt.errorbar(data_x, data_y, ls="-", marker="o",
                         color=self.color, label=self.label, yerr=yerr)
        else:
            plt.plot(data_x, data_y, ls="-", marker="o",
                     color=self.color, label=self.label)

        if self.label is not None:
            plt.legend(loc=loc)

    def plot_finding_efficiency(self, data=None):
        grouped = self.grouped_by_pt_data(data)

        self.plot(grouped.median().pt_truth, grouped.mean().is_matched, yerr=grouped.std().is_matched)
        plt.xlabel("pt")
        plt.ylabel("finding efficiency")

    def plot_hit_efficiency(self, data=None):
        grouped = self.grouped_by_pt_data(data)

        self.plot(grouped.median().pt_truth, grouped.mean().hit_efficiency, yerr=grouped.std().hit_efficiency)
        plt.xlabel("pt")
        plt.ylabel("hit efficiency")

    def print_useful_information(self):
        pr_data = self.pr_data
        mc_data = self.mc_data
        primaries = pr_data[self.pr_data.is_prompt == 1]
        primaries_mc = mc_data[self.mc_data.is_prompt == 1]

        print self.label
        print "Fake", 100 * primaries.is_fake.mean(), 100 * pr_data.is_fake.mean()
        print "Clone", 100 * primaries.is_clone.mean(), 100 * pr_data.is_clone.mean()
        print "Ghost", 100 * primaries.is_ghost.mean(), 100 * pr_data.is_ghost.mean()
        print "Fitted", 100 * primaries.is_fitted.mean(), 100 * pr_data.is_fitted.mean()
        print "Found", 100 * primaries_mc.is_matched.mean(), 100 * mc_data.is_matched.mean()
        print "Found2", 100.0 - 100 * primaries_mc.is_missing.mean(), 100.0 - 100 * mc_data.is_missing.mean()
        print "Merged", 100 * primaries_mc.is_merged.mean(), 100 * mc_data.is_merged.mean()
        print "Hit-Eff", 100 * primaries_mc.hit_efficiency.mean(), 100 * mc_data.hit_efficiency.mean()
        print "Wrong Hits", primaries.number_of_wrong_hits.mean(), pr_data.number_of_wrong_hits.mean()

    def append_to_dataframe(self, df):
        result = {"finding_efficiency": self.finding_efficiency,
                  "hit_efficiency": self.hit_efficiency,
                  "clone_rate": self.clone_rate,
                  "fake_rate": self.fake_rate,
                  "file_name": self.filename}
        if self.additional_information:
            result.update(self.additional_information)
        return df.append(result, ignore_index=True)


class TMVAPlotter():

    def __init__(self):
        pass

    def grouper(self, data, truth_value, non_truth_value, truth_column):
        if data.size == 0:
            return None
        if (data[truth_column] == 1).all():
            return truth_value
        else:
            return non_truth_value

    def hatcher(self, X, truth_column):
        return self.grouper(X, "//", "", truth_column)

    def filler(self, X, truth_column):
        return self.grouper(X, False, True, truth_column)

    def labeler(self, X, truth_column):
        return self.grouper(X, "signal", "background", truth_column)

    def plot_splitted(self, data, column_name, title=None, truth_column="truth"):
        grouped = data.groupby(data[truth_column])
        for name, X in grouped:
            X[column_name].hist(normed=True, histtype="bar", hatch=self.hatcher(X, truth_column),
                                fill=self.filler(X, truth_column), label=self.labeler(X, truth_column))

        plt.title(title)
        plt.legend()

tmvaPlotter = TMVAPlotter()
