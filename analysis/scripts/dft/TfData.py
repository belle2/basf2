#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import numpy as np


class TfDataBasf2():
    """
    handles data, necessary for the training
    """

    def __init__(self, train_x, train_y, valid_x, valid_y, batch_size, seed=None, epoch_random_shuffle=True):
        """
        declaration of class variables
        """
        #: training features
        self.train_x = train_x
        #: training targets
        self.train_y = train_y
        #: validation features
        self.valid_x = valid_x
        #: validation targets
        self.valid_y = valid_y

        #: batch size
        self.batch_size = batch_size
        #: random generator seed
        self.seed = seed
        #: bool, enables shuffling
        self.epoch_random_shuffle = epoch_random_shuffle

        #: number of training events
        self.train_events = self.train_x.shape[0]
        #: number of validation events
        self.valid_events = self.valid_x.shape[0]

        #: number of features
        self.feature_number = self.train_x.shape[1]

        #: number of batches
        self.batches = self.train_x.shape[0] // self.batch_size

        #: indices required for shuffling
        self.train_idx = np.zeros(self.train_x.shape[0])

        #: np ndarray for training batch features
        self.batch_train_x = np.zeros((self.feature_number, self.batch_size))

        #: np ndarray for training batch of targets
        self.batch_train_y = np.zeros(self.batch_size)

        #: set random generator
        self.random_state = np.random.RandomState(seed)

        #: sanitize labels
        self.sanitize_labels()

    def sanitize_labels(self):
        """
        checks for a binary classification problem
        transforms the two class labels to {0,1}
        """
        # not binary
        assert len(np.unique(self.train_y)) == 2
        # different classes
        assert np.array_equal(np.unique(self.train_y), np.unique(self.valid_y))

        # reject corner cases when classes would have special values
        if self.train_y.min() > 0:
            self.train_y[self.train_y == self.train_y.min()] = 0
            self.valid_y[self.valid_y == self.valid_y.min()] = 0

        if self.train_y.max() != 1:
            self.train_y[self.train_y == self.train_y.max()] = 1
            self.valid_y[self.valid_y == self.valid_y.max()] = 1

        # transform labels
        if self.train_y.min() != 0:
            self.train_y[self.train_y == self.train_y.min()] = 0
            self.valid_y[self.valid_y == self.valid_y.min()] = 0

    def batch_iterator(self):
        """
        iterator to provide training batches
        """
        self.train_idx = np.arange(len(self.train_idx))

        if self.epoch_random_shuffle:
            self.random_state.shuffle(self.train_idx)

        for i in range(self.batches):
            self.batch_train_x = self.train_x[self.train_idx[i * self.batch_size: (i + 1) * self.batch_size]]
            self.batch_train_y = self.train_y[self.train_idx[i * self.batch_size: (i + 1) * self.batch_size]]

            yield self.batch_train_x, self.batch_train_y


class TfDataBasf2Stub():
    """
    stub class just for initializing in basf2 begin_run
    """

    def __init__(self, batch_size, feature_number, event_number, train_fraction):
        """
        declare for initialization required batch parameters
        """
        #: batch size
        self.batch_size = batch_size

        #: feature number
        self.feature_number = feature_number

        #: number of batches
        self.batches = (event_number * train_fraction) // self.batch_size

        #: number of training training events
        self.train_events = int(train_fraction * event_number)

        #: number of validation events
        self.valid_events = int((1 - train_fraction) * event_number)
