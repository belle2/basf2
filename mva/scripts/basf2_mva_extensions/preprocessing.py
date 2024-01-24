#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import numpy as np


class fast_equal_frequency_binning():
    """
    This class provides a fast implementation of equal frequency binning.
    In Equal frequency binning the binning is chosen in a way that every bin has the same number of entries.
    An example with a Neural Network can be found in: mva/examples/keras/preprocessing.py
    """

    def __init__(self, state=None):
        """
        Init the class.
        If you saved a state before and wants to rebuild the class use the state parameter.
        """
        if state is None:
            #: State of the class. This will be saved
            self.state = {'binning_array': [], 'number_of_bins': 0}
        else:
            self.state = state

    def fit(self, x, number_of_bins=100):
        """
        Do the fitting -> calculate binning boundaries
        """
        for variable in range(len(x[0, :])):
            self.state['binning_array'].append(np.percentile(np.nan_to_num(x[:, variable]),
                                                             np.linspace(0, 100, number_of_bins + 1)))
        self.state['number_of_bins'] = number_of_bins

    def apply(self, x):
        """
        Bin a dataset
        """
        for variable in range(len(x[0, :])):
            x[:, variable] = np.digitize(np.nan_to_num(x[:, variable]),
                                         self.state['binning_array'][variable][1:-1]) / self.state['number_of_bins']
        return x

    def export_state(self):
        """
        Returns a pickable dictionary to save the state of the class in a mva weightfile
        """
        return self.state
