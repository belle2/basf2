#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Transformation classes

In this file all classes for the transformation methods are defined.
The base class is Transform.


"""
__author__ = 'swehle'


from .settings import ProTool

import numpy as np
import pandas as pd
from scipy.interpolate import InterpolatedUnivariateSpline


class Transform(ProTool):

    """
    Base Class for the transformations.
    The function _fit() is overwritten by the sub classes.

    Attributes
    ----------
    n_bins : int, optional
        Binning in x, will be set automatically
    max : float
        Maximum of the fitted distribution
    min : float
        Minimum of the fitted distribution
    is_processed : bool
        Status flag
    name : str
        Name of the transformation

    """

    def __init__(self, name="Original", n_bins=None):
        """ Init function

        :param name:    Name
        :param n_bins:  Binning for the transformations
        """
        #: Binning in x, will be set automatically
        self.n_bins = n_bins

        #: Maximum of the fitted distribution
        self.max = 0

        #: Minimum of the fitted distribution
        self.min = 0

        #: Status flag
        self.is_processed = False

        #: Name of the transformation
        self.name = name

        ProTool.__init__(self, "Transform." + self.name)

    def _initialise(self, x):
        """
        Sets limits for the data.
        Not called by the user.

        :param x: array type
        """
        self.io.debug("Initiating " + self.name)
        if self.n_bins is None:
            self.set_n_bins(len(x))
        self.max = np.max(x)
        self.min = np.min(x)

    def fit(self, x, y=None):
        """
        The fit function is calls the individual _fit() functions.

        :param x:   Distribution to fit, array type
        :param y:   optional for some transformations, sets signal class
        """
        self._initialise(x)
        self._fit(x, y)
        self.is_processed = True

    def __call__(self, x):
        """ Call function calls transform
        :param x:   Input data
        :return:    Transformed data
        """
        return self.transform(x)

    def _fit(self, x, y=None):
        """
        This is defined in the children and overwritten.
        :param x:   array x values
        :param y:   class variable [1,0]

        """

    def transform(self, x, set_limits=False):
        """
        This is defined in the children and overwritten.
        :param x:           Distribution to transform, array type
        :param set_limits:  Limits the range of the data to the fitted range
        :return:            Transformed data
        """
        if set_limits:
            self.set_limits(x)
        return self._transform(x)

    def _transform(self, x):
        """
        This is defined in the children and overwritten.
        In the base class it does nothing and returns the original distribution.

        :param x:   Distribution to transform, array type
        :return:    Transformed data
        """
        return x

    def set_n_bins(self, n):
        """
        Calculates the optimal size for the binning.
        :param n:   Length of the input data
        """
        self.n_bins = get_optimal_bin_size(n)
        self.io.debug("Bins are set to " + str(self.n_bins) + "\t " + str(n / float(self.n_bins)) + "per bin")

    def set_limits(self, x):
        """
        Limits the data to the fitted range.
        :param x:   Input data
        :return:    Limited data
        """
        try:
            _ = len(x)  # to catch exception
            x[x > self.max] = self.max
            x[x < self.min] = self.min
        except TypeError:
            if x < self.min:
                x = self.min
            if x > self.max:
                x = self.max
        return x


def get_optimal_bin_size(n):
    """
    This function calculates the optimal amount of bins for the number of events n.
    :param      n:  number of Events
    :return:        optimal bin size

    """
    return int(3 * n ** (1 / 3.0))


def get_average_in_bins(n):
    """
    Returns the expected amount of entries in each bins.
    :param n:   Length of the data
    :return:    Length of the data divided by the optimal bin size
    """
    return n / float(get_optimal_bin_size(n))


class CDF(Transform):

    """
    Calculates the cumulative distribution (CDF)
    Can be used for the flat transformation.

    Attributes
    ----------
    spline : InterpolatedUnivariateSpline
        Spline, fitting the CDF

    """

    def __init__(self, *args):
        """ Init function

        :param args: None
        """
        Transform.__init__(self, "CDF", *args)

        #: Spline, fitting the CDF
        self.spline = None

    def _fit(self, x, y=None):
        """
        Fit function calculates the cumulative distribution with numpy percentile.

        :param x:   Input distribution
        :param y:   Will not be used in this transformation
        """
        self.io.debug("Fitting CDF")
        y_ = np.linspace(0, 100, 2 * self.n_bins)
        x_ = pd.Series(np.percentile(x, list(y_)))

        # Count same values
        vc = x_.value_counts()
        vc = vc.sort_index()

        # replace same values
        for i, xi in enumerate(vc):
            if xi > 1:
                try:
                    nex_val = vc.index[i + 1]
                except IndexError:
                    nex_val = vc.index[i] + 0.01
                fill = np.linspace(vc.index[i], nex_val, xi)
                x_[x_ == vc.index[i]] = fill
        self.spline = InterpolatedUnivariateSpline(x_, y_)

    def _transform(self, x):
        """
        Transforms the input data according to the cdf.
        :param x:   Input data
        :return:    Transformed data
        """
        x = self.set_limits(x)
        return self.spline(x)


class ToFlat(Transform):

    """
    This transformation uses the CDF to transform input data to a
    flat transformation.

    Attributes
    ----------
    cdf : Transform.CDF
        Transformation with the CDF

    """

    def __init__(self, *args):
        """ Init function

        :param args: None
        """
        Transform.__init__(self, "Flat", *args)

        #: Transformation with the CDF
        self.cdf = CDF(*args)

    def _fit(self, x, y=None):
        """
        Fit function calculates the cumulative distribution with numpy percentile.

        :param x:   Inout distribution
        :param y:   Will not be used in this transformation
        """
        self.io.debug("Fitting Flat")
        self.cdf.fit(x)

    def _transform(self, x):
        """
        Transforms the input data according to the cdf.
        :param x:   Input data
        :return:    Transformed data
        """
        if not self.is_processed:
            self.fit(x)
        return self.cdf.transform(x)

    def get_flat_bins(self):
        """
        Returns the binning of the CDF
        :return: Binning for a flat distribution
        """
        return self.cdf.x

    def get_x(self, x_flat):
        """
        Dirty version for getting the original x value out of a flat x value.
        :param x_flat:  x value in the flat distribution
        :return:        x value on the original axis (approx)
        """
        x_cum = np.linspace(self.min, self.max, self.n_bins * 50)
        for xx in x_cum:
            if self.cdf.spline(xx) > x_flat:
                return xx
