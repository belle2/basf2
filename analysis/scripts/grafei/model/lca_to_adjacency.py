##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import torch as t
import numpy as np
from collections import Counter
from itertools import permutations
from .tree_utils import is_valid_tree


class InvalidLCAMatrix(Exception):
    """
    Specialized Exception sub-class raised for malformed LCA matrices or LCA matrices not encoding trees.
    """
    pass


class Node:
    """
    Class to hold levels of nodes in the tree.

    Args:
        level (int): Level in the tree.
        children (list[Node]): Children of the nodes.
        lca_index (int): Index in the LCAS matrix.
        lcas_level (int): Level in the LCAS matrix.
    """

    def __init__(self, level, children, lca_index=None, lcas_level=0):
        """
        Initialization
        """
        #: LCA level
        self.level = level
        #: Node children
        self.children = children
        #: LCA index
        self.lca_index = lca_index
        #: LCAS level
        self.lcas_level = lcas_level

        #: Parent nodes
        self.parent = None
        #: BFS index
        self.bfs_index = -1


def _get_ancestor(node):
    """
    Trail search for the highest ancestor of a node.
    """
    ancestor = node

    while ancestor.parent is not None:
        ancestor = ancestor.parent

    return ancestor


def _nodes_in_ancestors_children(parent, node1, node2):
    """
    Checks if any node in parent's line of descent is also an ancestor of both node1 and node2.
    """
    for child in parent.children:
        if (node1 in child.children) and (node2 in child.children):
            return True
        else:
            _nodes_in_ancestors_children(child, node1, node2)

    return False


def _pull_down(node):
    """
    Works up the node's history, pulling down a level any nodes
    whose children are all more than one level below.

    Performs the operation in place.
    """
    # First check the children
    if len(node.children) > 0:
        highest_child = max([c.level for c in node.children])
        node.level = highest_child + 1

    # Then move on to the parent
    if node.parent is not None:
        _pull_down(node.parent)

    return


def _breadth_first_enumeration(root, queue, adjacency_matrix):
    """
    Enumerates the tree breadth-first into a queue.
    """
    # Insert current root node into the queue
    level = root.level
    queue.setdefault(level, []).append(root)

    # Enumerate the children
    for child in root.children:
        _breadth_first_enumeration(child, queue, adjacency_matrix)

    return queue


def _breadth_first_adjacency(root, adjacency_matrix):
    """
    Enumerates the tree breadth-first into a queue.
    """
    queue = _breadth_first_enumeration(root, {}, adjacency_matrix)

    # On recursion end in the root node, traverse the tree once to assign bfs ids to each node
    index = 0
    for i in range(root.level, 0, -1):
        for node in queue[i]:
            node.bfs_index = index
            index += 1

    # Then traverse the tree again to fill in the adjacencies
    for i in range(root.level, 0, -1):
        for node in queue[i]:
            for child in node.children:
                adjacency_matrix[node.bfs_index, child.bfs_index] = 1
                adjacency_matrix[child.bfs_index, node.bfs_index] = 1


