import numpy as np


def decay2lca(root):
    """Return the LCA matrix of a decay

    Args:
        root (phasespace.GenParticle): the root particle of a decay tree

    Returns:
        numpy.ndarray, list: the lca matrix of the decay and the list of
        GenParticle names corresponding to each row and column

    """
    # NOTE find all leaf nodes
    node_list = [root]
    leaf_nodes = []
    parents = {
        root.name: None,
    }
    generations = {root.name: 0}
    while len(node_list) > 0:
        node = node_list.pop(0)
        # NOTE since there is no get parent
        for c in node.children:
            if c.name in parents:
                raise (ValueError("Node names have to be unique!"))
            parents[c.name] = node
            generations[c.name] = generations[node.name] + 1
        if len(node.children) == 0:
            leaf_nodes.append(node)
        else:
            node_list = [c for c in node.children] + node_list
    # NOTE init results
    names = [leaf_node.name for leaf_node in leaf_nodes]
    lca_mat = np.zeros((len(leaf_nodes), len(leaf_nodes)), dtype=int)

    # NOTE fix skipped generations such that leaves are all in the same one
    # NOTE and nodes can be "pulled down"
    depth = max({generations[name] for name in names})
    for name in names:
        generations[name] = depth
    node_list = [leaf_node for leaf_node in leaf_nodes]
    while len(node_list) > 0:
        node = node_list.pop(0)
        parent = parents[node.name]
        if parent is not None:
            node_list.append(parent)
            if len(node.children) > 0:
                generations[node.name] = (
                    min({generations[n.name] for n in node.children}) - 1
                )

    # NOTE trace ancestry for all leaves to root
    for i in range(len(leaf_nodes)):
        for j in range(i + 1, len(leaf_nodes)):
            _lca = _find_lca(leaf_nodes[i], leaf_nodes[j], parents, generations)
            lca_mat[i, j] = _lca
            lca_mat[j, i] = _lca

    return lca_mat, names


def _find_lca(node1, node2, parents, generations):
    if node1.name == node2.name:
        raise ValueError("{} and {} have to be different.".format(node1, node2))
    ancestry1 = []
    parent = node1
    while parent is not None:
        ancestry1.append(parent)
        parent = parents[parent.name]

    ancestry2 = []
    parent = node2
    while parent is not None:
        ancestry2.append(parent)
        parent = parents[parent.name]

    ancestry1.reverse()
    ancestry2.reverse()

    # NOTE basically find common subroot
    for i, x in enumerate(ancestry1):
        if x.name != ancestry2[i].name:
            subroot = parents[x.name]
            # g = generations[subroot.name]
            return generations[node1.name] - generations[subroot.name]

    return 0
