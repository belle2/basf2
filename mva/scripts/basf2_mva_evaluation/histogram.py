#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2015

import numpy


def binom_error(n_sig, n_tot):
    """
    for an efficiency = nSig/nTrueSig or purity = nSig / (nSig + nBckgrd), this function calculates the
    standard deviation according to http://arxiv.org/abs/physics/0701199 .
    """
    variance = numpy.where(n_tot > 0, (n_sig + 1) * (n_sig + 2) / ((n_tot + 2) * (n_tot + 3)) -
                           (n_sig + 1) ** 2 / ((n_tot + 2) ** 2), 0)
    return numpy.sqrt(variance)


def poisson_error(n_tot):
    """
    use poisson error, except for 0 we use an 68% CL upper limit
    """
    return numpy.where(n_tot > 0, numpy.sqrt(n_tot), numpy.log(1.0 / (1 - 0.6827)))


def weighted_mean_and_std(x, w):
    """
    Return the weighted average and standard deviation.
    @param x values
    @param w weights
    """
    mean = numpy.average(x, weights=w)
    var = numpy.average((x-mean)**2, weights=w)
    return (mean, numpy.sqrt(var))


class Histograms(object):
    """
    Extracts information from a pandas.DataFrame and stores it
    in a binned format.
    Therefore the size independent from the size of the pandas.DataFrame.
    Used by the plotting routines below.
    """

    #: Histogram of the full data
    hist = None
    #: Binning
    bins = None
    #: Bin centers
    bin_centers = None
    #: Bin widths
    bin_widths = None
    #: Dictionary of histograms for the given masks
    hists = None

    def __init__(self, data, column, masks=dict(), weight_column=None, bins=100, equal_frequency=True, range_in_std=None):
        """
        Creates a common binning of the given column of the given pandas.Dataframe,
        and stores for each given mask the histogram of the column
        @param data pandas.DataFrame  like object containing column and weight_column
        @param column string identifiying the column in the pandas.DataFrame which is binned.
        @param masks dictionary of names and boolean arrays, which select the data
                     used for the creation of histograms with these names
        @param weight_column identifiying the column in the pandas.DataFrame which is used as weight
        @param bins use given bins instead of default 100
        @param equal_frequency perform an equal_frequency binning
        @param range_in_std show only the data in a windows around +- range_in_std * standard_deviation around the mean
        """
        isfinite = numpy.isfinite(data[column])
        if range_in_std is not None:
            mean, std = weighted_mean_and_std(data[column][isfinite],
                                              None if weight_column is None else data[weight_column][isfinite])
            # Everything outside mean +- range_in_std * std is considered infinite
            isfinite = isfinite & (data[column] > (mean - range_in_std * std)) & (data[column] < (mean + range_in_std * std))

        if equal_frequency:
            if data[column][isfinite].size > 0:
                bins = numpy.unique(numpy.percentile(data[column][isfinite], q=range(bins + 1)))
            else:
                print('Empty Array')
                bins = [1]
            # If all values are unique, we make at least one bin
            if len(bins) == 1:
                bins = numpy.array([bins[0]-1, bins[0]+1])

        self.hist, self.bins = numpy.histogram(data[column][isfinite], bins=bins,
                                               weights=None if weight_column is None else data[weight_column])
        self.bin_centers = (self.bins + numpy.roll(self.bins, 1))[1:] / 2.0
        # Subtract a small number from the bin width, otherwise the errorband plot is unstable.
        self.bin_widths = (self.bins - numpy.roll(self.bins, 1))[1:] - 0.00001
        self.hists = dict()
        for name, mask in masks.items():
            self.hists[name] = numpy.histogram(data[column][mask & isfinite], bins=self.bins,
                                               weights=None if weight_column is None else data[weight_column][mask & isfinite])[0]

    def get_hist(self, name=None):
        """
        Return histogram with the given name. If none returns histogram of the full data.
        @param name name of the histogram
        @return numpy.array with hist data, numpy.array with corresponding poisson errors
        """
        if name is None:
            return self.hist, poisson_error(self.hist)
        return self.get_summed_hist([name])

    def get_summed_hist(self, names):
        """
        Return the sum of histograms with the given names.
        @param names names of the histograms
        @return numpy.array with hist data, numpy.array with corresponding poisson errors
        """
        default = numpy.zeros(len(self.bin_centers))
        hist = numpy.sum(self.hists.get(v, default) for v in names)
        hist_error = poisson_error(hist)
        return hist, hist_error

    def get_efficiency(self, signal_names):
        """
        Return the cumulative efficiency in each bin of the sum of the histograms with the given names.
        @param  signal_names of the histograms
        @return numpy.array with hist data, numpy.array with corresponding binomial errors
        """
        signal, _ = self.get_summed_hist(signal_names)
        cumsignal = (signal.sum() - signal.cumsum()).astype('float')

        efficiency = 0
        efficiency_error = 0
        if signal.sum() > 0:
            efficiency = cumsignal / signal.sum()
            efficiency_error = binom_error(cumsignal, signal.sum())
        return efficiency, efficiency_error

    def get_true_positives(self, signal_names):
        """
        Return the cumulative true positives in each bin of the sum of the histograms with the given names.
        @param names names of the histograms
        @return numpy.array with hist data, numpy.array with corresponding binomial errors
        """
        signal, _ = self.get_summed_hist(signal_names)
        cumsignal = (signal.sum() - signal.cumsum()).astype('float')
        signal_error = poisson_error(cumsignal)
        return cumsignal, signal_error

    def get_false_positives(self, bckgrd_names):
        """
        Return the cumulative false positives in each bin of the sum of the histograms with the given names.
        @param names names of the histograms
        @return numpy.array with hist data, numpy.array with corresponding binomial errors
        """
        background, _ = self.get_summed_hist(bckgrd_names)
        cumbackground = (background.sum() - background.cumsum()).astype('float')
        background_error = poisson_error(cumbackground)
        return cumbackground, background_error

    def get_purity(self, signal_names, bckgrd_names):
        """
        Return the cumulative purity in each bin of the sum of the histograms with the given names.
        @param names names of the histograms
        @return numpy.array with hist data, numpy.array with corresponding binomial errors
        """
        signal, _ = self.get_summed_hist(signal_names)
        bckgrd, _ = self.get_summed_hist(bckgrd_names)
        cumsignal = (signal.sum() - signal.cumsum()).astype('float')
        cumbckgrd = (bckgrd.sum() - bckgrd.cumsum()).astype('float')

        purity = cumsignal / (cumsignal + cumbckgrd)
        purity_error = binom_error(cumsignal, cumsignal + cumbckgrd)
        return purity, purity_error

    def get_signal_to_noise(self, signal_names, bckgrd_names):
        """
        Return the cumulative signal to noise ratio in each bin of the sum of the histograms with the given names.
        @param names names of the histograms
        @return numpy.array with hist data, numpy.array with corresponding binomial errors
        """
        signal, _ = self.get_summed_hist(signal_names)
        bckgrd, _ = self.get_summed_hist(bckgrd_names)
        cumsignal = (signal.sum() - signal.cumsum()).astype('float')
        cumbckgrd = (bckgrd.sum() - bckgrd.cumsum()).astype('float')

        signal2noise = cumsignal / (cumsignal + cumbckgrd)**0.5
        signal2noise_error = numpy.sqrt(cumsignal / (cumsignal + cumbckgrd) + (cumsignal / (2 * (cumsignal + cumbckgrd)))**2)
        return signal2noise, signal2noise_error

    def get_purity_per_bin(self, signal_names, bckgrd_names):
        """
        Return the purity in each bin of the sum of the histograms with the given names.
        @param names names of the histograms
        @return numpy.array with hist data, numpy.array with corresponding binomial errors
        """
        signal, _ = self.get_summed_hist(signal_names)
        bckgrd, _ = self.get_summed_hist(bckgrd_names)
        signal = signal.astype('float')
        bckgrd = bckgrd.astype('float')

        purity = signal / (signal + bckgrd)
        purity_error = binom_error(signal, signal + bckgrd)
        return purity, purity_error
