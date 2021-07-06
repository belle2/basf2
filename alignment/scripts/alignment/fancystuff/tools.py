#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

""" Tools collection

In the tools collection all plotting tools are gathered.

"""
__author__ = 'swehle'

from alignment.fancystuff import transform
from alignment.fancystuff.settings import create_figure

import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from scipy.stats import chisqprob
import math


def set_axis_label_range(ax, new_start, new_end, n_labels=5, axis=1, to_flat=None):
    """
    Set the labels to a different range
    :param ax:          axis object
    :param new_start:   New start value
    :param new_end:     New end value
    :param n_labels:    N labels
    :param axis:        default is x axis 1
    :param to_flat:     Flat transformation object for getting non linear values on the axis
    """

    start, end = ax.get_xlim()
    # print start, end
    label_position = np.append(np.arange(start, end, (end - start) / float(n_labels - 1)), end)

    # Wrong linear interploation
    new_labels = np.append(np.arange(new_start, new_end, (new_end - new_start) / float(n_labels - 1)), new_end)

    # None linear 'correct' case using the CDF as reference
    if to_flat is not None:
        assert isinstance(to_flat, transform.ToFlat)
        x_on_flat = np.linspace(0, 1, n_labels)
        new_labels = []

        for x, i in zip(x_on_flat, list(range(0, n_labels))):
            new_labels.append(to_flat.get_x(x))
        new_labels[-1] = to_flat.max
        new_labels[0] = to_flat.min

    if axis == 1:
        ax.set_xticks(label_position)
        ax.set_xticklabels(["%.2f" % i for i in new_labels])
    else:
        ax.set_yticks(label_position)
        ax.set_yticklabels(["%.2f" % i for i in new_labels])


def draw_flat_correlation(x, y, ax=None, draw_label=True, width=5):
    """
    This function draws a flat correlation distribution.
    Both x an y have to be equally sized and are transformed to a flat distribution.

    :param x:           dist x, pandas Series
    :param y:           dist y, pandas Series
    :param ax:          axis object if drawn in a subplot
    :param draw_label:  draw the labels of the distribution (only works with pandas Series)
    :param width:       width of the plot, default 5
    """

    not_on_axes = True if ax is None else False

    if ax is None:
        fig, ax = create_figure(width=width, ratio=7 / 6.)

    assert isinstance(x, pd.Series or np.array), 'Argument of wrong type!'
    assert isinstance(y, pd.Series or np.array), 'Argument of wrong type!'
    x_val = x.values
    y_val = y.values

    # Flat Distribution
    tx = transform.ToFlat()
    ty = transform.ToFlat()
    tx.fit(x_val)
    ty.fit(y_val)

    # bins and expected events
    n_bins = transform.get_optimal_bin_size(min(len(x), len(y)))
    n_bins = int(math.sqrt(n_bins) * 2)
    nexp = len(x) / n_bins ** 2
    nerr = math.sqrt(nexp)
    a = np.histogram2d(tx.transform(x_val), ty.transform(y_val), bins=(n_bins, n_bins))

    # Transforming the matrix
    a = np.array(a[0])
    a = (a - nexp) / nerr

    # Draw the matrix
    im = ax.imshow(a.T, interpolation='nearest', vmin=-5, vmax=5)
    if not_on_axes:
        print("Printing colorbar")
        plt.colorbar(im, fraction=0.046, pad=0.04)
        set_axis_label_range(ax, x.min(), x.max(), to_flat=tx)
        set_axis_label_range(ax, y.min(), y.max(), axis=0, to_flat=ty)
    else:
        ax.set_xticklabels([])
        ax.set_yticklabels([])

    if draw_label:
        ax.set_xlabel(x.name)
        ax.set_ylabel(y.name)

    # Calculate overall chi2 error for flat distribution
    chi2 = 0
    for i in range(0, n_bins):
        for j in range(0, n_bins):
            # a[i][j] = (a[i][j] - nexp) / nerr
            chi2 += a[i][j] * a[i][j]

    proba = chisqprob(chi2, n_bins * n_bins - ((n_bins - 1) + (n_bins - 1) + 1))
    if not_on_axes:
        ax.set_title("Probability of flat hypothesis %.2f%%" % (proba * 100))
        return im


class ProfilePlot():

    """ Basic Profile plot

    Creates the profile Histogram from x and y distrinbutions
    It plots mean(y) in bins of x

    Attributes:
        x_axis (array)  : Binning in x
        mean (array)    : Mean of y in bin x
        err (array)     : Std of Mean y in bin x
        label (string)  : Matplotlib label for the plot
    """

    def __init__(self, x, y, x_axis=None, n_bins=None, label=None):
        """ init function
        :param x:       Distribution in x
        :param y:       Distribution in y
        :param n_bins:  (optional) n bins in x, is set automatically if not provided
        :param x_axis:  binning for the x-axis
        :param label:   Matplotlib label for the plot
        """
        if x_axis is None:
            x_axis = transform.get_optimal_bin_size(len(x))
        if n_bins is not None:
            x_axis = n_bins

        #: Binning in x
        _, self.x_axis = np.histogram(x, x_axis)

        #: Mean of y in bin x
        self.mean = []

        #: Std of Mean y in bin x
        self.err = []

        #: Matplotlib label for the plot
        self.label = label

        # Calculating the Profile histogram
        for last_x, next_x in zip(self.x_axis[:-1], self.x_axis[1:]):
            bin_range = (x > last_x) & (x < next_x)
            n_y_in_bin = len(y[bin_range])
            if n_y_in_bin == 0:
                self.mean.append(0)
                self.err.append(0)
            else:
                self.mean.append(np.mean(y[bin_range]))
                self.err.append(np.sqrt(np.var(y[bin_range]) / n_y_in_bin))

    def draw(self, color='black'):
        """ Draw function
        :param color: matplotlib color
        """
        bin_centers = (self.x_axis[1:] + self.x_axis[:-1]) / 2.0
        plt.errorbar(bin_centers, self.mean, color=color, yerr=self.err,
                     linewidth=2, ecolor=color, label=self.label, fmt='.')


