from scipy.linalg import block_diag
import torch as t


def decay2adjacency(root):
    """Return the adjacency matrix of a decay

    Args:
        root (phasepace.GenParticle): The root particle of a decay tree.

    Returns:
        numpy.ndarray, list: The adjacency matrix of the decay and the list of
            GenParticle names corresponding to each row and column.
    """

    # List of adjacency matrices to combine as block diagonal
    # Initialise it with the trivial adjacency of the node by itself
    adj_list = [t.zeros(1)]

    # Also need to keep track of the size of sub adjacencies to add connections to children
    # Again initialise with current node
    adj_pos = [1]

    # List of particle names for each position in final adjacency matrix
    names = [root.name]

    # Hitting rock bottom doesn't mean that you have to stay there
    if not root.has_children:
        return adj_list[0], names

    # Collect up the adjacency matrices of all children
    for child in root.children:
        # Recurse
        child_adj, child_names = decay2adjacency(child)
        adj_list.append(child_adj)
        names.extend(child_names)

        # Notice this is a cumulative sum since we're interested in the child's
        # position in the final adjacency matrix
        adj_pos.append(adj_list[-1].shape[0] + adj_pos[-1])

    # Now create the block diagonal of them all
    adj_mat = block_diag(*adj_list)
    # Just because scipy returns a numpy array and we want torch tensors
    adj_mat = t.Tensor(adj_mat)

    # And finally mark the connections from root to all the children
    for pos in adj_pos[:-1]:
        adj_mat[0, pos] = 1
        adj_mat[pos, 0] = 1

    return adj_mat, names