def _reconstruct(lca_matrix):
    """
    Does the actual heavy lifting of the adjacency matrix reconstruction. Traverses the LCA matrix level-by-level,
    starting at one. For each level new nodes have to be inserted into the adjacency matrix, if a LCA matrix with this
    level number exists. The newly created node(s) will then be connected to the lower leaves, respectively,
    sub-graphs. This function may produce reconstructions that are valid graphs, but not trees.
    """
    n = lca_matrix.shape[0]
    total_nodes = n
    # depths = int(lca_matrix.max())
    levels = sorted(lca_matrix.unique().tolist())
    # Want to skip over leaves
    levels.remove(0)

    # Create nodes for all leaves
    leaves = [Node(1, [], lca_index=i) for i in range(n)]

    # Iterate level-by-level through the matrix, starting from immediate connections
    # we can correct missing intermediate levels here too
    # Just use current_level to check the actual LCA entry, once we know which level it is
    # (ignoring missed levels) then use the index (corrected level)
    # for current_level in range(1, depths + 1):
    for idx, current_level in enumerate(levels, 1):
        # Iterate through each leaf in the LCA matrix
        for column in range(n):
            # Iterate through all corresponding nodes
            # The LCA matrix is symmetric, hence, check only the from the diagonal down
            for row in range(column + 1, n):
                # Skip over entries not in current level
                if lca_matrix[row, column] <= 0:
                    raise InvalidLCAMatrix
                elif lca_matrix[row, column] != current_level:
                    continue

                # Get the nodes
                a_node = leaves[column]
                another_node = leaves[row]

                # Determine the ancestors of both nodes
                an_ancestor = _get_ancestor(a_node)
                a_level = an_ancestor.level

                another_ancestor = _get_ancestor(another_node)
                another_level = another_ancestor.level

                # The nodes both already have an ancestor at that level, confirm it's the same one
                # and check that the common ancestor doesn't have a child which is in turn an ancestor of both left and right nodes
                if a_level == another_level == (idx + 1):
                    if (
                        an_ancestor is not another_ancestor
                        or _nodes_in_ancestors_children(
                            an_ancestor, a_node, another_node
                        )
                    ):
                        raise InvalidLCAMatrix
                # Should also check neither have an ancestor above the current level
                # If so then something went really wrong
                elif a_level > idx + 1 or another_level > idx + 1:
                    raise InvalidLCAMatrix

                # The nodes don't have an ancestor at the level we're inspecting.
                # We need to make one and connect them to it
                elif a_level < idx + 1 and another_level < idx + 1:
                    parent = Node(idx + 1, [an_ancestor, another_ancestor], lcas_level=current_level)
                    an_ancestor.parent = parent
                    another_ancestor.parent = parent
                    total_nodes += 1

                # the left node already has a higher order parent, lets attach to it
                # I think should confirm that a_level == idx + 1 too
                elif another_level < idx + 1 and a_level == idx + 1:
                    # This should be the another_ancestor.parent getting assigned
                    # another_node.parent = an_ancestor
                    # an_ancestor.children.append(another_node)
                    another_ancestor.parent = an_ancestor
                    an_ancestor.children.append(another_ancestor)

                # Same for right
                elif a_level < idx + 1 and another_level == idx + 1:
                    an_ancestor.parent = another_ancestor
                    another_ancestor.children.append(an_ancestor)

                # If all this fails I think that's also bad
                else:
                    raise InvalidLCAMatrix

    # The LCAs aren't guaranteed to actually be "lowest" ancestors, we need to make sure
    # by pulling down any nodes that can be (i.e. have all children more than one level down)
    for leaf in leaves:
        _pull_down(leaf)

    # We have created the tree structure, let's initialize the adjacency matrix and find the root to traverse from
    root = _get_ancestor(leaves[0])

    return root, total_nodes


