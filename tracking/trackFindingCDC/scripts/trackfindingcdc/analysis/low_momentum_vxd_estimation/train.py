import pandas as pd
import numpy as np

import fit_functions
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt

from sklearn import tree


class DEDXEstimationTrainer:

    def __init__(self):
        self.dedx_estimator_function = None
        self.dedx_column = "dedx"

    def train(self, data):
        # We have everything
        raise NotImplementedError("Use this class as a base class only")

    def test(self, data):
        if self.dedx_estimator_function is None:
            raise ValueError("Train the estimator first!")

        return self.dedx_estimator_function(data[self.dedx_column])


class GroupedDEDXEstimationTrainer(DEDXEstimationTrainer):
    number_of_bins_in_dedx = 20
    number_of_bins_in_p = 29
    number_of_head_values_used_to_fit = 20

    def create_dedx_bins(self, data):
        dedx_bins = np.linspace(
            data[
                self.dedx_column].min(), data[
                self.dedx_column].max(), GroupedDEDXEstimationTrainer.number_of_bins_in_dedx)
        dedx_cuts = pd.cut(data[self.dedx_column], dedx_bins)
        return data.groupby(dedx_cuts), dedx_bins

    def create_p_bins(self, data):
        p_bins = np.linspace(data.p.min(), data.p.max(), GroupedDEDXEstimationTrainer.number_of_bins_in_p)
        p_cuts = pd.cut(data.p, p_bins)
        return data.groupby(p_cuts), p_bins

    def use_only_the_highest_values(self, data, number_of_values=None):
        if number_of_values is None:
            return data
        else:
            return data.sort("number_of_p_values", ascending=False).head(number_of_values).sort()

    def create_fit_data(self, dedx_bin):
        p_binned_data, p_bins = self.create_p_bins(dedx_bin)

        number_of_p_values = pd.Series(p_binned_data.count().p.values, name="number_of_p_values")
        p_bin_centers = pd.Series(0.5 * (p_bins[:-1] + p_bins[1:]), name="p_bin_centers")

        all_fit_data = pd.DataFrame([number_of_p_values, p_bin_centers]).T
        fit_data = self.use_only_the_highest_values(all_fit_data, GroupedDEDXEstimationTrainer.number_of_head_values_used_to_fit)

        return fit_data

    def fit_p_to_dedx_bin(self, dedx_bin):
        fit_data = self.create_fit_data(dedx_bin)
        return self.train_function(fit_data)


