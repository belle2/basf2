#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This module contains various utility functions for the CAF and Job submission Backends to use.
"""

from basf2 import *
import os
from collections import deque
from collections import OrderedDict
from collections import namedtuple
from collections import defaultdict
import pathlib
import json
from functools import singledispatch, update_wrapper
import contextlib
import enum
import shutil

import ROOT
from ROOT.Belle2 import PyStoreObj, CalibrationAlgorithm, IntervalOfValidity

#: A newline string for B2INFO that aligns with the indentation of B2INFO's first line
b2info_newline = "\n" + (7 * " ")


def B2INFO_MULTILINE(lines):
    """
    Parameters:
        lines (list[str]): Lines to be printed in a single call to B2INFO

    Quick little function that creates a string for B2INFO from a list of strings.
    But it appends a newline character + the necessary indentation to the follwing line
    so that the B2INFO output is nicely aligned.
    Then it calls B2INFO on the output.
    """
    log_string = b2info_newline.join(lines)
    B2INFO(log_string)


def find_gaps_in_iov_list(iov_list):
    """
    Finds the runs that aren't covered by the input IoVs in the list. This cannot find missing
    runs which lie between two IoVs that are separated by an experiment e.g. between
    IoV(1,1,1,10) => IoV(2,1,2,5) it is unknown if there were supposed to be more runs than run
    number 10 in experiment 1 before starting experiment 2. Therefore this is not counted as a gap
    and will not be added to the output list of IoVs

    Parameters:
        iov_list (list[IoV]): A SORTED list of Non-overlapping IoVs that you want to check for 'gaps'
                              i.e. runs that aren't covered.

    Returns:
        list[IoV]: The IoVs corresponding to gaps in the input list of IoVs
    """
    gaps = []
    previous_iov = None
    for current_iov in iov_list:
        if previous_iov:
            previous_highest = ExpRun(previous_iov.exp_high, previous_iov.run_high)
            current_lowest = ExpRun(current_iov.exp_low, current_iov.run_low)
            iov_gap = previous_highest.find_gap(current_lowest)
            if iov_gap:
                B2DEBUG(29, "Gap found between {} and {} = {}".format(previous_iov, current_iov, iov_gap))
                gaps.append(iov_gap)
        previous_iov = current_iov
    return gaps


class ExpRun(namedtuple('ExpRun_Factory', ['exp', 'run'])):
    """
    Class to define a single (Exp,Run) number i.e. not an IoV.
    It is derived from a namedtuple created class.

    We use the name 'ExpRun_Factory' in the factory creation so that
    the MRO doesn't contain two of the same class names which is probably fine
    but feels wrong.

    KeyWord Arguments:
        exp (int): The experiment number
        run (int): The run number
    """

    def make_iov(self):
        """
        Returns:
            IoV: A simple IoV corresponding to this single ExpRun
        """
        return IoV(self.exp, self.run, self.exp, self.run)

    def find_gap(self, other):
        """
        Finds the IoV gap bewteen these two ExpRuns
        """
        lower, upper = sorted((self, other))
        if lower.exp == upper.exp and lower.run != upper.run:
            if (upper.run - lower.run) > 1:
                return IoV(lower.exp, lower.run + 1, lower.exp, upper.run - 1)
            else:
                return None
        else:
            return None


class IoV(namedtuple('IoV_Factory', ['exp_low', 'run_low', 'exp_high', 'run_high'])):
    """
    Python class to more easily manipulate an IoV and compare against others.
    Uses the C++ framework IntervalOfValidity internally to do various comparisons.
    It is derived from a namedtuple created class.

    We use the name 'ExpRun_Factory' in the factory creation so that
    the MRO doesn't contain two of the same class names which is probably fine
    but feels wrong.

    Default construction is an 'empty' IoV of -1,-1,-1,-1
    e.g. i = IoV() => IoV(exp_low=-1, run_low=-1, exp_high=-1, run_high=-1)

    For an IoV that encompasses all experiments and runs use 0,0,-1,-1
    """

    def __new__(cls, exp_low=-1, run_low=-1, exp_high=-1, run_high=-1):
        """
        The special method to create the tuple instance. Returning the instance
        calls the __init__ method
        """
        return super().__new__(cls, exp_low, run_low, exp_high, run_high)

    def __init__(self, exp_low=-1, run_low=-1, exp_high=-1, run_high=-1):
        """
        Called after __new__
        """
        self._cpp_iov = IntervalOfValidity(self.exp_low, self.run_low, self.exp_high, self.run_high)

    def contains(self, iov):
        """
        Check if this IoV contains another one that is passed in.
        """
        return self._cpp_iov.contains(iov._cpp_iov)

    def overlaps(self, iov):
        """
        Check if this IoV overlaps another one that is passed in.
        """
        return self._cpp_iov.overlaps(iov._cpp_iov)


@enum.unique
class AlgResult(enum.Enum):
    """
    Enum of Calibration results. Shouldn't be very necessary to use this
    over the direct CalibrationAlgorithm members but it's nice to have
    something pythonic ready to go.
    """
    #: OK Return code
    ok = CalibrationAlgorithm.c_OK
    #: not enought data Return code
    not_enough_data = CalibrationAlgorithm.c_NotEnoughData
    #: iteration required Return code
    iterate = CalibrationAlgorithm.c_Iterate
    #: failure Return code
    failure = CalibrationAlgorithm.c_Failure


IoV_Result = namedtuple('IoV_Result', ['iov', 'result'])


def runs_overlapping_iov(iov, runs):
    """
    Takes an overall IoV() object and a list of Exp,Run tuples (i,j)
    and returns the list of Exp,Run tuples containing only those runs that overlap
    with the IoV range.
    """
    overlapping_runs = []
    for run in runs:
        # Construct an IOV of one run
        run_iov = IoV(run[0], run[1], run[0], run[1])
        if run_iov.overlaps(iov):
            overlapping_runs.append(run)
    return overlapping_runs


def iov_from_runs(runs):
    """
    Takes a list of (Exp,Run) and returns the overall IoV from the lowest ExpRun to the highest.
    It returns an IoV() object and assumes that the list was in order to begin with.
    """
    if len(runs) > 1:
        exprun_low, exprun_high = runs[0], runs[-1]
    else:
        exprun_low, exprun_high = runs[0], runs[0]
    return IoV(exprun_low[0], exprun_low[1], exprun_high[0], exprun_high[1])


def iov_from_runvector(iov_vector):
    """
    Takes a vector of ExpRun from CalibrationAlgorithm and returns
    the overall IoV from the lowest ExpRun to the highest. It returns
    an IoV() object. It assumes that the vector was in order to begin with.
    """
    import copy
    exprun_list = [list((iov.first, iov.second)) for iov in iov_vector]
    if len(exprun_list) > 1:
        exprun_low, exprun_high = exprun_list[0], exprun_list[-1]
    else:
        exprun_low, exprun_high = exprun_list[0], copy.deepcopy(exprun_list[0])
    return IoV(exprun_low[0], exprun_low[1], exprun_high[0], exprun_high[1])


def runs_from_vector(exprun_vector):
    """
    Takes a vector of ExpRun from CalibrationAlgorithm and returns
    a Python list of (exp,run) tuples in the same order.
    """
    return [(exprun.first, exprun.second) for exprun in exprun_vector]


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

    >>> @method_dispatch             # Default method
    >>> def my_method(self, default_type, ...):
    >>>     pass

    >>> @my_method.register(list)    # Registers list method for dispatch
    >>> def _(self, list_type, ...):
    >>>     pass

    Doesn't work the same for property decorated class methods, as these
    return a property builtin not a function and change the method naming.
    Do this type of decoration to get them to work:

    >>> @property
    >>> def my_property(self):
    >>>     return self._my_property

    >>> @my_property.setter
    >>> @method_dispatch
    >>> def my_property(self, input_property):
    >>>     pass

    >>> @my_property.fset.register(list)
    >>> def _(self, input_list_properties):
    >>>     pass
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


def get_iov_from_file(file_path):
    """
    Returns an IoV of the exp/run contained within the given file.
    Uses the b2file-metadata-show basf2 tool.
    """
    import subprocess
    metadata_output = subprocess.check_output(['b2file-metadata-show', '--json', file_path])
    m = json.loads(metadata_output.decode('utf-8'))
    return IoV(m['experimentLow'], m['runLow'], m['experimentHigh'], m['runHigh'])


def get_file_iov_tuple(file_path):
    """
    Simple little function to return both the input file path and the relevant IoV, instead of just the IoV
    """
    B2INFO("Finding IoV for {}".format(file_path))
    return (file_path, get_iov_from_file(file_path))


def make_file_to_iov_dictionary(file_path_patterns, polling_time=10, pool=None):
    """
    Takes a list of file path patterns (things that glob would understand) and runs b2file-metadata-show over them to
    extract the IoV.

    Paramters:
        file_path_patterns (list[str]): The list of file path patterns you want to get IoVs for.

    Keyword Arguments:
        polling_time (int): Time between checking if our results are ready.
        pool: Optional Pool object used to multprocess the b2file-metadata-show subprocesses.
            We don't close or join the Pool as you might want to use it yourself, we just wait until the results are ready.

    Returns:
        dict: Maping of matching input file paths (Key) to their IoV (Value)
    """
    absolute_file_paths = find_absolute_file_paths(file_path_patterns)
    file_to_iov = {}
    if not pool:
        for file_path in absolute_file_paths:
            B2INFO("Finding IoV for {}".format(file_path))
            file_to_iov[file_path] = get_iov_from_file(file_path)
    else:
        import time
        results = []
        for file_path in absolute_file_paths:
            results.append(pool.apply_async(get_file_iov_tuple, (file_path,)))

        while True:
            if all(map(lambda result: result.ready(), results)):
                break
            B2INFO("Still waiting for IoVs to be calculated.")
            time.sleep(polling_time)

        for result in results:
            file_iov = result.get()
            file_to_iov[file_iov[0]] = file_iov[1]

    return file_to_iov


def find_absolute_file_paths(file_path_patterns):
    """
    Takes a file path list (including wildcards) and performs glob.glob()
    to extract the absolute file paths to all matching files.

    Also uses set() to prevent multiple instances of the same file path
    but returns a list of file paths.

    Any root:// urls are taken as absolute file paths already and are simply
    passed through. They should NOT contain wildcard patterns.
    """
    import glob
    existing_file_paths = set()
    for file_pattern in file_path_patterns:
        if file_pattern[:7] != "root://":
            input_files = glob.glob(file_pattern)
            if not input_files:
                B2WARNING("No files matching {0} can be found, it will be skipped!".format(file_pattern))
            else:
                for file_path in input_files:
                    file_path = os.path.abspath(file_path)
                    if os.path.isfile(file_path):
                        existing_file_paths.add(file_path)
        else:
            B2INFO(("Found an xrootd file path {0} it will not be checked for validity.".format(input_file_path)))
            existing_file_paths.add(file_pattern)

    abs_file_paths = list(existing_file_paths)
    return abs_file_paths


def parse_raw_data_iov(file_path):
    """
    For as long as the Raw data is stored using a  predictable directory/filename structure
    we can take advantage of it to more quickly infer the IoV of the files.

    Parameters:
        file_path (str): The absolute file path of a Raw data file on KEKCC

    Returns:
        `IoV`: The Single Exp,Run IoV that the Raw data file corresponds to.
    """
    Path = pathlib.Path
    file_path = Path(file_path)

    # We'll try and extract the exp and run from both the directory and filename
    # That wil let us check that everything is as we expect

    reduced_path = file_path.relative_to("/hsm/belle2/bdata/Data/Raw")
    path_exp = int(reduced_path.parts[0][1:])
    path_run = int(reduced_path.parts[1][1:])

    split_filename = reduced_path.name.split(".")
    filename_exp = int(split_filename[1])
    filename_run = int(split_filename[2])

    if path_exp == filename_exp and path_run == filename_run:
        return IoV(path_exp, path_run, path_exp, path_run)
    else:
        raise ValueError("Filename and directory gave different IoV after parsing for: {}".format(file_path))


def create_directories(path, overwrite=True):
    """
    Creates a new directory path. If it alread exists it will either leave it as is (including any contents),
    or delete it and re-create it fresh. It will only delete the end point, not any intermediate directories created
    """
    # Delete if overwriting and it exists
    if (path.exists() and overwrite):
        shutil.rmtree(path)
    # If it never existed or we just deleted it, make it now
    if not path.exists():
        os.makedirs(path)
