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

from sklearn.model_selection import train_test_split


def fit(model, filename, treename, variables, target_variable, config):

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

    # do train test split with shuffling
    train_size = config['train_valid_fraction']
    X_train, X_test, y_train, y_test = train_test_split(X, y, train_size=train_size, random_state=42)

    # configure the optimizer
    cosine_decay_scheduler = keras.optimizers.schedules.CosineDecay(
        initial_learning_rate=config['initial_learning_rate'],
        decay_steps=config['decay_steps'],
        alpha=config['alpha']
    )

    optimizer = keras.optimizers.AdamW(
        learning_rate=cosine_decay_scheduler, weight_decay=config['weight_decay']
    )

    # compile the model
    model.compile(
        optimizer=optimizer,
        loss=keras.losses.binary_crossentropy,
        metrics=[
            'accuracy',
            keras.metrics.AUC()])

    model.summary()

    # perform fit() with early stopping callback
    callbacks = [keras.callbacks.EarlyStopping(
        monitor='val_loss',
        min_delta=0,
        patience=config['patience'],
        verbose=1,
        mode='auto',
        baseline=None,
        restore_best_weights=True)]

    model.fit(
        X_train,
        y_train,
        validation_data=(
            X_test,
            y_test),
        batch_size=config['batch_size'],
        epochs=config['epochs'],
        callbacks=callbacks,
        verbose=1)

    print()
