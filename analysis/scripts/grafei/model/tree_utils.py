import torch as t
from variables import variables as vm
import numpy as np


def node_masses(particles, nodes):
    """
    Computes the invariant mass of particles who have the same ancestor, one for each element in nodes.

    Args:
        particles (list): list of predicted matched particles.
        nodes (list): list of lists of indices, one for each ancestor node that you want to consider. Each list contains
                        the position of the particles in the LCA matrix.

    Returns:
        masses (list): list of invariant masses, one for each element in indices.
    """
    masses = []

    for indices in nodes:
        fsps = [j for i, j in enumerate(particles) if i in indices]

        E = 0.0
        px = 0.0
        py = 0.0
        pz = 0.0

        for particle in fsps:
            E += vm.evaluate("E", particle)
            px += vm.evaluate("px", particle)
            py += vm.evaluate("py", particle)
            pz += vm.evaluate("pz", particle)

        m2 = E**2 - px**2 - py**2 - pz**2
        m = np.sqrt(m2) if m2 >= 0 else np.nan

        masses.append(m)

    return masses


def masses_to_classes(array):
    """
    Convert the mass hypotheses to classes.
    """
    array[array > 0] *= -1  # All elements become negative
    array[array == -11] = 1  # Electrons are class 1
    array[array == -13] = 2  # Muons are class 2
    array[array == -211] = 3  # Pions are class 3
    array[array == -321] = 4  # Kaons are class 4
    array[array == -2212] = 5  # Protons are class 5
    array[array == -22] = 6  # Photons are class 6
    array[array <= 0] = 0  # Others are 0

    return array


def check_undirected(adjacency_matrix):
    """
    Checks whether an adjacency matrix-encoded graph is undirected, i.e. symmetric.

    Args:
        adjacency_matrix (iterable): 2-dimensional matrix (N, N) encoding the graph's node adjacencies. Linked nodes
            should have value unequal to zero.

    Returns:
        bool: True if the graph encoded by adjacency matrix is undirected, False otherwise.
    """
    n, m = adjacency_matrix.shape
    if n != m:
        return False

    return (adjacency_matrix == adjacency_matrix.T).all()


def _connectedness_dfs(adjacency_matrix, index, reached):
    """
    Actual depth-first search of graph connectedness. Starting from the node marked by index a recursive search is
    performed. Visited nodes are marked as reachable during recursion. If the graph is not connected, the reachability
    'reached' mask will contain zero elements.

    Args:
        adjacency_matrix (iterable): 2-dimensional matrix (N, N) encoding the graph's node adjacencies. Linked nodes
            should have value unequal to zero.
        index (int): Row index into adjacency matrix for which to perform the DFS search.
        reached (iterable): 1-dimensional bit-array (N) encoding the already visited and hence connected elements. This
            variable is modified in-place.
    """
    n = adjacency_matrix.shape[0]
    reached[index] = 1

    # traverse through all
    for column in range(n):
        # recursively search for connectedness nodes that are adjacent and avoid nodes already marked as reachable
        if adjacency_matrix[index, column] != 0 and not reached[column]:
            _connectedness_dfs(adjacency_matrix, column, reached)


def check_connectedness(adjacency_matrix, allow_disconnected_leaves=False):
    """
    Checks whether all sub-graphs of an adjacency matrix-encoded graph are connected, i.e. have at least one edge
    linking them.

    Args:
        adjacency_matrix (numpy.ndarray): 2-dimensional matrix (N, N) encoding the graph's node adjacencies. Linked
            nodes should have value unequal to zero.
        allow_disconnected_leaves (bool): Allows singular nodes to be disconnected from the entire graph.

    Returns:
        bool: True if all sub-graphs encoded by the adjacency matrix are connected, False otherwise.
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

    Args:
        adjacency_matrix (iterable): 2-dimensional matrix (N, N) encoding the graph's node adjacencies. Linked nodes
            should have value unequal to zero.
        index (int): Row index into adjacency matrix for which to perform the DFS search.
        parent (int): Row index into adjacency matrix for the immediate parent.
        reached (iterable): 1-dimensional bit-array (N) encoding the already visited and hence connected elements. This
            variable is modified in-place.
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


def check_acyclic(adjacency_matrix):
    """
    Checks whether the graph encoded by the passed adjacency matrix is acyclic, i.e. all non-empty trails in the graph
    do not contain repetitions. Node self-references are legal and simply ignored.

    Args:
        adjacency_matrix (numpy.ndarray): 2-dimensional matrix (N, N) encoding the graph's node adjacencies. Linked
            nodes should have value unequal to zero.

    Returns:
        bool: True if the graph is acyclic, False otherwise.
    """
    n, m = adjacency_matrix.shape
    if n != m:
        return False

    reached = t.zeros(n, dtype=t.uint8)

    return _acyclic_dfs(adjacency_matrix, 0, -1, reached)


def is_valid_tree(adjacency_matrix):
    """
    Checks whether the graph encoded by the passed adjacency matrix encodes a valid tree, i.e. an undirected, acyclic
    and connected graph.

    Args:
        adjacency_matrix (numpy.ndarray): 2-dimensional matrix (N, N) encoding the graph's node adjacencies. Linked
            nodes should have value unequal to zero.

    Returns:
        bool: True if the encoded graph is a tree, False otherwise.
    """
    undirected = check_undirected(adjacency_matrix)
    connected = check_connectedness(adjacency_matrix)
    acyclic = check_acyclic(adjacency_matrix)

    return undirected and connected and acyclic
