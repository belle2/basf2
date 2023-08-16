import torch
from .tree_utils import is_valid_tree


class InvalidAdjacencyMatrix(Exception):
    """
    Specialized Exception sub-class raised for malformed adjacency matrices or adjacency matrices that do not encode trees.
    """

    pass


def _find_path(column, adjacency_matrix, levels):
    """
    Determines all node indices in the trail from the passed starting node (column) to the root of the tree encoded by
    the passed adjacency matrix.

    Args:
        column (int): Column index into the adjacency matrix encoding the node for which to find the path to the root.
        adjacency_matrix (iterable): 2-dimensional matrix (N, N) encoding the graph's node adjacencies. Linked nodes
            should have value unequal to zero.
        levels (iterable): 1-dimensional vector (N) encoding at which level each node resides in the tree. Leaves are
            assigned to level 1, parents have higher indices.

    Returns:
        set: The set of node indices along the trail from the starting node (column) to the root.
    """
    n = adjacency_matrix.shape[0]
    path = {column}
    level = 0

    while True:
        for row in range(n - 1, -1, -1):
            # skip own node
            if row == column:
                continue
            if adjacency_matrix[row, column] and levels[row] > level:
                column = row
                path.add(row)
                # level += 1
                level = levels[row]
                break
        else:
            return path


def _find_level(column, adjacency_matrix):
    """
    Determines the tree level of a singular node given an adjacency matrix.

    Args:
        column (int): the column/row of the node for which to determine the tree level
        adjacency_matrix (iterable): 2-dimensional matrix (N, N) encoding the graph's node adjacencies. Linked nodes
            should have value unequal to zero.

    Returns:
        int: the node's tree level.
    """
    n = adjacency_matrix.shape[0]
    level = -1

    for i in range(column + 1, n):
        if adjacency_matrix[column, i]:
            level = max(level, _find_level(i, adjacency_matrix))

    return level + 1


def _get_levels(adjacency_matrix):
    """
    Returns a vector that contains the tree level for each node, i.e. column/row, in the adjacency matrix

    Args:
        adjacency_matrix (iterable): 2-dimensional matrix (N, N) encoding the graph's node adjacencies. Linked nodes
            should have value unequal to zero.

    Returns:
        iterable: 1-dimensional (N) the tree levels of the nodes.
    """
    n = adjacency_matrix.shape[0]
    levels = torch.zeros((n,), dtype=torch.int64)

    for i in range(n):
        levels[i] = _find_level(i, adjacency_matrix)

    return levels


def _adjacency2lca(adjacency_matrix):
    """
    Converts a tree's adjacency matrix (N,N) into its corresponding LCA-gram matrix representation, i.e. a square matrix
    (M, M), with M < N, where each row/column corresponds to a leaf of the tree and each matrix entry is the level of
    the lowest-common-ancestor (LCA) of the two leaves.

    Args:
        adjacency_matrix (iterable): 2-dimensional matrix (N, N) encoding the graph's node adjacencies. Linked nodes
            should have value unequal to zero.

    Returns:
        iterable: 2-dimensional (M, M) LCA-gram-matrix corresponding to the passed adjacency matrix.
    """
    n, m = adjacency_matrix.shape

    # determine the leaf nodes
    is_leaf = (adjacency_matrix.sum(axis=0) == 1).type(torch.int64)

    # create the lca matrix
    leaf_count = is_leaf.sum()
    lca_matrix = torch.zeros(
        (
            leaf_count,
            leaf_count,
        )
    )

    # determine the levels of each node
    levels = _get_levels(adjacency_matrix)

    # for every leaf get the paths to the root
    paths = [
        _find_path(column, adjacency_matrix, levels)
        for column in range(n)
        if levels[column] == 0
    ]

    # compare all path with one another
    for column, path_a in enumerate(paths):
        for row in range(column + 1, leaf_count):
            path_b = paths[row]

            common = path_a & path_b
            join_level = min(levels[node] for node in common)

            lca_matrix[row, column] = join_level
            lca_matrix[column, row] = join_level

    return lca_matrix


def adjacency2lca(adjacency_matrix):
    """
    Converts a tree's adjacency matrix (N,N) into its corresponding LCA-gram matrix representation, i.e. a square matrix
    (M, M), with M < N, where each row/column corresponds to a leaf of the tree and each matrix entry is the level of
    the lowest-common-ancestor (LCA) of the two leaves. The levels are enumerated top-down from the root for breadth-first-search,
    or traversed directly down to every leaf for depth-first-search.

    Args:
        adjacency_matrix (iterable): 2-dimensional matrix (N, N) encoding the graph's node adjacencies. Linked nodes
            should have value unequal to zero.

    Returns:
        iterable: 2-dimensional (M, M) LCA-gram-matrix corresponding to the passed adjacency matrix.

    Raises:
        InvalidAdjacencyMatrix: If passed adjacency matrix is malformed (e.g. not 2d or not square) or does not encode
            a tree.
    """
    # Ensure input is torch tensor or can be converted to it
    if not isinstance(adjacency_matrix, torch.Tensor):
        try:
            adjacency_matrix = torch.Tensor(adjacency_matrix)
        except TypeError as err:
            print(f"Input type must be compatible with torch Tensor: {err}")
            raise

    if len(adjacency_matrix.shape) != 2:
        raise InvalidAdjacencyMatrix

    n, m = adjacency_matrix.shape
    if n != m:
        raise InvalidAdjacencyMatrix

    if not (adjacency_matrix == adjacency_matrix.T).all():
        raise InvalidAdjacencyMatrix

    if not is_valid_tree(adjacency_matrix):
        raise InvalidAdjacencyMatrix

    return _adjacency2lca(adjacency_matrix)
