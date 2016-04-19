from basf2 import *
from collections import deque
import json
from functools import singledispatch, update_wrapper

import ROOT
from ROOT.Belle2 import PyStoreObj, CalibrationAlgorithm


#: Get friendly names for the return values
algorithm_result_names = {
    CalibrationAlgorithm.c_OK: "OK",
    CalibrationAlgorithm.c_NotEnoughData: "NotEnoughData",
    CalibrationAlgorithm.c_Iterate: "Iterate",
    CalibrationAlgorithm.c_Failure: "Failure",
}


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
    ['c', 'b', 'a']
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


def decode_json_string(object_string):
    """
    Simple function to call json.loads() on a string to return the
    Python object constructed (Saves importing json everywhere).
    """
    return json.loads(object_string)


def method_dispatch(func):
    """
    Decorator that behaves exactly like functools.singledispatch
    but which takes the second argument to be the important one
    that we want to check the type of and dispatch to the correct function.

    This is needed when trying to dispatch a method in a class, since the
    first argument of the method is always 'self'.
    """
    dispatcher = singledispatch(func)

    def wrapper(*args, **kw):
        return dispatcher.dispatch(args[1].__class__)(*args, **kw)
    wrapper.register = dispatcher.register
    update_wrapper(wrapper, func)
    return wrapper


class CalibrationAlgorithmRunner(Module):
    """
    Algorithm runner.

    This module is responsible of calling the CalibrationAlgorithm with the
    correct run iovs and record all calibration results.
    (Shamelessly stolen from calibration_framework.py)
    """

    def __init__(self, algorithm):
        """Constructor"""
        super().__init__()
        #: Algorithm used in this runner
        self.algorithm = algorithm
        #: list of currently collected iovs
        self.runs = set()

    def beginRun(self):
        """Collect all runs we have seen"""
        event = PyStoreObj("EventMetaData").obj()
        self.runs.add((event.getExperiment(), event.getRun()))

    def execute(self, runs):
        """Execute the algorithm over list of runs"""
        # create std::vector<ExpRun> for the argument
        iov_vec = ROOT.vector("std::pair<int,int>")()
        pair = ROOT.pair("int", "int")()
        for run in runs:
            pair.first, pair.second = run
            iov_vec.push_back(pair)
        # run the algorithm
        result = self.algorithm.execute(iov_vec, 1)  # Use 1 iteration as default for now
        return result

    def terminate(self):
        """Run the calibration algorithm at the end of the process"""
        runs = []  # Start with no runs
        for run in sorted(self.runs):
            runs.append(run)  # Add in some seen runs and execute over them
            result = self.execute(runs)
            # if anything else then NotEnoughData is returned then we
            # empty the list of runs for the next call.
            if result != CalibrationAlgorithm.c_NotEnoughData:
                runs = []
