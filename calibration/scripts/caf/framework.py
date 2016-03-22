#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Calibration Framework - David Dossett 2016

This module implements several objects/functions to configure and run the calibration.
These can then be used to construct the workflow of the calibration job.
Much of the current flow is borrowed from the calibration_framework.py module
and will be extended soon.
"""

from basf2 import *
import os
import shutil
from datetime import datetime
import collections
import pickle

from .utils import topological_sort
import caf.backends

import ROOT
from ROOT.Belle2 import PyStoreObj, CalibrationAlgorithm


class Calibration():
    """
    Every Calibration object must have a collector and at least one algorithm.
    You have the option to add in your collector/algorithm by argument here, or
    later by changing the properties.

    Calibration(name, collector, algorithms, max_iterations)
    - 'name' must be set when you create the class instance. It should be unique
    if you plan to add multiple ones to a CAF().
    - 'collector' should be a CalibrationCollectorModule or a string with the module name.
    - 'algorithms' should be a CalibrationAlgorithm or a list of them.
    - 'max iterations' is set to 5 by default but can be set to any >0
      integer. The overall calibration will fail if max iterations is reached.

    # Example
    >>> cal = Calibration('TestCalibration1')

    >>> col1 = register_module('CaTest')
    >>> cal.collector = col1
    OR
    >>> cal.collector = 'CaTest'  # Automatically create module

    >>> alg1 = TestAlgo()
    >>> alg2 = TestAlgo()
    >>> cal.algorithms = alg1
    OR
    >>> cal.algorithms = [alg1]
    OR
    cal.algorithms = [alg1, alg2]  # Multiple algorithms for one collector

    ## Or can use optional arguments to pass in some/all during initialisation ##
    >>> cal = Calibration( 'TestCalibration1', 'CaTest', [alg1,alg2], 2)
    """

    def __init__(self, name, collector=None, algorithms=None, max_iterations=5):
        """
        You have to specify a unique name for the calibration when creating it.
        You can specify the collector and algorithms later on or here but it won't run
        until those are done.
        """

        self.name = name
        self._collector, self._algorithms = None, None
        self.collector = collector
        self.algorithms = algorithms
        self.max_iterations = max_iterations

    def is_valid(self):
        """A full calibration consists of a collector AND an associated algorithm
        This returns False if either are missing or if the collector and algorithm are mismatched."""
        if (not self.collector or not self.algorithms):
            return False
        else:
            for alg in self.algorithms:
                alg_type = type(alg).__name__
                if self.collector.type() != alg.getCollectorName():
                    B2WARNING(("Algorithm '%s' requested collector '%s' but got '%s'"
                               % (alg_type, alg.getCollectorName(), self.collector.type())))
                    return False
            else:
                return True

    @property
    def name(self):
        """
        Getter for the name property.
        """
        return self._name

    @name.setter
    def name(self, name):
        """
        Setter for the name property. Checks that a string was passed in.
        """
        if isinstance(name, str):
            self._name = name
        else:
            B2FATAL("Tried to set Calibration name to a non-string type")

    @property
    def collector(self):
        """
        Getter for the collector property.
        """
        return self._collector

    @collector.setter
    def collector(self, collector):
        """
        Setter for the collector property, checks if already a module or need to create one from string.
        """
        # check if collector is already a module or if we need to create one
        # from the name
        if collector:
            if isinstance(collector, str):
                collector = register_module(collector)
            if not isinstance(collector, Module):
                print(collector)
                B2FATAL("Collector needs to be either a Module or the name of such a module")
        self._collector = collector

    @property
    def algorithms(self):
        """
        Getter for the algorithms property.
        """
        return self._algorithms

    @algorithms.setter
    def algorithms(self, algorithms):
        """
        Setter for the algorithms property, checks if single or list of algorithms passed in.
        """
        if algorithms:
            # Need to check if an algorithm or a list of them was passed in.
            if isinstance(algorithms, collections.Iterable):
                self._algorithms = []
                for alg in algorithms:
                    if isinstance(alg, CalibrationAlgorithm):
                        self._algorithms.append(alg)
                    else:
                        # print(alg)
                        B2FATAL(("Something other than CalibrationAlgorithm instance passed in."
                                 "Algorithm needs to inherit from Belle2::CalibrationAlgorithm"))
            else:
                if isinstance(algorithms, CalibrationAlgorithm):
                    self._algorithms = [algorithms]
                else:
                    # print(algorithms)
                    B2FATAL(("Something other than CalibrationAlgorithm instance passed in."
                             "Algorithm needs to inherit from Belle2::CalibrationAlgorithm"))
        else:
            self._algorithms = []

    def __repr__(self):
        return self.name


class CAF():
    """
    The top level class to hold calibration objects and process them:
    - It will define the configuration/flow of logic for the calibrations,
    but most of the heavy lifting should be done through outside functions
    or smaller classes to prevent this getting too big.
    - This will eventually be able to either run everything locally, or submit
    collection to a batch system/grid based on user's choice.
    - It should be able to sort the required collection/algorithm steps into a
    valid order based on dependencies.
    - At the most basic level, it should take a set of files to process and
    output/upload calibration constants for the defined calibrations.
    """
    def __init__(self):
        self.calibrations = {}
        self.dependencies = {}
        self.backend = caf.backends.Local()
        self.input_files = []
        self.output_path = 'calibration_results'

    def add_calibration(self, calibration):
        """
        Adds calibrations that are to be used in this program to the list.
        Also adds an empty dependency list to the overall dictionary.
        You should not directly alter a Calibration object after it has been
        added here.
        """
        if calibration.is_valid():
            if calibration.name not in self.calibrations:
                self.calibrations[calibration.name] = calibration
                self.dependencies[calibration.name] = []
            else:
                B2WARNING('Tried to add a calibration with the name '+calibration.name+' twice.')
        else:
            B2WARNING(("Tried to add incomplete/invalid calibration to the framwork."
                       "It was not added and will not be part of the final process."))

    def add_dependency(self, calibration_name, depends_on):
        """Adds dependency of one entire calibration step on another by
        using their unique names found from:
        >>> cal.name
        You can define multiple dependencies for a single calibration simply
        by calling this multiple times. Be careful not to add a circular/cyclic
        dependency or the sort will return an empty list and the CAF job will fail.
        """
        if calibration_name != depends_on:
            # Tests if we have the calibrations added
            if set([calibration_name, depends_on]).issubset([cal for cal in self.calibrations.keys()]):
                    # Note that we add the depends_on as the key and the calibration
                    # name as the value. This is to make the sorting algorithm
                    # simpler and more efficient later.
                    self.dependencies[depends_on].append(calibration_name)
            else:
                B2WARNING(("Tried to add dependency for calibrations not in the framework."
                           "Dependency was not added."))
        else:
            B2WARNING(("Tried to add dependency for calibration on itself."
                       "Dependency was not added."))

    def order_calibrations(self):
        """
        Takes everything put into the dependencies dictionary and passes it to
        a sorting algorithm. Returns True if sort was succesful, False if it
        failed (most likely a cyclic dependency)
        """
        self.order = topological_sort(self.dependencies)
        return bool(self.order)  # Returns False if sort had problems

    def run(self):
        """
        Runs the overall calibration job and saves the output to the output_path.
        """
        self.order_calibrations()
        self._make_output_dir()
        self._make_collector_paths()

    @property
    def backend(self):
        """
        Getter for the backend property.
        """
        return self._backend

    @backend.setter
    def backend(self, backend):
        """
        Setter for the backend property. Checks that a Backend instance was passed in.
        """
        if isinstance(backend, caf.backends.Backend):
            self._backend = backend
        else:
            B2ERROR('backend property must inherit from Backend class')

    def _make_output_dir(self):
        """
        Creates the output directory. If it already exists it will copy the directory tree to
        a new one based on the current time.
        """
        if os.path.isdir(self.output_path):
            B2WARNING('%s output directory already exists. The directory and its contents will be moved'
                      % self.output_path)
            shutil.move(self.output_path, ''.join(datetime.now().isoformat()+'_'+self.output_path))

        # make a new output directory, it shouldn't already exist as we took care of that above
        os.mkdir(self.output_path)

    def _make_collector_paths(self):
        """
        Creates separate basf2 paths for collectors and serializes them in the self.output_path/paths directory
        """
        path_output_dir = os.path.join(self.output_path, 'paths')
        os.mkdir(path_output_dir)
        for calibration_name in self.order:
            path = create_path()
            calibration = self.calibrations[calibration_name]
            path.add_module(calibration.collector)
            path_file_name = calibration.collector.name()+'.pickle'
            with open(os.path.join(path_output_dir, path_file_name), 'bw') as serialized_path_file:
                pickle.dump(serialize_path(path), serialized_path_file)


# class CalibrationAlgorithmRunner(Module):
#     """
#     Algorithm runner.
#
#     This module is responsible of calling the CalibrationAlgorithm with the
#     correct run iovs and record all calibration results.
#     (Shamelessly stolen from calibration_framework.py)
#     """
#
#     def __init__(self, algorithm):
#         """Constructor"""
#         super().__init__()
#         self.algorithm = algorithm
#         # list of currently collected iovs
#         self.runs = set()
#
#     def beginRun(self):
#         """Collect all runs we have seen"""
#         event = PyStoreObj("EventMetaData").obj()
#         self.runs.add((event.getExperiment(), event.getRun()))
#
#     def execute(self, runs):
#         """Execute the algorithm over list of runs"""
#         # create std::vector<ExpRun> for the argument
#         iov_vec = ROOT.vector("std::pair<int,int>")()
#         pair = ROOT.pair("int", "int")()
#         for run in runs:
#             pair.first, pair.second = run
#             iov_vec.push_back(pair)
#         # run the algorithm
#         result = self.algorithm.execute(iov_vec, 1) # Use 1 iteration as default for now
#         return result
#
#     def terminate(self):
#         """Run the calibration algorithm"""
#         runs = []
#         for run in sorted(self.runs):
#             runs.append(run)
#             result = self.execute(runs)
#             # if anything else then NotEnoughData is returned then we
#             # empty the list of runs for the next call
#             if result != CalibrationAlgorithm.c_NotEnoughData:
#                 runs = []
