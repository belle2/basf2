#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import numpy as np


def hist2array(hist, return_edges=False):
    """
    Function to convert a histogram into a numpy array and optionally also
    return a list of the bin edges
    Parameters:
        hist: one-dimensional histogram
        return_edges (bool): flag whether bin edges should be calculated and returned
    Return:
        numpy array and optionally a list of the bin edges
    """
    nbins = hist.GetNbinsX()
    arr = np.zeros(nbins)
    for i in range(nbins):
        arr[i] = hist.GetBinContent(i + 1)
    if return_edges:
        edges = []
        edges.append(np.empty(nbins + 1))
        hist.GetLowEdge(edges[-1])
        edges[-1][nbins] = hist.GetBinLowEdge(nbins) + hist.GetBinWidth(nbins)
        return arr, edges
    return arr


def array2hist(array, hist):
    """
    Function to fill a histogram from an array
    Parameters:
        array: array
        hist: histogram
    """
    ndim = array.ndim
    if ndim == 1:
        nbins = len(array)
        for i in range(nbins):
            hist.SetBinContent(i + 1, array[i])
    elif ndim == 2:
        for i in range(hist.GetNbinsX()):
            for j in range(hist.GetNbinsY()):
                hist.SetBinContent(i + 1, j + 1, array[i, j])