class FittedGroupedDEDXEstimatorTrainer(GroupedDEDXEstimationTrainer):

    def __init__(self, result_function, use_sigma_for_result_fitting):
        self.result_function = result_function
        self.result_parameters_for_each_dedx_bin = {}
        self.use_sigma_for_result_fitting = use_sigma_for_result_fitting

        GroupedDEDXEstimationTrainer.__init__(self)

    def create_result_dataframe(self):
        result_df = pd.DataFrame([{"dedx_bin_center": dedx_bin_center,
                                   "mu": fit_parameters[1][1],
                                   "sigma": fit_parameters[0]} for dedx_bin_center,
                                  fit_parameters in self.result_parameters_for_each_dedx_bin.iteritems()
                                  if fit_parameters is not None])

        if len(result_df) == 0:
            raise ValueError("Could not find any fitted parameters!")

        if self.use_sigma_for_result_fitting:
            result_df["mu_plus_sigma"] = result_df.mu + result_df.sigma
            result_df["mu_minus_sigma"] = result_df.mu - result_df.sigma

        result_df.sort("dedx_bin_center", inplace=True)

        return result_df

    def fit_result_parameters(self):
        result_df = self.create_result_dataframe()

        p0 = (7e+08, -4e+04, 0.1, 0)

        if self.use_sigma_for_result_fitting:
            popt, pcov = curve_fit(self.result_function, result_df.dedx_bin_center, result_df.mu, p0=p0,
                                   sigma=result_df.sigma, absolute_sigma=True)
        else:
            popt, pcov = curve_fit(self.result_function, result_df.dedx_bin_center, result_df.mu, p0=p0)

        return popt, lambda dedx: self.result_function(dedx, *popt)

    def train(self, data):
        dedx_binned_data, dedx_bins = self.create_dedx_bins(data)

        def fit_and_save_results(dedx_bin):
            fit_result = self.fit_p_to_dedx_bin(dedx_bin)
            return {dedx_bin.mean()[self.dedx_column]: fit_result}

        for result in dedx_binned_data.apply(fit_and_save_results):
            self.result_parameters_for_each_dedx_bin.update(result)

        self.dedx_estimator_parameters, self.dedx_estimator_function = self.fit_result_parameters()

    def plot_fit_result(self, data):
        plot_dedx_data = np.linspace(data[self.dedx_column].min(), data[self.dedx_column].max(), 100)
        result_df = self.create_result_dataframe()

        plt.plot(plot_dedx_data, self.dedx_estimator_function(plot_dedx_data), color="black", label="Fitted estimator")
        if self.use_sigma_for_result_fitting:
            color = "black"
            plt.errorbar(result_df.dedx_bin_center, result_df.mu, marker="o", ls="", label="Data Points", yerr=result_df.sigma)

        plt.ylim(0, 0.14)
        plt.xlabel("dEdX in ADC count/cm")
        plt.ylabel("p in GeV/c")
        plt.legend(frameon=True)

    def plot_grouped_result(self, data):
        dedx_binned_data, dedx_bins = self.create_dedx_bins(data)

        # List to prevent bug in pd.DataFrame.apply
        already_plotted_list = []

        def plot_fitted_results(dedx_bin):
            dedx_bin_center = dedx_bin.mean().values[0]

            if dedx_bin_center not in already_plotted_list:
                already_plotted_list.append(dedx_bin_center)

                fit_data = self.create_fit_data(dedx_bin)
                plt.plot(fit_data.p_bin_centers, fit_data.number_of_p_values, ls="", marker=".", color="black")

            return True

        plt.xlabel("p in GeV/c")
        plt.ylabel("Entries")

        dedx_binned_data.apply(plot_fitted_results)


class FunctionFittedGroupedDEDXEstimatorTrainer(FittedGroupedDEDXEstimatorTrainer):

    def __init__(self, fit_function, dimension_of_fit_function, result_function, use_sigma_for_result_fitting):
        self.dimension_of_fit_function = dimension_of_fit_function
        self.fit_function = fit_function

        FittedGroupedDEDXEstimatorTrainer.__init__(self, result_function, use_sigma_for_result_fitting)

        def train_function(fit_data):
            max_value = self.use_only_the_highest_values(fit_data, 1).p_bin_centers.values[0]

            if self.dimension_of_fit_function == 3:
                p0 = (1e3, max_value, 4e-2)
            elif self.dimension_of_fit_function == 6:
                p0 = (1e3, max_value, 4e-2, 1, 1, 1)

            popt, pcov = curve_fit(self.fit_function, fit_data.p_bin_centers, fit_data.number_of_p_values, p0=p0)

            return [np.sqrt(np.diag(pcov)[1]), popt]

        self.train_function = train_function

    def plot_grouped_result(self, data):
        FittedGroupedDEDXEstimatorTrainer.plot_grouped_result(self, data)

        dedx_binned_data, dedx_bins = self.create_dedx_bins(data)

        p_plot_data = np.linspace(data.p.min(), data.p.max(), 1000)

        # List to prevent bug in pd.DataFrame.apply
        already_plotted_list = []

        def plot_fitted_results(dedx_bin):
            dedx_bin_center = dedx_bin.mean().values[0]

            if dedx_bin_center not in already_plotted_list:
                fitted_results = self.result_parameters_for_each_dedx_bin[dedx_bin.mean()[self.dedx_column]]
                already_plotted_list.append(dedx_bin_center)
                unneeded, fit_options = fitted_results

                dedx_plot_data = self.fit_function(p_plot_data, *fitted_results[1])
                plt.plot(p_plot_data, dedx_plot_data)

            return True

        dedx_binned_data.apply(plot_fitted_results)