def draw_flat_corr_matrix(df, pdf=None, tight=False, col_numbers=False, labels=None, fontsize=18, size=12):
    """
    :param df:          DataFrame of the input data
    :param pdf:         optional, file to save
    :param tight:       tight layout, be careful
    :param col_numbers: switch between numbers or names for the columns
    :param labels:      optional, list of latex labels
    :param fontsize:    size of the labels
    """
    assert isinstance(df, pd.DataFrame), 'Argument of wrong type!'

    n_vars = np.shape(df)[1]

    if labels is None:
        labels = df.columns

    fig, axes = plt.subplots(nrows=n_vars, ncols=n_vars, figsize=(size, size))
    for i, row in zip(list(range(n_vars)), axes):
        for j, ax in zip(list(range(n_vars)), row):
            if i is j:
                plt.sca(ax)
                plt.hist(df.ix[:, i].values, transform.get_optimal_bin_size(len(df)), color="gray", histtype='step')
                ax.set_yticklabels([])
                set_axis_label_range(ax, df.ix[:, i].min(), df.ix[:, i].max(), n_labels=3)
            else:
                draw_flat_correlation(df.ix[:, i], df.ix[:, j], ax=ax, draw_label=False)

            if i is n_vars - 1 and j is not n_vars - 1:
                plt.setp(ax.get_xticklabels(), visible=False)

            if i is n_vars - 1:
                ax.xaxis.set_label_coords(0.5, -0.15)

    if tight:
        plt.tight_layout()

    # Common outer label
    for i, row in zip(list(range(n_vars)), axes):
        for j, ax in zip(list(range(n_vars)), row):
            if i == n_vars - 1:
                if col_numbers:
                    ax.set_xlabel("%d" % j)
                else:
                    ax.set_xlabel(labels[j], fontsize=fontsize)
            if j == 0:
                if col_numbers:
                    ax.set_ylabel("%d" % i)
                else:
                    ax.set_ylabel(labels[i], fontsize=fontsize)

    if pdf is None:
        # plt.show()
        pass
    else:
        pdf.savefig()
        plt.close()


def draw_fancy_correlation_matrix(df, pdf=None, tight=False, col_numbers=False, labels=None, fontsize=18, size=12):
    """
    Draws a colored correlation matrix with a profile plot overlay.

    :param df:          DataFrame of the input data
    :param pdf:         optional, file to save
    :param tight:       tight layout, be carefult
    :param col_numbers: swith bwtween numbers or names for the clumns
    :param labels:      optional, list of latex labels
    :param fontsize:    size of the labels
    """

    import matplotlib

    assert isinstance(df, pd.DataFrame), 'Argument of wrong type!'

    n_vars = np.shape(df)[1]

    if labels is None:
        labels = df.columns

    corr = df.corr().values
    norm = matplotlib.colors.Normalize(vmin=-1, vmax=1)
    color = plt.cm.jet
    cma = plt.cm.ScalarMappable(norm=norm, cmap=color)

    fig, axes = plt.subplots(nrows=n_vars, ncols=n_vars, figsize=(size, size))
    for i, row in zip(list(range(n_vars)), axes):
        for j, ax in zip(list(range(n_vars)), row):
            if i is j:
                plt.sca(ax)
                plt.hist(df.ix[:, i].values, transform.get_optimal_bin_size(len(df)), color="gray", histtype='step')
                # plt.xlabel(df.columns[i] if isinstance(df.columns[i], basestring) else "%d" % df.columns[i])
                ax.set_yticklabels([])
                set_axis_label_range(ax, df.ix[:, i].min(), df.ix[:, i].max(), n_labels=3)
            else:
                plt.sca(ax)

                h = ProfilePlot(df.ix[:, i].values, df.ix[:, j].values, label='data', n_bins=10)
                h.draw(color="white")

                x_middle = (plt.xlim()[1] + plt.xlim()[0]) / 2.
                y_middle = (plt.ylim()[1] + plt.ylim()[0]) / 2.

                ax.text(x_middle, y_middle, "$%.3f$" % corr[i][j], fontsize=24, va='center', ha='center')

                ax.patch.set_facecolor(cma.to_rgba(corr[i][j]))

                ax.set_yticklabels([])
                ax.set_xticklabels([])

            if i is n_vars - 1 and j is not n_vars - 1:
                plt.setp(ax.get_xticklabels(), visible=False)

            if i is n_vars - 1:
                ax.xaxis.set_label_coords(0.5, -0.15)

    if tight:
        plt.tight_layout()

    # Common outer label
    for i, row in zip(list(range(n_vars)), axes):
        for j, ax in zip(list(range(n_vars)), row):
            if i == n_vars - 1:
                if col_numbers:
                    ax.set_xlabel("%d" % j)
                else:
                    ax.set_xlabel(labels[j], fontsize=fontsize)
            if j == 0:
                if col_numbers:
                    ax.set_ylabel("%d" % i)
                else:
                    ax.set_ylabel(labels[i], fontsize=fontsize)

    if pdf is None:
        # plt.show()
        pass
    else:
        pdf.savefig()
        plt.close()
