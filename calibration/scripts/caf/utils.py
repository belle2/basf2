from basf2 import *
from collections import deque, defaultdict


def topological_sort(dependencies):
    """
    Does a topological sort of a graph (dictionary) where the keys are the
    node names, and the values are lists of node names that depend on the
    key (including zero dependencies). It should return the sorted
    list of nodes.
    >>> dependencies = {}
    >>> dependencies['c'] = ['a','b']
    >>> dependencies['b'] = ['a']
    >>> dependencies['a'] = []
    >>> sorted = topological_sort(dependencies)
    >>> print(sorted)
    ['a', 'b', 'c']
    """
    # We find the In-degree (number of dependencies) for each node
    # and store it.
    in_degrees = {k: 0 for k in dependencies}
    for node, adjacency_list in dependencies.items():
        for future_node in adjacency_list:
            in_degrees[future_node] += 1

    # We build a deque of nodes with no dependencies
    sources = deque([])
    for name, in_degree in in_degrees.items():
        if in_degree == 0:
            sources.appendleft(name)

    order = []
    while sources:  # Keep adding and removing from this until solved
        source = sources.pop()  # Pick a node with no dependencies
        order.append(source)  # Add it to our ordered nodes
        for node in dependencies[source]:  # Remove vertices from adjacent nodes
            in_degrees[node] -= 1
            if in_degrees[node] == 0:  # If we've created a new source, add it.
                sources.appendleft(node)

    if len(order) == len(dependencies):  # Check if all nodes were ordered
        return order                     # If not, then there was a cyclic dependence
    else:
        B2WARNING('Cyclic dependency detected, check CAF.add_dependency() calls')
        return []
