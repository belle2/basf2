import torch as t
# import numpy as np
# import re
# from collections import defaultdict, Counter
from collections import Counter
from .tree_utils import is_valid_tree


class InvalidLCAMatrix(Exception):
    """
    Specialized Exception sub-class raised for malformed LCA-gram-matrices or LCA-gram-matrices that do not encode trees.
    """

    pass


class BadDecayString(Exception):
    """
    Specialized Exception sub-class raised for invalid decay strings.
    """

    pass


class Node:
    """
    Class to hold levels of nodes in the tree.
    """

    def __init__(self, level, children, lca_index=None, lcas_level=0):
        self.level = level
        self.children = children
        self.lca_index = lca_index
        self.lcas_level = lcas_level

        # self.actual_level = level  # It won't be modified
        self.parent = None
        self.bfs_index = -1
        self.dfs_index = -1


# def _print_history(node):
#     """
#     Debug function to print history of node.

#     Args:
#         node (Node): root node to inspect.
#     """

#     print([child.lcas_level for child in node.children])
#     for child in node.children:
#         _print_history(child)

def _get_ancestor(node):
    """
    Trail search for the highest ancestor of a node.

    Args:
        node (Node): A node instance for which to determine the ancestor.

    Returns:
        ancestor (Node): the node's ancestor, returns self if a disconnected leaf node.
    """
    ancestor = node

    while ancestor.parent is not None:
        ancestor = ancestor.parent

    return ancestor


def _nodes_in_ancestors_children(parent, node1, node2):
    """
    Check if any node in parent's line of descent is also an ancestor of both node1 and node2.

    Args:
        parent (Node): A node instance used to check its line of descent.
        node1 (Node): A node instance in the decay tree.
        node2 (Node): A node instance in the decay tree.

    Returns:
        (bool): True if both node1 and node2 have a common ancestor in parent's line of descent.
    """
    for child in parent.children:
        if (node1 in child.children) and (node2 in child.children):
            return True
        else:
            _nodes_in_ancestors_children(child, node1, node2)

    return False


def _pull_down(node):
    """
    Work up the node's history, pulling down a level any nodes
    whose children are all more than one level below.

    Performs the operation in place.

    Args:
        node (Node): A node instance for which to determine the ancestor.
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
    Enumerates the tree breadth-first into a queue

    Args:
        root (Node): the root of the tree to get the enumeration of.
        queue (dict): level-indexed dictionary
        adjacency_matrix (iterable): 2-dimensional LCA-gram-matrix (M, M).
    """
    # insert current root node into the queue
    level = root.level
    queue.setdefault(level, []).append(root)

    # enumerate the children
    for child in root.children:
        _breadth_first_enumeration(child, queue, adjacency_matrix)

    return queue


def _breadth_first_adjacency(root, adjacency_matrix):
    """
    Enumerates the tree breadth-first into a queue

    Args:
        root (Node): the root of the tree to get the enumeration of.
        queue (dict): level-indexed dictionary
        adjacency_matrix (iterable): 2-dimensional LCA-gram-matrix (M, M).
    """
    queue = _breadth_first_enumeration(root, {}, adjacency_matrix)

    # on recursion end in the root node, traverse the tree once to assign bfs ids to each node
    index = 0
    for i in range(root.level, 0, -1):
        for node in queue[i]:
            node.bfs_index = index
            index += 1

    # then traverse the tree again to fill in the adjacencies
    for i in range(root.level, 0, -1):
        for node in queue[i]:
            for child in node.children:
                adjacency_matrix[node.bfs_index, child.bfs_index] = 1
                adjacency_matrix[child.bfs_index, node.bfs_index] = 1


def _depth_first_enumeration(root, index):
    """
    Fills the passed adjacency matrix based on the tree spanned by `root`.

    Args:
        root (Node): the root of the tree to enumerate the indices of.
        index (int): 2-dimensional adjacency matrix to be filled (N, N).
    """
    root.dfs_index = index
    branch_index = index

    for child in root.children:
        branch_index = _depth_first_enumeration(child, branch_index + 1)

    return branch_index


def _depth_first_labeling(root, adjacency_matrix):
    """
    Sets the adjacencies for the a node and its children depth-first.

    Args:
        root (Node): the root of the tree to get the adjacency matrix of.
        lca_matrix (iterable): 2-dimensional adjacency matrix to be filled (N, N).
    """
    for child in root.children:
        adjacency_matrix[child.dfs_index, root.dfs_index] = 1
        adjacency_matrix[root.dfs_index, child.dfs_index] = 1

        _depth_first_labeling(child, adjacency_matrix)


def _depth_first_adjacency(root, adjacency_matrix):
    """
    Fills the passed adjacency matrix based on the tree spanned by `root`.

    Args:
        root (Node): the root of the tree to get the adjacency matrix of.
        lca_matrix (iterable): 2-dimensional adjacency matrix to be filled (N, N).
    """
    _depth_first_enumeration(root, index=0)
    _depth_first_labeling(root, adjacency_matrix)


