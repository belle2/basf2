import numpy as np
import os
from root_pandas import read_root

import pandas as pd


class TrackingValidationResult:
    # Plotting imports

    #: This class represents a loaded validation root file.
    #: It has methods for plotting the mostly needed graphs

    def __init__(self, filename, label=None, color_index=0, additional_information=None):
        #: The root filename
        self.filename = filename
        #: The pr data
        self.pr_data = read_root(self.filename, tree_key="pr_tree/pr_tree")
        self.pr_data["is_prompt"] = (
            np.sqrt(self.pr_data.x_truth ** 2 + self.pr_data.y_truth ** 2) < 0.5) & (self.pr_data.is_primary == 1)
        #: the mc data
        self.mc_data = read_root(self.filename, tree_key="mc_tree/mc_tree")
        self.mc_data["is_prompt"] = (
            np.sqrt(self.mc_data.x_truth ** 2 + self.mc_data.y_truth ** 2) < 0.5) & (self.mc_data.is_primary == 1)

        #: the mc prompt data
        self.mc_prompts = self.mc_data[self.mc_data.is_prompt == 1]
        #: the pr prompt data
        self.pr_prompts = self.pr_data[self.pr_data.is_prompt == 1]

        import seaborn as sb

        colors = sb.color_palette()

        #: the label
        self.label = label
        #: the color index
        self.color = colors[color_index % len(colors)]

        #: the finding efficiency
        self.finding_efficiency = None
        #: the hit efficiency
        self.hit_efficiency = None
        #: the fake rate
        self.fake_rate = None
        #: the clone rate
        self.clone_rate = None
        self.get_figure_of_merits()

        #: the additional information
        self.additional_information = additional_information

    @staticmethod
    def from_calculations(calculations, key="output_file_name", parameter_part=None):
        if parameter_part:
            return [
                TrackingValidationResult(
                    c.get(key),
                    label=c.get_parameters()[parameter_part],
                    color_index=i) for i,
                c in enumerate(calculations)]
        else:
            return [
                TrackingValidationResult(
                    c.get(key),
                    label=c.get_parameters(),
                    color_index=i) for i,
                c in enumerate(calculations)]

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
        import matplotlib.pyplot as plt
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
        import matplotlib.pyplot as plt
        if yerr is not None:
            plt.errorbar(data_x, data_y, ls="-", marker="o",
                         color=self.color, label=self.label, yerr=yerr, lw=4)
        else:
            plt.plot(data_x, data_y, ls="-", marker="o",
                     color=self.color, label=self.label, lw=4)

        if self.label is not None:
            plt.legend(loc=loc, frameon=True)

    def plot_finding_efficiency(self, data=None):
        import matplotlib.pyplot as plt
        grouped = self.grouped_by_pt_data(data)

        self.plot(grouped.median().pt_truth, grouped.mean().is_matched, yerr=1 / np.sqrt(grouped.count().is_matched))
        plt.xlabel(r"$p_T$ of the MC tracks (in GeV)")
        plt.ylabel("Finding Efficiency")

    def plot_hit_efficiency(self, data=None):
        import matplotlib.pyplot as plt
        grouped = self.grouped_by_pt_data(data)

        self.plot(grouped.median().pt_truth, grouped.mean().hit_efficiency, yerr=1 / np.sqrt(grouped.sum().mc_number_of_hits))
        plt.xlabel(r"$p_T$ of the MC tracks (in GeV)")
        plt.ylabel("Hit Efficiency")

    def print_useful_information(self):
        pr_data = self.pr_data
        mc_data = self.mc_data
        primaries = pr_data[self.pr_data.is_prompt == 1]
        primaries_mc = mc_data[self.mc_data.is_prompt == 1]

        print(self.label)
        print("Fake", 100 * primaries.is_fake.mean(), 100 * pr_data.is_fake.mean())
        print("Clone", 100 * primaries.is_clone.mean(), 100 * pr_data.is_clone.mean())
        print("Ghost", 100 * primaries.is_ghost.mean(), 100 * pr_data.is_ghost.mean())
        print("Fitted", 100 * primaries.is_fitted.mean(), 100 * pr_data.is_fitted.mean())
        print("Found", 100 * primaries_mc.is_matched.mean(), 100 * mc_data.is_matched.mean())
        print("Found2", 100.0 - 100 * primaries_mc.is_missing.mean(), 100.0 - 100 * mc_data.is_missing.mean())
        print("Merged", 100 * primaries_mc.is_merged.mean(), 100 * mc_data.is_merged.mean())
        print("Hit-Eff", 100 * primaries_mc.hit_efficiency.mean(), 100 * mc_data.hit_efficiency.mean())
        print("Wrong Hits", primaries.number_of_wrong_hits.mean(), pr_data.number_of_wrong_hits.mean())

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
