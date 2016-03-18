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

import collections

import ROOT
from ROOT.Belle2 import CalibrationAlgorithm


class Calibration():
    """
    Every Calibration object must have a collector and at least one algorithm.
    You have the option to add in your collector/algorithm by argument here, or
    later by changing the properties.

    Calibration(collector, algorithms, max_iterations)
    - 'collector' should be a CalibrationCollectorModule or a string with the module name.
    - 'algorithms' should be a CalibrationAlgorithm or a list of them.
    - 'max iterations' is set to 5 by default but can be set to any >0
      integer. The overall calibration will fail if max iterations is reached.

    ## Can set everything up step by step ##
    >>> cal = Calibration()
    >>> cal.name = 'TestCalibration1'

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
    >>> cal = Calibration( 'CaTest', [alg1,alg2], 'TestCalibration1', 2)
    """

    def __init__(self, collector=None, algorithms=None, name=None, max_iterations=5):
        self._manual_name_set = False

        self._collector, self._algorithms = None, None
        self.collector = collector
        self.algorithms = algorithms
        self.max_iterations = max_iterations

        if name:
            self.name = name
        else:
            self._create_name()

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

    def _create_name(self):
        if self.is_valid():
            alg_sep = ','
            algs = alg_sep.join([type(alg).__name__ for alg in self.algorithms])
            sep = '-'
            self._name = 'Calibration('+sep.join([str(id(self)), self.collector.name(), '['+algs+']'])+')'
        # If we don't have a valid Calibration the name should still be unique so we use the id().
        else:
            self._name = 'Calibration('+str(id(self))+')'

    @property
    def name(self):
        return self._name

    @name.setter
    def name(self, name):
        if isinstance(name, str):
            self._name = name
            self._manual_name_set = True
        else:
            B2FATAL("Tried to set Calibration name to a non-string type")

    @property
    def collector(self):
        return self._collector

    @collector.setter
    def collector(self, collector):
        # check if collector is already a module or if we need to create one
        # from the name
        if collector:
            if isinstance(collector, str):
                collector = register_module(collector)
            if not isinstance(collector, Module):
                print(collector)
                B2FATAL("Collector needs to be either a Module or the name of such a module")
        self._collector = collector
        if not self._manual_name_set:
            self._create_name()

    @property
    def algorithms(self):
        return self._algorithms

    @algorithms.setter
    def algorithms(self, algorithms):
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
        if not self._manual_name_set:
            self._create_name()

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
        self.calibrations = []
        self.dependencies = collections.defaultdict(list)

    def add_calibration(self, calibration):
        """Adds calibrations that are to be used in this program to the list.
        """
        if calibration.is_valid():
            self.calibrations.append(calibration)
        else:
            B2WARNING(("Tried to add incomplete/invalid calibration to the framwork."
                       "It was not added and will not be part of the final process."))

    def add_dependency(self, calibration_name, depends_on):
        """Adds dependency of one entire calibration step on another by
        using their unique names found from:
        >>> cal.name
        You can define multiple dependencies for a single calibration simply
        by calling this multiple times. Be careful not to add a circular/cyclic
        dependency or the CAF job will fail.
        """
        if calibration_name != depends_on:
            # Tests if we have the calibrations added
            if set([calibration_name, depends_on]).issubset([cal.name for cal in self.calibrations]):
                    self.dependencies[calibration_name].append(depends_on)
            else:
                B2WARNING(("Tried to add dependency for calibrations not in the framework."
                           "Dependency was not added."))
        else:
            B2WARNING(("Tried to add dependency for calibration on itself."
                       "Dependency was not added."))