def _reconstruct(lca_matrix):
    """
    Does the actual heavy lifting of the adjacency matrix reconstruction. Traverses the LCA matrix level-by-level,
    starting at one. For each level new nodes have to be inserted into the adjacency matrix, if a LCA matrix with this
    level number exists. The newly created node(s) will then be connected to the lower leaves, respectively,
    sub-graphs. This function may produce reconstructions that are valid graphs, but not trees.

    Args:
        lca_matrix (iterable): 2-dimensional LCA-gram-matrix (M, M).

    Returns:
        Node: the root node of the reconstructed tree.
        int: the total number of nodes in the tree

    Raises:
        InvalidLCAMatrix: If passed LCA-gram-matrix is not a tree.
    """
    n = lca_matrix.shape[0]
    total_nodes = n
    # depths = int(lca_matrix.max())
    levels = sorted(lca_matrix.unique().tolist())
    # Want to skip over leaves
    levels.remove(0)

    # create nodes for all leaves
    leaves = [Node(1, [], lca_index=i) for i in range(n)]

    # iterate level-by-level through the matrix, starting from immediate connections
    # we can correct missing intermediate levels here too
    # Just use current_level to check the actual LCA entry, once we know which level it is
    # (ignoring missed levels) then use the index (corrected level)
    # for current_level in range(1, depths + 1):
    for idx, current_level in enumerate(levels, 1):
        # iterate through each leaf in the LCA matrix
        for column in range(n):
            # iterate through all corresponding nodes
            # the LCA matrix is symmetric, hence, check only the from the diagonal down
            for row in range(column + 1, n):
                # skip over entries not in current level
                if lca_matrix[row, column] <= 0:
                    raise InvalidLCAMatrix
                elif lca_matrix[row, column] != current_level:
                    continue

                # get the nodes
                a_node = leaves[column]
                another_node = leaves[row]

                # determine the ancestors of both nodes
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
                    # parent = Node(max(a_level, another_level) + 1, [an_ancestor, another_ancestor])
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

                # same for right
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

    # we have created the tree structure, let's initialize the adjacency matrix and find the root to traverse from
    root = _get_ancestor(leaves[0])

    return root, total_nodes


def lca2adjacency(lca_matrix, format="bfs"):
    """
    Converts a tree's LCA-gram matrix representation, i.e. a square matrix (M, M), where each row/column corresponds to
    a leaf of the tree and each matrix entry is the level of the lowest-common-ancestor (LCA) of the two leaves, into
    the corresponding two-dimension adjacency matrix (N,N), with M < N. The levels are enumerated top-down from the
    root.

    Args:
        lca_matrix (iterable): 2-dimensional LCA-gram-matrix (M, M).
        format (string): output format of the generated adjacency matrix. Can be either on of the two 'bfs' or 'dfs' for
            bread-first or depth-first.

    Returns:
        iterable: 2-dimensional matrix (N, N) encoding the graph's node adjacencies. Linked nodes have values unequal to
            zero.

    Raises:
        InvalidLCAMatrix: If passed LCA-gram-matrix is malformed (e.g. not 2d or not square) or does not encode a tree.
    """
    # sanitize the output format
    if format not in {"bfs", "dfs"}:
        raise ValueError(f"format must be one of bfs|dfs, but was {format}")

    # ensure input is torch tensor or can be converted to it
    if not isinstance(lca_matrix, t.Tensor):
        try:
            lca_matrix = t.Tensor(lca_matrix)
        except TypeError as err:
            print(f"Input type must be compatible with torch Tensor: {err}")
            raise

    # ensure two dimensions
    if len(lca_matrix.shape) != 2:
        raise InvalidLCAMatrix

    # ensure that it is square
    n, m = lca_matrix.shape
    if n != m:
        raise InvalidLCAMatrix

    # check symmetry
    if not (lca_matrix == lca_matrix.T).all():
        raise InvalidLCAMatrix

    try:
        root, total_nodes = _reconstruct(lca_matrix)
    except IndexError:
        raise InvalidLCAMatrix

    # allocate the adjacency matrix
    adjacency_matrix = t.zeros((total_nodes, total_nodes), dtype=t.int64)
    try:
        if format == "bfs":
            _breadth_first_adjacency(root, adjacency_matrix)
        else:
            _depth_first_adjacency(root, adjacency_matrix)
    except IndexError:
        raise InvalidLCAMatrix

    # check whether what we reconstructed is actually a tree - might be a regular graph for example
    if not is_valid_tree(adjacency_matrix):
        raise InvalidLCAMatrix

    return adjacency_matrix


