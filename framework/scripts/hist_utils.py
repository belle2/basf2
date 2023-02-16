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
        return arr, edges
    return arr
