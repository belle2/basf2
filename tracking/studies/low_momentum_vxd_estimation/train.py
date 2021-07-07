##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import pandas as pd
import numpy as np

from . import fit_functions
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt

from sklearn import tree


class DEDXEstimationTrainer:
    """Train a neural network for dE/dx-based particle identification"""

    def __init__(self):
        """Constructor"""
        #: by default, the dE/dx-particle-identification trainer has not run yet
        self.dedx_estimator_function = None
        #: the default data column is 'dedx'
        self.dedx_column = "dedx"

    def train(self, data):
        """Train on the input data"""
        # We have everything
        raise NotImplementedError("Use this class as a base class only")

    def test(self, data):
        """Get the trained neural-network output value for test data"""
        if self.dedx_estimator_function is None:
            raise ValueError("Train the estimator first!")

        return self.dedx_estimator_function(data[self.dedx_column])


class GroupedDEDXEstimationTrainer(DEDXEstimationTrainer):
    """Train a neural network for dE/dx-based particle identification"""

    #: number of dE/dx bins
    number_of_bins_in_dedx = 20
    #: number of track-momentum bins
    number_of_bins_in_p = 29
    #: number of head values in fit
    number_of_head_values_used_to_fit = 20

    def create_dedx_bins(self, data):
        """Construct the dE/dx bins and then populate them with the data"""
        dedx_bins = np.linspace(
            data[
                self.dedx_column].min(), data[
                self.dedx_column].max(), GroupedDEDXEstimationTrainer.number_of_bins_in_dedx)
        dedx_cuts = pd.cut(data[self.dedx_column], dedx_bins)
        return data.groupby(dedx_cuts), dedx_bins

    def create_p_bins(self, data):
        """Construct the momentum bins and then populate them with the data"""
        p_bins = np.linspace(data.p.min(), data.p.max(), GroupedDEDXEstimationTrainer.number_of_bins_in_p)
        p_cuts = pd.cut(data.p, p_bins)
        return data.groupby(p_cuts), p_bins

    def use_only_the_highest_values(self, data, number_of_values=None):
        """Sort the data then select only the highest N values"""
        if number_of_values is None:
            return data
        else:
            return data.sort("number_of_p_values", ascending=False).head(number_of_values).sort()

    def create_fit_data(self, dedx_bin):
        """Fit track-momentum values"""
        p_binned_data, p_bins = self.create_p_bins(dedx_bin)

        number_of_p_values = pd.Series(p_binned_data.count().p.values, name="number_of_p_values")
        p_bin_centers = pd.Series(0.5 * (p_bins[:-1] + p_bins[1:]), name="p_bin_centers")

        all_fit_data = pd.DataFrame([number_of_p_values, p_bin_centers]).T
        fit_data = self.use_only_the_highest_values(all_fit_data, GroupedDEDXEstimationTrainer.number_of_head_values_used_to_fit)

        return fit_data

    def fit_p_to_dedx_bin(self, dedx_bin):
        """Fit the track-momentum values in the selected dE/dx bin, then train on the fitted values"""
        fit_data = self.create_fit_data(dedx_bin)
        return self.train_function(fit_data)