def lca_to_adjacency(lca_matrix):
    """
    Converts a tree's LCA matrix representation, i.e. a square matrix (M, M) where each row/column corresponds to
    a leaf of the tree and each matrix entry is the level of the lowest-common-ancestor (LCA) of the two leaves, into
    the corresponding two-dimension adjacency matrix (N,N), with M < N. The levels are enumerated top-down from the
    root.

    .. seealso::
        The pseudocode for LCA to tree conversion is described in
        `Kahn et al <https://iopscience.iop.org/article/10.1088/2632-2153/ac8de0>`_.

    :param lca_matrix: 2-dimensional LCA matrix (M, M).
    :type lca_matrix: `Tensor <https://pytorch.org/docs/stable/tensors.html#torch.Tensor>`_

    :return: 2-dimensional matrix (N, N) encoding the graph's node adjacencies. Linked nodes have values unequal to zero.
    :rtype: `Tensor <https://pytorch.org/docs/stable/tensors.html#torch.Tensor>`_

    Raises:
        InvalidLCAMatrix: If passed LCA matrix is malformed (e.g. not 2d or not square) or does not encode a tree.
    """

    # Ensure input is torch tensor or can be converted to it
    if not isinstance(lca_matrix, t.Tensor):
        try:
            lca_matrix = t.Tensor(lca_matrix)
        except TypeError as err:
            print(f"Input type must be compatible with torch Tensor: {err}")
            raise

    # Ensure two dimensions
    if len(lca_matrix.shape) != 2:
        raise InvalidLCAMatrix

    # Ensure that it is square
    n, m = lca_matrix.shape
    if n != m:
        raise InvalidLCAMatrix

    # Check symmetry
    if not (lca_matrix == lca_matrix.T).all():
        raise InvalidLCAMatrix

    try:
        root, total_nodes = _reconstruct(lca_matrix)
    except IndexError:
        raise InvalidLCAMatrix

    # Allocate the adjacency matrix
    adjacency_matrix = t.zeros((total_nodes, total_nodes), dtype=t.int64)
    try:
        _breadth_first_adjacency(root, adjacency_matrix)
    except IndexError:
        raise InvalidLCAMatrix

    # Check whether what we reconstructed is actually a tree - might be a regular graph for example
    if not is_valid_tree(adjacency_matrix):
        raise InvalidLCAMatrix

    return adjacency_matrix


def _get_fsps_of_node(node):
    """
    Given a node, finds all the final state particles connected to it and get their indices in the LCA.

    Args:
        node (Node): Node to be inspected.

    Returns:
        indices (list): List of final state particles' indices in the LCA matrix connected to node.
    """
    indices = []

    if node.lca_index is not None:  # If you simply use 'if node.lca_index:' you will always miss the first fsp
        indices.append(node.lca_index)
    else:
        for child in node.children:
            indices.extend(_get_fsps_of_node(child))

    return list(set(indices))