class GaussianEstimatorTrainer(FunctionFittedGroupedDEDXEstimatorTrainer):

    def __init__(self):
        FunctionFittedGroupedDEDXEstimatorTrainer.__init__(
            self,
            fit_functions.norm,
            3,
            fit_functions.inverse_squared,
            use_sigma_for_result_fitting=True)


class LandauEstimatorTrainer(FunctionFittedGroupedDEDXEstimatorTrainer):

    def __init__(self):
        FunctionFittedGroupedDEDXEstimatorTrainer.__init__(
            self,
            fit_functions.landau,
            3,
            fit_functions.inverse_squared,
            use_sigma_for_result_fitting=True)


class MaximumEstimatorTrainer(FittedGroupedDEDXEstimatorTrainer):

    def __init__(self):
        FittedGroupedDEDXEstimatorTrainer.__init__(self, fit_functions.inverse_squared, use_sigma_for_result_fitting=False)

        def train_function(fit_data):
            max_value = self.use_only_the_highest_values(fit_data, 1).p_bin_centers.values[0]

            return [None, [None, max_value, None]]

        self.train_function = train_function


class MedianEstimatorTrainer(FittedGroupedDEDXEstimatorTrainer):

    def __init__(self):
        FittedGroupedDEDXEstimatorTrainer.__init__(self, fit_functions.inverse_squared, use_sigma_for_result_fitting=True)

        def train_function(fit_data):
            weighted_p_values = fit_data.apply(lambda data: [data.p_bin_centers] * int(data.number_of_p_values), axis=1).sum()
            median_value = np.median(weighted_p_values)
            iqr = np.percentile(weighted_p_values, 75) - np.percentile(weighted_p_values, 50)

            return [iqr, [None, median_value, None]]

        self.train_function = train_function


class GaussianEstimatorTrainerSQRT(FunctionFittedGroupedDEDXEstimatorTrainer):

    def __init__(self):
        FunctionFittedGroupedDEDXEstimatorTrainer.__init__(
            self,
            fit_functions.norm,
            3,
            fit_functions.inverse_sqrt,
            use_sigma_for_result_fitting=True)


class LandauEstimatorTrainerSQRT(FunctionFittedGroupedDEDXEstimatorTrainer):

    def __init__(self):
        FunctionFittedGroupedDEDXEstimatorTrainer.__init__(
            self,
            fit_functions.landau,
            3,
            fit_functions.inverse_sqrt,
            use_sigma_for_result_fitting=True)


class MaximumEstimatorTrainerSQRT(FittedGroupedDEDXEstimatorTrainer):

    def __init__(self):
        FittedGroupedDEDXEstimatorTrainer.__init__(self, fit_functions.inverse_sqrt, use_sigma_for_result_fitting=False)

        def train_function(fit_data):
            max_value = self.use_only_the_highest_values(fit_data, 1).p_bin_centers.values[0]

            return [None, [None, max_value, None]]

        self.train_function = train_function


class MedianEstimatorTrainerSQRT(FittedGroupedDEDXEstimatorTrainer):

    def __init__(self):
        FittedGroupedDEDXEstimatorTrainer.__init__(self, fit_functions.inverse_sqrt, use_sigma_for_result_fitting=True)

        def train_function(fit_data):
            weighted_p_values = fit_data.apply(lambda data: [data.p_bin_centers] * int(data.number_of_p_values), axis=1).sum()
            median_value = np.median(weighted_p_values)
            iqr = np.percentile(weighted_p_values, 75) - np.percentile(weighted_p_values, 50)

            return [iqr, [None, median_value, None]]

        self.train_function = train_function


class MVADEDXEstimationTrainer(DEDXEstimationTrainer):

    def __init__(self):
        self.tree = tree.DecisionTreeRegressor()
        DEDXEstimationTrainer.__init__(self)

    def train(self, data):

        train_data = data.copy()
        del train_data["p"]

        p_values = data["p"]

        self.tree.fit(train_data.values, p_values.values)

    def test(self, data):

        test_data = data.copy()
        del test_data["p"]

        return self.tree.predict(test_data.values)
