import sys
import numpy as np


def getDoca(name_values):
    """
    Computes DOCA between two tracks.

    Args:
        name_values (dict): dict of numpy arrays containing px, py, pz, x, y, z

    Returns:
        doca (numpy array): array containing doca values
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

    v = np.cross(p, p[:, None]).reshape(-1, 3)  # Momenta cross-product
    v_norm = np.linalg.norm(v, axis=1).reshape((-1, 1))  # Norm of each cross-product
    v_norm[
        v_norm == 0
    ] = 1  # Suppress divide by 0 warnings in the diagonal (anyway it will be ignored)
    v_u = v / v_norm

    dr = np.subtract(r, r[:, None]).reshape(-1, 3)  # Difference in 3-positions
    dr_x_vu = (
        np.dot(dr, v_u.T).diagonal().reshape((1, -1))
    )  # Dot products between r and v_u

    # Doca computed here
    doca = np.linalg.norm(v_u * dr_x_vu.T - dr * (v_norm < eps), axis=1).reshape(
        n_parts, n_parts
    )

    return doca[
        ~np.eye(doca.shape[0], dtype=bool)
    ]  # Remove diagonal elements and flatten


def getCostheta(name_values):
    """
    Computes cos of angle between two tracks.

    Args:
        name_values (dict): dict of numpy arrays containing p, px, py, pz

    Returns:
        costheta (numpy array): array containing cos theta values
    """
    ux = name_values["px"] / name_values["p"]
    uy = name_values["py"] / name_values["p"]
    uz = name_values["pz"] / name_values["p"]

    u = np.array([ux, uy, uz]).T

    costheta = np.inner(u, u)

    return costheta[
        ~np.eye(costheta.shape[0], dtype=bool)
    ]  # remove diagonal elements and flatten


available_features = {  # put here available features with respective functions (prepend edge_ to the name)
    "edge_costheta": getCostheta,
    "edge_doca": getDoca,
}


def available_edge_features(feat, name_values):
    """
    Returns value of edge feature if contained in dictionary defined below

    Args:
        feat (string): edge feature name
        name_values (dict): dict of numpy array containing node features
    Returns:
        (Numpy array): edge feature `feat` computed given certain node features `name_values`
    """
    if feat not in available_features:
        sys.exit(
            "Requested edge feature not available, but you could add it in grafei/data/edge_features.py!"
        )

    return available_features[feat](name_values)


def compute_edge_features(
    edge_feature_names: list, features: list, x: np.ndarray
) -> np.ndarray:
    """
    Computes a series of edge features strarting from node features

    Args:
        edge_feature_names (list): list of edge features names
        features (list): list of node feature names
        x (numpy array): array of node features
    Returns:
        edge_features (numpy array): array of edge features
    """

    # Will be filled and converted to np.ndarray of shape [E, F_e] with
    # E=nodes*(nodes-1) (assume no self-interactions) and F_e number of edge
    # features
    edge_features = []
    # Remove `feat_` from feature names
    features = [f.replace("feat_", "") for f in features]

    # Associate node feature names with values
    name_values = {}
    for name, values in zip(
        features,
        x.T[:, ],
    ):
        name_values.update({name: values})

    # Compute edge features
    for feat in edge_feature_names:
        feature_values = available_edge_features(feat, name_values)
        edge_features.append(feature_values)

    return np.array(edge_features).T
