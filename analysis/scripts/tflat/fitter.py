#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import keras
import uproot
import numpy as np
import ROOT


def fit(model, filename, treename, variables, target_variable, config, checkpoint_filepath):

    # read training data from file
    with uproot.open({filename: treename}) as tree:
        X = tree.arrays(
            map(ROOT.Belle2.MakeROOTCompatible.makeROOTCompatible, variables),
            library="pd",
        ).to_numpy()
        y = tree[target_variable].array(library="np")

    # transform labels to 0, 1
    if y.min() != 0:
        y[y == y.min()] = 0

    # assert binary labels
    assert len(np.unique(y)) == 2

    # in-place shuffling of training data
    # note: must be done with same random seed
    rng = np.random.default_rng(42)
    rng.shuffle(X)
    rng = np.random.default_rng(42)
    rng.shuffle(y)

    # configure early stopping callback
    callbacks = [keras.callbacks.EarlyStopping(
        monitor='val_loss',
        min_delta=0,
        patience=config['patience'],
        verbose=1,
        mode='auto',
        baseline=None,
        restore_best_weights=True)]

    # configure checkpointing callback
    model_checkpoint_callback = keras.callbacks.ModelCheckpoint(
        filepath=checkpoint_filepath,
        monitor='val_loss',
        mode='min',
        save_best_only=True)
    callbacks.append(model_checkpoint_callback)

    # perform fit() with callbacks
    model.fit(
        X,
        y,
        validation_split=1-config['train_valid_fraction'],
        batch_size=config['batch_size'],
        epochs=config['epochs'],
        callbacks=callbacks,
        verbose=2)

    print()
