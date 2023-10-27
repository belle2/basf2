import numpy as np
from typing import Union


def power(array: np.ndarray, power: Union[int, float]) -> np.ndarray:
    """Preprocessing function to take power of given feature"""
    return np.sign(array) * np.power(np.abs(array), power)


def linear(array: np.ndarray, mu=0.0, sigma=1.0) -> np.ndarray:
    """Preprocessing function to linear scale given feature"""
    return (array - mu) / sigma


methods = {"power": power, "linear": linear}


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
    Function to normalize input features

    Args:
        normalize (dict): what kind of normalization and parameters to apply
        features (list): list of node feature names
        x (array): array of node features
        edge_features (list): list of edge feature names
        x_edges (array): array of edge features
        global_features (list): list of global feature names
        x_global (array): array of global features
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
