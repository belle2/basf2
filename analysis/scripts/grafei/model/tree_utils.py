##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import torch as t
import numpy as np


def masses_to_classes(array):
    """
    Converts mass hypotheses to classes used in cross-entropy computation.

    Classes are:

    .. math::
        e \\to 1\\\\
        \\mu \\to 2\\\\
        \\pi \\to 3\\\\
        K \\to 4\\\\
        p \\to 5\\\\
        \\gamma \\to 6\\\\
        \\text{others} \\to 0

    Args:
        array (numpy.ndarray): Array containing PDG mass codes.

    Returns:
        numpy.ndarray: Array containing mass hypothese converted to classes.
    """
    array = -1 * np.abs(array)  # All elements become negative
    array[array == -11] = 1  # Electrons are class 1
    array[array == -13] = 2  # Muons are class 2
    array[array == -211] = 3  # Pions are class 3
    array[array == -321] = 4  # Kaons are class 4
    array[array == -2212] = 5  # Protons are class 5
    array[array == -22] = 6  # Photons are class 6
    array[array <= 0] = 0  # Others are 0

    return array


def _check_undirected(adjacency_matrix):
    """
    Checks whether an adjacency matrix-encoded graph is undirected, i.e. symmetric.
    """
    n, m = adjacency_matrix.shape
    if n != m:
        return False

    return (adjacency_matrix == adjacency_matrix.T).all()


def _connectedness_dfs(adjacency_matrix, index, reached):
    """
    Actual depth-first search of graph connectedness. Starting from the node marked by index a recursive search is
    performed. Visited nodes are marked as reachable during recursion. If the graph is not connected, the reachability
    `reached` mask will contain zero elements.
    """
    n = adjacency_matrix.shape[0]
    reached[index] = 1

    # Traverse through all
    for column in range(n):
        # Recursively search for connectedness nodes that are adjacent and avoid nodes already marked as reachable
        if adjacency_matrix[index, column] != 0 and not reached[column]:
            _connectedness_dfs(adjacency_matrix, column, reached)


def _check_connectedness(adjacency_matrix, allow_disconnected_leaves=False):
    """
    Checks whether all sub-graphs of an adjacency matrix-encoded graph are connected,
    i.e. have at least one edge linking them.
    """
    n, m = adjacency_matrix.shape
    if n != m:
        return False

    reached = t.zeros(n, dtype=t.uint8)
    _connectedness_dfs(adjacency_matrix, 0, reached)

    if allow_disconnected_leaves:
        reached = t.logical_or(reached, adjacency_matrix.sum(axis=1) == 0)

    return reached.all()


def _acyclic_dfs(adjacency_matrix, index, parent, reached):
    """
    Actual depth-first search of graph cycles. Starting from the node marked by index a recursive search is performed.
    Visited nodes are marked as reachable during recursion. If a node is found in a trail that has been previously
    marked as already reached this indicates a cycle.
    """
    n = adjacency_matrix.shape[0]
    reached[index] = 1

    for row in range(n):
        # the passed adjacency matrix may contain self-references
        # while technically not acyclic, these are allowed,
        if row == index:
            continue

        if adjacency_matrix[index, row] != 0:
            if not reached[row]:
                # cycle
                if not _acyclic_dfs(adjacency_matrix, row, index, reached):
                    return False
            elif row != parent:
                # cycle
                return False
    return True


def _check_acyclic(adjacency_matrix):
    """
    Checks whether the graph encoded by the passed adjacency matrix is acyclic, i.e. all non-empty trails in the graph
    do not contain repetitions. Node self-references are legal and simply ignored.
    """
    n, m = adjacency_matrix.shape
    if n != m:
        return False

    reached = t.zeros(n, dtype=t.uint8)

    return _acyclic_dfs(adjacency_matrix, 0, -1, reached)


def is_valid_tree(adjacency_matrix):
    """
    Checks whether the graph encoded by the passed adjacency matrix encodes a valid tree,
    i.e. an undirected, acyclic and connected graph.

    Args:
        adjacency_matrix (numpy.ndarray): 2-dimensional matrix (N, N) encoding the graph's node adjacencies.
            Linked nodes should have value unequal to zero.

    Returns:
        bool: True if the encoded graph is a tree, False otherwise.
    """
    undirected = _check_undirected(adjacency_matrix)
    connected = _check_connectedness(adjacency_matrix)
    acyclic = _check_acyclic(adjacency_matrix)

    return undirected and connected and acyclic