class FittedGroupedDEDXEstimatorTrainer(GroupedDEDXEstimationTrainer):
    """Train a neural network for dE/dx-based particle identification"""

    def __init__(self, result_function, use_sigma_for_result_fitting):
        """Constructor"""

        #: cached copy of the result function
        self.result_function = result_function
        #: cached copy of the dictionary of fitting parameters for each dE/dx bin
        self.result_parameters_for_each_dedx_bin = {}
        #: cached copy of the flag to add mean+/-sigma values to the output Dataframe
        self.use_sigma_for_result_fitting = use_sigma_for_result_fitting

        GroupedDEDXEstimationTrainer.__init__(self)

    def create_result_dataframe(self):
        """Fit for the mean dE/dx and standard deviation, return the fit Dataframe"""
        result_df = pd.DataFrame([{"dedx_bin_center": dedx_bin_center,
                                   "mu": fit_parameters[1][1],
                                   "sigma": fit_parameters[0]} for dedx_bin_center,
                                  fit_parameters in self.result_parameters_for_each_dedx_bin.items()
                                  if fit_parameters is not None])

        if len(result_df) == 0:
            raise ValueError("Could not find any fitted parameters!")

        if self.use_sigma_for_result_fitting:
            result_df["mu_plus_sigma"] = result_df.mu + result_df.sigma
            result_df["mu_minus_sigma"] = result_df.mu - result_df.sigma

        result_df.sort("dedx_bin_center", inplace=True)

        return result_df

    def fit_result_parameters(self):
        """Define the parameters for the fit, assign initial guesses"""
        result_df = self.create_result_dataframe()

        p0 = (7e+08, -4e+04, 0.1, 0)

        if self.use_sigma_for_result_fitting:
            popt, pcov = curve_fit(self.result_function, result_df.dedx_bin_center, result_df.mu, p0=p0,
                                   sigma=result_df.sigma, absolute_sigma=True)
        else:
            popt, pcov = curve_fit(self.result_function, result_df.dedx_bin_center, result_df.mu, p0=p0)

        return popt, lambda dedx: self.result_function(dedx, *popt)

    def train(self, data):
        """Train the neural network using curated data"""
        dedx_binned_data, dedx_bins = self.create_dedx_bins(data)

        def fit_and_save_results(dedx_bin):
            fit_result = self.fit_p_to_dedx_bin(dedx_bin)
            return {dedx_bin.mean()[self.dedx_column]: fit_result}

        for result in dedx_binned_data.apply(fit_and_save_results):
            self.result_parameters_for_each_dedx_bin.update(result)

        #: cached copies of the fit parameters and estimator function
        self.dedx_estimator_parameters, self.dedx_estimator_function = self.fit_result_parameters()

    def plot_fit_result(self, data):
        """Plot the fitted results"""
        plot_dedx_data = np.linspace(data[self.dedx_column].min(), data[self.dedx_column].max(), 100)
        result_df = self.create_result_dataframe()

        plt.plot(plot_dedx_data, self.dedx_estimator_function(plot_dedx_data), color="black", label="Fitted estimator")
        if self.use_sigma_for_result_fitting:
            # color = "black"
            plt.errorbar(result_df.dedx_bin_center, result_df.mu, marker="o", ls="", label="Data Points", yerr=result_df.sigma)

        plt.ylim(0, 0.14)
        plt.xlabel("dEdX in ADC count/cm")
        plt.ylabel("p in GeV/c")
        plt.legend(frameon=True)

    def plot_grouped_result(self, data):
        """Plot the fitted grouped results"""
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
    """Train a neural network for dE/dx-based particle identification"""

    def __init__(self, fit_function, dimension_of_fit_function, result_function, use_sigma_for_result_fitting):
        """Constructor"""

        #: cached value of the degrees of freedom in the fit
        self.dimension_of_fit_function = dimension_of_fit_function
        #: cached copy of the fitting function
        self.fit_function = fit_function

        FittedGroupedDEDXEstimatorTrainer.__init__(self, result_function, use_sigma_for_result_fitting)

        def train_function(fit_data):
            """Train on the fit to curated-data highest values whose truth value is known"""
            max_value = self.use_only_the_highest_values(fit_data, 1).p_bin_centers.values[0]

            if self.dimension_of_fit_function == 3:
                p0 = (1e3, max_value, 4e-2)
            elif self.dimension_of_fit_function == 6:
                p0 = (1e3, max_value, 4e-2, 1, 1, 1)

            popt, pcov = curve_fit(self.fit_function, fit_data.p_bin_centers, fit_data.number_of_p_values, p0=p0)

            return [np.sqrt(np.diag(pcov)[1]), popt]

        #: this class's training function
        self.train_function = train_function

    def plot_grouped_result(self, data):
        """Plot the fitted grouped results"""
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
    """Train a neural network for dE/dx-based particle identification using a Gaussian estimator"""

    def __init__(self):
        """Constructor"""
        FunctionFittedGroupedDEDXEstimatorTrainer.__init__(
            self,
            fit_functions.norm,
            3,
            fit_functions.inverse_squared,
            use_sigma_for_result_fitting=True)


class LandauEstimatorTrainer(FunctionFittedGroupedDEDXEstimatorTrainer):
    """Train a neural network for dE/dx-based particle identification using a Landau estimator"""

    def __init__(self):
        """Constructor"""
        FunctionFittedGroupedDEDXEstimatorTrainer.__init__(
            self,
            fit_functions.landau,
            3,
            fit_functions.inverse_squared,
            use_sigma_for_result_fitting=True)


