##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import numpy as np
from typing import Union


def _power(array: np.ndarray, power: Union[int, float]):
    """Preprocessing function to take power of given feature."""
    return np.sign(array) * np.power(np.abs(array), power)


def _linear(array: np.ndarray, mu=0.0, sigma=1.0):
    """Preprocessing function to linear scale given feature."""
    return (array - mu) / sigma


methods = {"power": _power, "linear": _linear}


def normalize_features(
    normalize={},
    features=[],
    x=[],
    edge_features=[],
    x_edges=[],
    global_features=[],
    x_global=[],
):
    """
    Function to normalize input features.

    ``normalize`` should be a dictionary of the form ``{'power', [0.5], 'linear', [-0.5, 4.1]}``.
    ``power`` and ``linear`` are the only processes supported.

    Args:
        normalize (dict): Normalization processes and parameters.
        features (list): List of node feature names.
        x (numpy.ndarray): Array of node features.
        edge_features (list): List of edge feature names.
        x_edges (numpy.ndarray): Array of edge features.
        global_features (list): List of global feature names.
        x_global (numpy.ndarray): Array of global features.
    """
    for feat, processes in normalize.items():
        # Start with node features
        feat_name = f"feat_{feat}"
        if feat_name in features:
            feat_idx = features.index(feat_name)

            # Apply normalizations in order
            for proc in processes:
                args = proc[1:]
                x[:, feat_idx] = methods[proc[0]](x[:, feat_idx], *args)
            continue  # assume no features of different type with same name

        # Continue with edge features
        feat_name = f"edge_{feat}"
        if feat_name in edge_features:
            feat_idx = edge_features.index(feat_name)

            # Apply normalizations in order
            for proc in processes:
                args = proc[1:]
                x_edges[:, feat_idx] = methods[proc[0]](x_edges[:, feat_idx], *args)
            continue  # assume no features of different type with same name

        # Continue with global features
        feat_name = f"glob_{feat}"
        if feat_name in global_features:
            feat_idx = global_features.index(feat_name)

            # Apply normalizations in order
            for proc in processes:
                args = proc[1:]
                x_global[:, feat_idx] = methods[proc[0]](x_global[:, feat_idx], *args)
