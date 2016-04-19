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
from .utils import CalibrationAlgorithmRunner
from .utils import decode_json_string
import caf.backends

import ROOT
from ROOT.Belle2 import PyStoreObj, CalibrationAlgorithm

collector_steering_file_path = os.environ.get('BELLE2_LOCAL_DIR')
collector_steering_file_path = os.path.join(collector_steering_file_path, 'calibration/scripts/caf/run_collector_path.py')


class Calibration():
    """
    Every Calibration object must have a collector and at least one algorithm.
    You have the option to add in your collector/algorithm by argument here, or
    later by changing the properties.

    Calibration(name, collector, algorithms, input_files)
    - 'name' must be set when you create the class instance. It should be unique
    if you plan to add multiple ones to a CAF().
    - 'collector' should be a CalibrationCollectorModule or a string with the module name.
    - 'algorithms' should be a CalibrationAlgorithm or a list of them.
    - 'input_files' is set to None by default but can be set to a string/list of strings.
      The input files have to be understood by the backend used e.g. LFNs for DIRAC,
      accessible local paths for Batch/Local.

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

    # Or can use optional arguments to pass in some/all during initialisation #
    >>> cal = Calibration( 'TestCalibration1', 'CaTest', [alg1,alg2], ['/path/to/file.root'])

    # Change the input file list later on before running with CAF()
    >>> cal.input_files = ['path/to/file.root', 'other/path/to/file2.root']
    """

    def __init__(self, name, collector=None, algorithms=None, input_files=None):
        """
        You have to specify a unique name for the calibration when creating it.
        You can specify the collector/algorithms/input files later on or here but it won't run
        until those are done.
        """

        #: Name of calibration object
        self.name = name
        #: Internal calibration collector/algorithms stored for this calibration
        self._collector, self._algorithms = None, None
        #: Publicly accessible collector
        self.collector = collector
        #: Publicly accessible algorithms
        self.algorithms = algorithms
        #: Files used for collection procedure
        self.input_files = input_files

    def is_valid(self):
        """A full calibration consists of a collector AND an associated algorithm AND input_files.
        This returns False if any are missing or if the collector and algorithm are mismatched."""
        if (not self.collector or not self.algorithms or not self.input_files):
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
            #: Internal storage of calibration object name
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
        #: Internal storage of collector attribute
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

    @property
    def input_files(self):
        """
        Getter for the input_files attribute.
        """
        return self._input_files

    @input_files.setter
    def input_files(self, input_files):
        """
        Setter for the input_files attribute. Checks that a string/list of strings was passed in.
        And then builds a list from that.
        """
        if input_files:
            # Need to check if a string or a list of them was passed in.
            if isinstance(input_files, collections.Iterable):
                #: Internal storage of input_files attribute
                self._input_files = []
                for file_path in input_files:
                    if isinstance(file_path, str):
                        self._input_files.append(file_path)
                    else:
                        B2FATAL("Something other than string passed in as an input file.")
            else:
                if isinstance(input_files, str):
                    self._input_files = [input_files]
                else:
                    B2FATAL(("Something other than string passed in as an input file"))
        else:
            self._input_files = []

    def __repr__(self):
        """
        Representation of Calibration class (what happens when you print a Calibration() instance)
        """
        return self.name


