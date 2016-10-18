from basf2 import *
import os
from collections import deque
from collections import OrderedDict
from collections import namedtuple
from collections import defaultdict
import json
from functools import singledispatch, update_wrapper
import contextlib
import enum
import shutil

import ROOT
from ROOT.Belle2 import PyStoreObj, CalibrationAlgorithm


@enum.unique
class AlgResult(enum.Enum):
    """
    Enum of Calibration results. Shouldn't be very necessary to use this
    over the direct CalibrationAlgorithm members but it's nice to have
    something pythonic ready to go.
    """
    ok = CalibrationAlgorithm.c_OK
    not_enough_data = CalibrationAlgorithm.c_NotEnoughData
    iterate = CalibrationAlgorithm.c_Iterate
    failure = CalibrationAlgorithm.c_Failure

IoV = namedtuple('IoV', ['exp_low', 'run_low', 'exp_high', 'run_high'])
IoV_Result = namedtuple('IoV_Result', ['iov', 'result'])


def iov_from_vector(iov_vector):
    """
    Takes a vector of ExpRun from CalibrationAlgorithm and returns
    the overall IoV from the lowest ExpRun to the highest. It returns
    a tuple of the form ((exp_low, run_low), (exp_high, run_high))
    It assumes that the vector was in order to begine with.
    """
    iov_list = [(iov.first, iov.second) for iov in iov_vector]
    if len(iov_list) > 1:
        iov_low, iov_high = iov_list[0], iov_list[-1]
    else:
        iov_low, iov_high = iov_list[0], iov_list[0]
    return IoV(iov_low[0], iov_low[1], iov_high[0], iov_high[1])


def find_sources(dependencies):
    """
    Returns a deque of node names that have no input dependencies
    """
    # Create an OrderedDict to make sure that our sources are
    # in the same order that we started with
    in_degrees = OrderedDict((k, 0) for k in dependencies)
    for node, adjacency_list in dependencies.items():
        for future_node in adjacency_list:
            in_degrees[future_node] += 1

    # We build a deque of nodes with no dependencies
    sources = deque([])
    for name, in_degree in in_degrees.items():
        if in_degree == 0:
            sources.appendleft(name)

    return sources


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


def all_dependencies(dependencies, order=None):
    """
    Here we pass in a dictionary of the form that is used in topological sort
    where the keys are nodes, and the values are a list of the nodes that depend
    on it.

    However, the value (list) does not necessarily contain all of the future nodes
    that depend on each one, only those that are directly adjacent in the graph.
    So there are implicit dependencies not shown in the list.

    This function calculates the implicit future nodes and returns an OrderedDict
    with a full list for each node. This may be expensive in memory for
    complex graphs so be careful.

    If you care about the ordering of the final OrderedDict you can pass in a list
    of the nodes. The final OrderedDict then has the same order as the order parameter.
    """
    full_dependencies = OrderedDict()

    def add_out_nodes(node, node_set):
        """
        This is a recursive function that follows the tree of adjacent future nodes
        and adds all of them to a set (so that we have unique items)
        """
        for out_node in dependencies[node]:
            node_set.add(out_node)
            add_out_nodes(out_node, node_set)

    if not order:
        order = dependencies.keys()
    # Loop over the nodes in the order and recursively head upwards through explicit
    # adjacent nodes.
    for node in order:
        node_dependencies = set()
        add_out_nodes(node, node_dependencies)
        full_dependencies[node] = list(node_dependencies)

    return full_dependencies


def past_from_future_dependencies(future_dependencies):
    nodes = list(future_dependencies.keys())
    past_dependencies = defaultdict(list)
    for node, deps in future_dependencies.items():
        for dep in deps:
            past_dependencies[dep].append(node)
    return past_dependencies


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
    Just decorate around class methods and their alternate functions:

    @method_dispatch             # Default method
    def my_method(self, default_type, ...):
        ...

    @my_method.register(list)    # Registers list method for dispatch

    def _(self, list_type, ...):
        ...

    Doesn't work the same for property decorated class methods, as these
    return a property builtin not a function and change the method naming.
    Do this type of decoration to get them to work:

    @property
    def my_property(self):
        return self._my_property

    @my_property.setter
    @method_dispatch
    def my_property(self, input_property):
        ...

    @my_property.fset.register(list)
    def _(self, input_list_properties):
        ...
    """
    dispatcher = singledispatch(func)

    def wrapper(*args, **kw):
        return dispatcher.dispatch(args[1].__class__)(*args, **kw)
    wrapper.register = dispatcher.register
    update_wrapper(wrapper, func)
    return wrapper


@contextlib.contextmanager
def temporary_workdir(path):
    """Context manager that changes the working directory to the given
    path and then changes it back to its previous value on exit.
    """
    prev_cwd = os.getcwd()
    os.chdir(path)
    try:
        yield
    finally:
        os.chdir(prev_cwd)


class PathExtras():
    """
    Simple wrapper for basf2 paths to allow some extra python functionality directly on
    them e.g. comparing whether or not a module is contained within a path with 'in' keyword.
    """

    def __init__(self, path=None):
        """
        Initialising with a path
        """
        if path:
            #: Attribute to hold path object that this class wraps
            self.path = path
        else:
            path = []
        #: Holds a list of module names for the path in self.path
        self._module_names = []
        self._update_names()

    def _update_names(self):
        """
        Takes the self.path attribute and uses the current state to recreate the
        self.module_names list
        """
        for module in self.path.modules():
            self._module_names.append(module.name())

    def __contains__(self, module_name):
        """
        Special method to allow 'module_name in path' type comparisons. Returns
        a boolean and compares by module name.
        """
        self._update_names()
        return module_name in self._module_names

    def index(self, module_name):
        """
        Returns the index of the first instance of a module in the contained path.
        """
        return self._module_names.index(module_name)


def merge_local_databases(list_database_dirs, output_database_dir):
    """
    Takes a list of database directories and merges them into one new directory,
    defined by the output_database_dir.
    It assumes that each of the database directories is of the standard form:

    directory_name
        -> database.txt
        -> <payload file name>
        -> <payload file name>
        -> ...
    """
    os.mkdir(output_database_dir)
    database_file_path = os.path.join(output_database_dir, 'database.txt')
    with open(database_file_path, 'w') as db_file:
        for directory in list_database_dirs:
            if not os.path.exists(directory):
                B2WARNING("Database directory {0} requested by collector but it doesn't exist!".format(directory))
                continue
            else:
                # Get only the files, not directories
                listdir, isfile, join = os.listdir, os.path.isfile, os.path.join
                file_names = [file_name for file_name in listdir(directory) if isfile(join(directory, file_name))]
                file_names.remove('database.txt')
                # Now we need the absolute paths to all of the payload files so we can copy them across
                file_names = [os.path.join(directory, file_name) for file_name in file_names[:]]
                for file_name in file_names:
                    shutil.copy(file_name, output_database_dir)
                # Now grab all the IoV stuff from each database.txt files and merge it.
                with open(os.path.join(directory, 'database.txt'), 'r') as f:
                    for line in f.readlines():
                        db_file.write(line)