def _is_good_decay_string(decay_string):
    """
    Check if decay string has the good syntax.

    Args:
        decay_string (str): decay string to check

    Returns:
        decay_string (str): decay string with some formatting
    """

    # Empty decay string is allowed
    if decay_string == "":
        return decay_string

    # Remove whitespaces from decay string
    decay_string = "".join(decay_string.split())

    # Decay string should start with 5
    try:
        int(decay_string[0])
    except ValueError:
        raise BadDecayString("Decay string should start with 5. Example of decay string:'5 -> 1 p m'")
    if int(decay_string[0]) != 5:
        raise BadDecayString("Decay string should start with 5. Example of decay string:'5 -> 1 p m'")

    # ... and continue with '->'
    if not decay_string[1:3] == "->":
        raise BadDecayString("Decay string should contain '->' just after the root node. Example of decay string:'5 -> 1 p m'")

    FSPs = decay_string[3:]

    # Select only allowed characters
    for e in set(FSPs):
        if e not in ['1', '2', '3', '4', 'i', 'o', 'g', 'k', 'm', 'e', 'p']:
            raise BadDecayString(
                "Allowed characters are 1-4 and 'i', 'o', 'g', 'k', 'm', 'e', 'p'. Example of decay string:'5 -> 1 p m'")

    for p in ['i', 'o', 'g', 'k', 'm', 'e', 'p']:
        decay_string = decay_string.replace(p, "0")

    # If only one FSP, then must be a 0
    if len(FSPs) == 1 and FSPs[0] not in ['i', 'o', 'g', 'k', 'm', 'e', 'p']:
        raise BadDecayString("If the signal side is composed of only one particle, it should be a FSP. Example:'5->p'")

    return decay_string


def _get_fsps_of_node(node):
    """
    Given a node, find all the final state particles connected to it and get their indices in the LCA.

    Args:
        node (Node): node to be inspected.

    Returns:
        indices (list): list of final state particles' indices in the LCA matrix connected to node.
    """
    indices = []

    if node.lca_index is not None:  # If you simply use 'if node.lca_index:' you will always miss the first fsp
        indices.append(node.lca_index)
    else:
        for child in node.children:
            indices.extend(_get_fsps_of_node(child))

    return list(set(indices))


def select_good_decay(lcas_matrix, decay_string):
    """
    Cheks if given decay string is found in LCAS matrix.
    WARNING: you have to make sure to call this function only for valid tree structures,
             because it doesn't throw an exception in that case

    Args:
        lcas_matrix (torch.Tensor): LCAS matrix
        decay_string (str): decay string

    Returns:
        bool: True if LCAS matches decay string, False otherwise
        int: LCAS level of root node
        list: LCA indices of FSPs belonging to the signal side ([-1] if LCAS does not match decay string)
    """

    # Check if decay string has good syntax
    decay_string = _is_good_decay_string(decay_string)

    # Reconstruct decay chain
    root, _ = _reconstruct(lcas_matrix)

    # If decay string is empty then False
    if decay_string == "":
        return (False, root.lcas_level, [-1])

    # If root is not Ups nor B then False
    if root.lcas_level not in [5, 6]:
        return (False, root.lcas_level, [-1])

    # If root is B don't go any further (function is supposed to check wheter signal-side on Ups decay is good)
    if root.lcas_level == 5:
        return (True, 5, [i for i in range(lcas_matrix.shape[0])])

    # Remove root and first arrow
    decay_string = decay_string[3:]
    # Get final state particles
    FSPs = [int(fsp) for fsp in decay_string]

    # Case 1: only one FSP in the signal-side
    # There should be two nodes: one '5' and one '0'
    if len(FSPs) == 1:
        if Counter([child.lcas_level for child in root.children]) != Counter({5: 1, 0: 1}):
            return (False, root.lcas_level, [-1])
        # I think the exceptions are over, LCAS is good
        lca_idx = root.children[0].lca_index if root.children[0].lcas_level == 0 else root.children[1].lca_index
        return (True, root.lcas_level, [lca_idx])

    # Case 2: more FSP in the signal-side
    else:
        # There should be two nodes labelled as '5'
        if Counter([child.lcas_level for child in root.children]) != Counter({5: 2}):
            return (False, root.lcas_level, [-1])
        # If there are two '5', at least one of them should decay into the nodes given by the decay string
        fsps_counter = Counter(FSPs)
        B1_counter = Counter([gchild.lcas_level for gchild in root.children[0].children])
        B2_counter = Counter([gchild.lcas_level for gchild in root.children[1].children])
        if (B1_counter != fsps_counter) and (B2_counter != fsps_counter):
            return (False, root.lcas_level, [-1])

        # # Find all decays of root children
        # decays = re.findall(r"\(.*?\)", decay_string)

        # I think the exceptions are over, LCAS is good
        sig_B = root.children[0] if B1_counter == fsps_counter else root.children[1]
        return (True, root.lcas_level, _get_fsps_of_node(sig_B))
