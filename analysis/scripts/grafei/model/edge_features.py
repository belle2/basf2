##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import sys
import numpy as np


def compute_doca(name_values):
    """
    Computes DOCA between two tracks.

    Args:
        name_values (dict): Dictionary of numpy arrays containing px, py, pz, x, y, z.

    Returns:
        numpy.ndarray: Array containing doca values.
    """
    eps = 1e-7

    px = name_values["px"]
    py = name_values["py"]
    pz = name_values["pz"]
    x = name_values["x"]
    y = name_values["y"]
    z = name_values["z"]

    p = np.array([px, py, pz]).T
    r = np.array([x, y, z]).T

    n_parts = len(px)

    # Momenta cross-product
    v = np.cross(p, p[:, None]).reshape(-1, 3)
    # Norm of each cross-product
    v_norm = np.linalg.norm(v, axis=1).reshape((-1, 1))
    # Suppress divide by 0 warnings in the diagonal (anyway it will be ignored)
    v_norm[v_norm == 0] = 1
    v_u = v / v_norm

    # Difference in 3-positions
    dr = np.subtract(r, r[:, None]).reshape(-1, 3)
    # Dot products between r and v_u
    dr_x_vu = (
        np.dot(dr, v_u.T).diagonal().reshape((1, -1))
    )

    # Doca computed here
    doca = np.linalg.norm(v_u * dr_x_vu.T - dr * (v_norm < eps), axis=1).reshape(
        n_parts, n_parts
    )

    # Remove diagonal elements and flatten
    return doca[~np.eye(doca.shape[0], dtype=bool)]


def compute_cosTheta(name_values):
    """
    Computes cosinus of angle between two tracks.

    Args:
        name_values (dict): Dictionary of numpy arrays containing p, px, py, pz.

    Returns:
        numpy.ndarray: Array containing cosinus of theta values.
    """
    ux = name_values["px"] / name_values["p"]
    uy = name_values["py"] / name_values["p"]
    uz = name_values["pz"] / name_values["p"]

    u = np.array([ux, uy, uz]).T

    costheta = np.inner(u, u)

    # Remove diagonal elements and flatten
    return costheta[~np.eye(costheta.shape[0], dtype=bool)]


# Put here available features with respective functions (prepend edge_ to the name)
available_features = {
    "edge_costheta": compute_cosTheta,
    "edge_doca": compute_doca,
}


def _available_edge_features(feat, name_values):
    """
    Returns value of edge feature if contained in dictionary available_features.
    """
    if feat not in available_features:
        sys.exit(
            "Requested edge feature not available, but you could add it in grafei/data/edge_features.py!"
        )

    return available_features[feat](name_values)


def compute_edge_features(edge_feature_names, features, x):
    """
    Computes a series of edge features starting from node features.

    Args:
        edge_feature_names (list): List of edge features names.
        features (list): List of node feature names.
        x (numpy.ndarray): Array of node features.

    Returns:
        numpy.ndarray: Array of edge features.
    """

    # Will be filled and converted to np.ndarray of shape [E, F_e] with
    # E=nodes*(nodes-1) (assume no self-interactions) and F_e number of edge features
    edge_features = []
    # Remove `feat_` from feature names
    features = [f.replace("feat_", "") for f in features]

    # Associate node feature names with values
    name_values = dict(zip(features, x.T))

    # Compute edge features
    for feat in edge_feature_names:
        feature_values = _available_edge_features(feat, name_values)
        edge_features.append(feature_values)

    return np.array(edge_features).T
