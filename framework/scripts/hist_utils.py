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