class CAF():
    """
    Class to hold calibration objects and process them:
    - It will define the configuration/flow of logic for the calibrations,
    but most of the heavy lifting should be done through outside functions
    or smaller classes to prevent this getting too big.
    - This will eventually be able to either run everything locally, or submit
    collection to a batch system/grid based on user's choice.
    - It should be able to sort the required collection/algorithm steps into a
    valid order based on dependencies.
    - Much of the checking for consistency should be done here and in the calibration
    class. Leaving functions that it calls to assume everything is correctly set up.
    - Choosing which files to use as input should be done from outside during the
    setup of the CAF and Calibration instances. Allowing the CAF to assume that this
    is your most logical set of data to calibrate at the moment and that you want
    constants for ALL runs passed in (whether they are the best constants or not).
    """
    def __init__(self):
        """
        Initialise CAF instance. Sets up some empty containers for attributes.
        Sets the default backend to Local, and some other defaults to  what is
        in the config file.

        Note that the config file is in the calibration/data directory and we assume
        that $BELLE2_LOCAL_DIR is set and that it is the correct release directory to
        access the config file.
        """
        config_file_path = os.environ.get('BELLE2_LOCAL_DIR')
        if config_file_path:
            config_file_path = os.path.join(config_file_path, 'calibration/data/caf.cfg')
            import configparser
            #: Configuration object for CAF, can change the defaults through a single config file
            self.config = configparser.ConfigParser()
            self.config.read(config_file_path)
        else:
            B2FATAL("Tried to use $BELLE2_LOCAL_DIR but it wasn't there. Is basf2 set up?")

        #: Dictionary of calibrations for this CAF instance
        self.calibrations = {}
        #: Dictionary of dependenciesof calibration objects
        self.dependencies = {}
        #: The backend to use for this CAF run (Public)
        self.backend = caf.backends.Local()
        #: Output path to store results of calibration and bookkeeping information
        self.output_dir = self.config['CAF_DEFAULTS']['ResultsDir']

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
        - Takes everything put into the dependencies dictionary and passes it to a sorting
        algorithm.
        - Returns True if sort was succesful, False if it failed (most likely a cyclic dependency)
        """
        #: List to store calibration order after sorting.
        self.order = topological_sort(self.dependencies)
        return bool(self.order)  # Returns False if sort had problems

    def configure_jobs(self, calibrations):
        """
        Creates a Job object for each collector to pass to the Backend.
        """
        #: Dictionary of named job objects to send to the backend
        self.jobs = {}
        for calibration_name in calibrations:
            job = Job(calibration_name)
            self._make_calibration_dir(calibration_name)
            collector_path_file = self._make_collector_path(calibration_name)
            job.output_dir = os.path.join(self.output_dir, calibration_name, 'output')
            job.working_dir = os.path.join(self.output_dir, calibration_name, 'input')
            job.cmd = ['basf2', 'run_collector_path.py']
            job.input_sandbox_files.append(collector_steering_file_path)
            job.input_sandbox_files.append(collector_path_file)
            job.output_files = ['*.mille']
            self.jobs[calibration_name] = job

    def run(self):
        """
        - Runs the overall calibration job, saves the outputs to the output_dir directory,
        and creates database payloads.
        - Upload may be moved to another method or function entirely to give the option of
        monitoring output before committing to database.
        """
        self.order_calibrations()
        # Creates the overall output directory and reset the attribute to use an absolute path to it.
        self.output_dir = self._make_output_dir()

        self.configure_jobs(self.order)

        # Run the collection stage
        for calibration_name in self.order:
            job = self.jobs[calibration_name]
            print("Submitting {0} to Backend".format(calibration_name))
            self.backend.submit(job)

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
            #: Private backend attribute
            self._backend = backend
        else:
            B2ERROR('backend property must inherit from Backend class')

    def _make_output_dir(self):
        """
        Creates the output directory. If it already exists we quit the program to prevent horrible
        problems by either overwriting the files in this directory or moving it to a new name.
        It returns the absolute path of the new output_dir
        """
        if os.path.isdir(self.output_dir):
            B2FATAL('{0} output directory already exists.'.format(self.output_dir))
        else:
            os.mkdir(self.output_dir)
            abs_output_dir = os.path.join(os.getcwd(), self.output_dir)
            if os.path.exists(abs_output_dir):
                return abs_output_dir
            else:
                B2FATAL("Attempted to create output_dir {0}, but it didn't work.".format(abs_output_dir))

    def _make_calibration_dir(self, calibration_name):
        """
        Creates a directory for a calibration object contained in this CAF.
        """
        # This should always work as we will be running _make_output_dir() first and the calibration names
        # should be unique => no need for superfluous checks
        os.mkdir(os.path.join(self.output_dir, calibration_name))

    def _make_collector_path(self, calibration_name):
        """
        Creates a basf2 path for the correct collector and serializes it in the self.output_dir/<calibration_name>/paths directory
        """
        path_output_dir = os.path.join(self.output_dir, calibration_name, 'paths')
        # Should work fine as we previously make the other directories
        os.mkdir(path_output_dir)
        # Create empty path and add collector to it
        path = create_path()
        calibration = self.calibrations[calibration_name]
        path.add_module(calibration.collector)
        # Dump the basf2 path to file and repeat
        path_file_name = calibration.collector.name()+'.pickle'
        path_file_name = os.path.join(path_output_dir, path_file_name)
        with open(path_file_name, 'bw') as serialized_path_file:
            pickle.dump(serialize_path(path), serialized_path_file)
        # Return the pickle file path for addition to the input sandbox
        return path_file_name


class Job:
    """
    Generic Job object used to tell a Backend what to do.
    - This is a way to store necessary information about a process for
    submission and pass it in one object to a backend, rather than having
    the framework set each parameter directly.
    - Hopefully means that ANY use case can be more easily supported,
    not just the CAF. You just have to fill a job object and pass it to a
    Backend for the job submission to work.
    - Use absolute paths for all directories, otherwise you'll likely get into trouble
    """

    def __init__(self, name):
        """
        Init method of Job object.
        - Here you just set the job name, everything else comes later.
        """
        #: Job object's name. Only descriptive, not necessarily unique.
        self.name = name
        #: Files to be tarballed and sent along with the job (NOT the input root files)
        self.input_sandbox_files = []
        #: Working directory of the job (str). Default is '.', mostly used in Local() backend
        self.working_dir = '.'
        #: Output directory (str), where we will download our output_files to. Default is '.'
        self.output_dir = '.'
        #: Files that we produce during the job and want to be returned. Can use wildcard (*)
        self.output_files = []
        #: Command and arguments as a list that wil be run by the job on the backend
        self.cmd = []