class MaximumEstimatorTrainer(FittedGroupedDEDXEstimatorTrainer):
    """Train a neural network for dE/dx-based particle identification using only the highest values"""

    def __init__(self):
        """Constructor"""
        FittedGroupedDEDXEstimatorTrainer.__init__(self, fit_functions.inverse_squared, use_sigma_for_result_fitting=False)

        def train_function(fit_data):
            """Train on the curated-data highest values whose truth value is known"""
            max_value = self.use_only_the_highest_values(fit_data, 1).p_bin_centers.values[0]

            return [None, [None, max_value, None]]

        #: this class's training function
        self.train_function = train_function


class MedianEstimatorTrainer(FittedGroupedDEDXEstimatorTrainer):
    """Train a neural network for dE/dx-based particle identification using only the median values"""

    def __init__(self):
        """Constructor"""
        FittedGroupedDEDXEstimatorTrainer.__init__(self, fit_functions.inverse_squared, use_sigma_for_result_fitting=True)

        def train_function(fit_data):
            """Train on the curated-data median values whose truth value is known"""
            weighted_p_values = fit_data.apply(lambda data: [data.p_bin_centers] * int(data.number_of_p_values), axis=1).sum()
            median_value = np.median(weighted_p_values)
            iqr = np.percentile(weighted_p_values, 75) - np.percentile(weighted_p_values, 50)

            return [iqr, [None, median_value, None]]

        #: this class's training function
        self.train_function = train_function


class GaussianEstimatorTrainerSQRT(FunctionFittedGroupedDEDXEstimatorTrainer):
    """Train a neural network for dE/dx-based particle identification using a Gaussian estimator"""

    def __init__(self):
        """Constructor"""
        FunctionFittedGroupedDEDXEstimatorTrainer.__init__(
            self,
            fit_functions.norm,
            3,
            fit_functions.inverse_sqrt,
            use_sigma_for_result_fitting=True)


class LandauEstimatorTrainerSQRT(FunctionFittedGroupedDEDXEstimatorTrainer):
    """Train a neural network for dE/dx-based particle identification using a Landau estimator"""

    def __init__(self):
        """Constructor"""
        FunctionFittedGroupedDEDXEstimatorTrainer.__init__(
            self,
            fit_functions.landau,
            3,
            fit_functions.inverse_sqrt,
            use_sigma_for_result_fitting=True)


class MaximumEstimatorTrainerSQRT(FittedGroupedDEDXEstimatorTrainer):
    """Train a neural network for dE/dx-based particle identification using only the highest values"""

    def __init__(self):
        """Constructor"""
        FittedGroupedDEDXEstimatorTrainer.__init__(self, fit_functions.inverse_sqrt, use_sigma_for_result_fitting=False)

        def train_function(fit_data):
            """Train on the curated-data highest values whose truth value is known"""
            max_value = self.use_only_the_highest_values(fit_data, 1).p_bin_centers.values[0]

            return [None, [None, max_value, None]]

        #: this class's training function
        self.train_function = train_function


class MedianEstimatorTrainerSQRT(FittedGroupedDEDXEstimatorTrainer):
    """Train a neural network for dE/dx-based particle identification using only the median values"""

    def __init__(self):
        """Constructor"""
        FittedGroupedDEDXEstimatorTrainer.__init__(self, fit_functions.inverse_sqrt, use_sigma_for_result_fitting=True)

        def train_function(fit_data):
            """Train on the curated-data median values whose truth value is known"""
            weighted_p_values = fit_data.apply(lambda data: [data.p_bin_centers] * int(data.number_of_p_values), axis=1).sum()
            median_value = np.median(weighted_p_values)
            iqr = np.percentile(weighted_p_values, 75) - np.percentile(weighted_p_values, 50)

            return [iqr, [None, median_value, None]]

        #: this class's training function
        self.train_function = train_function


class MVADEDXEstimationTrainer(DEDXEstimationTrainer):
    """Train a neural network for dE/dx-based particle identification using multivariate data analysis"""

    def __init__(self):
        """Constructor"""

        #: cached copy of the MVA tool
        self.tree = tree.DecisionTreeRegressor()
        DEDXEstimationTrainer.__init__(self)

    def train(self, data):
        """Train the neural network using curated data"""

        train_data = data.copy()
        del train_data["p"]

        p_values = data["p"]

        self.tree.fit(train_data.values, p_values.values)

    def test(self, data):
        """Get the trained neural-network output value for test data"""

        test_data = data.copy()
        del test_data["p"]

        return self.tree.predict(test_data.values)