def select_good_decay(predicted_lcas, predicted_masses, sig_side_lcas=None, sig_side_masses=None):
    """
    Checks if given LCAS matrix is found in reconstructed LCAS matrix and mass hypotheses are correct.

    .. warning:: You have to make sure to call this function only for valid tree structures encoded in ``predicted_lcas``,
        otherwise it will throw an exception.

    Mass hypotheses are indicated by letters. The following convention is used:

    .. math::
        'e' \\to e \\\\
        'i' \\to \\pi \\\\
        'k' \\to K \\\\
        'p' \\to p \\\\
        'm' \\to \\mu \\\\
        'g' \\to \\gamma \\\\
        'o' \\to \\text{others}

    .. warning:: The order of mass hypotheses should match that of the final state particles in the LCAS.

    :param predicted_lcas: LCAS matrix.
    :type predicted_lcas: `Tensor <https://pytorch.org/docs/stable/tensors.html#torch.Tensor>`_
    :param predicted_masses: List of predicted mass classes.
    :type predicted_masses: list[str]
    :param sig_side_lcas: LCAS matrix of your signal-side.
    :type sig_side_lcas: `Tensor <https://pytorch.org/docs/stable/tensors.html#torch.Tensor>`_
    :param sig_side_masses: List of mass hypotheses for your FSPs.
    :type sig_side_masses: list[str]

    Returns:
        bool, int, list: True if LCAS and masses match, LCAS level of root node,
        LCA indices of FSPs belonging to the signal side ([-1] if LCAS does not match decay string).
    """

    # Reconstruct decay chain
    root, _ = _reconstruct(predicted_lcas)

    # If root is not Ups nor B then decay is not good
    if root.lcas_level not in [5, 6]:
        return (False, root.lcas_level, [-1])

    # If root is B don't go any further (function is supposed to check wheter signal-side on Ups decay is good)
    if root.lcas_level == 5:
        return (True, 5, [i for i in range(predicted_lcas.shape[0])])

    # If chosen LCAS or masses are None then decay is not good
    if sig_side_lcas is None or sig_side_masses is None:
        return (False, root.lcas_level, [-1])

    # Check if the LCA matrix/masses you chose are valid
    try:
        if sig_side_lcas.item() == 0:
            more_fsps = False
        else:
            raise InvalidLCAMatrix("If you have only one sig-side FSP, the LCA matrix should be [[0]]")
    except ValueError:
        try:
            lca_to_adjacency(sig_side_lcas)
            more_fsps = True
        except InvalidLCAMatrix:
            raise InvalidLCAMatrix("You chose an invalid LCA matrix")

    # Check if the number of FSPs in the LCA is the same as the number of mass hypotheses
    if sig_side_lcas.shape[0] != len(sig_side_masses):
        raise InvalidLCAMatrix("The dimension of the LCA matrix you chose does not match with the number of mass hypotheses")

    # Check if mass hypotheses are allowed
    for e in set(sig_side_masses):
        if e not in ['i', 'o', 'g', 'k', 'm', 'e', 'p']:
            # Ok this is not properly an InvalidLCAMatrix case but I'm too lazy to define dedicated exception
            raise InvalidLCAMatrix("Allowed mass hypotheses are 'i', 'o', 'g', 'k', 'm', 'e', 'p'")

    # Convert mass hypotheses to classes and then to integers
    for s, n in zip(["i", "k", "p", "e", "m", "g", "o"], ["3", "4", "5", "1", "2", "6", "0"]):
        sig_side_masses = list(map(lambda x: x.replace(s, n), sig_side_masses))
    sig_side_masses = t.from_numpy(np.array(sig_side_masses, dtype=int))

    # Let's start the proper decay check
    # Case 1: only one FSP in the signal-side
    if not more_fsps:
        # There should be two nodes: one '5' and one '0'
        if Counter([child.lcas_level for child in root.children]) != Counter({5: 1, 0: 1}):
            return (False, root.lcas_level, [-1])

        # Get FSP index in LCA
        fsp_idx = root.children[0].lca_index if root.children[0].lcas_level == 0 else root.children[1].lca_index

        # Check mass hypothesis
        if predicted_masses[fsp_idx] != sig_side_masses[0]:
            return (False, root.lcas_level, [-1])

        # I think the exceptions are over, decay is good
        return (True, root.lcas_level, [fsp_idx])

    # Case 2: more FSPs in the signal-side
    else:
        # There should be two nodes labelled as '5'
        if Counter([child.lcas_level for child in root.children]) != Counter({5: 2}):
            return (False, root.lcas_level, [-1])

        # If there are two '5', at least one of them should decay into the nodes given by the chosen LCAS/masses
        # Step 1: get LCA indices of both Bs
        B1_indices = _get_fsps_of_node(root.children[0])
        B2_indices = _get_fsps_of_node(root.children[1])

        # Step 2: Loop over the two Bs and select LCA sub-matrix and sub-masses
        for indices in [B1_indices, B2_indices]:
            # Step 3: check whether number of FSPs in the chosen sig-side corresponds to that of one of the B's
            if sig_side_lcas.shape[0] != len(indices):
                continue

            sub_lca = predicted_lcas[indices][:, indices]
            sub_masses = predicted_masses[indices]

            # Step 4: your chosen sig-side LCAS/masses could have different ordering,
            # we have to check all possible permutations
            for permutation in permutations(list(range(len(sub_lca)))):
                permutation = list(permutation)
                permuted_sig_side_lca = sig_side_lcas[permutation][:, permutation]
                permuted_sig_side_masses = sig_side_masses[permutation]
                # Step 5: if one of the permutations works decay is good
                if (permuted_sig_side_lca == sub_lca).all() and (permuted_sig_side_masses == sub_masses).all():
                    return (True, root.lcas_level, indices)

        # If we get here decay is not good
        return (False, root.lcas_level, [-1])
