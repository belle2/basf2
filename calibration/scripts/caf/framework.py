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
import sys
import shutil
from pprint import PrettyPrinter
from datetime import datetime
from collections import OrderedDict
from collections import deque
import pickle
from time import sleep
import multiprocessing
import glob

from .utils import topological_sort
from .utils import all_dependencies
from .utils import decode_json_string
from .utils import method_dispatch
from .utils import find_sources
import caf.utils
import caf.backends

import ROOT
from ROOT.Belle2 import PyStoreObj, CalibrationAlgorithm

collector_steering_file_path = os.environ.get('BELLE2_LOCAL_DIR')
collector_steering_file_path = os.path.join(collector_steering_file_path, 'calibration/scripts/caf/run_collector_path.py')

pp = PrettyPrinter()


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
        You can specify the collector/algorithms/input files later on or here but it won't be valid
        until those are set.
        """

        #: Name of calibration object
        self.name = name
        #: Internal calibration collector/algorithms/input_files stored for this calibration
        self._collector = None
        #: Internal calibration algorithms stored for this calibration
        self._algorithms = []
        #: Internal input_files stored for this calibration
        self._input_files = []

        if collector:
            #: Publicly accessible collector
            self.collector = collector
        if algorithms:
            #: Publicly accessible algorithms
            self.algorithms = algorithms
        if input_files:
            #: Files used for collection procedure
            self.input_files = input_files
        #: Since many algorithms require some different setup, this is a function object to run prior to alg.execute()
        self.pre_algorithm = None

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
            B2ERROR("Tried to set Calibration name to a non-string type")

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
                B2ERROR("Collector needs to be either a Module or the name of such a module")
        #: Internal storage of collector attribute
        self._collector = collector

    @property
    def algorithms(self):
        """
        Getter for the algorithms property.
        """
        return self._algorithms

    @algorithms.setter
    @method_dispatch
    def algorithms(self, value):
        """
        Setter for the algorithms property, checks if single or list of algorithms passed in.
        """
        if isinstance(value, CalibrationAlgorithm):
            self._algorithms = [value]
        else:
            B2ERROR(("Something other than CalibrationAlgorithm instance passed in ({0})."
                     "Algorithm needs to inherit from Belle2::CalibrationAlgorithm".format(type(value))))

    @algorithms.fset.register(tuple)
    @algorithms.fset.register(list)
    def _(self, value):
        """
        Alternate algorithms setter for lists and tuples of CalibrationAlgorithms
        """
        if value:
            self._algorithms = []
            for alg in value:
                if isinstance(alg, CalibrationAlgorithm):
                    self._algorithms.append(alg)
                else:
                    B2ERROR(("Something other than CalibrationAlgorithm instance passed in {0}."
                             "Algorithm needs to inherit from Belle2::CalibrationAlgorithm".format(type(value))))

    @property
    def input_files(self):
        """
        Getter for the input_files attribute.
        """
        return self._input_files

    @input_files.setter
    @method_dispatch
    def input_files(self, file):
        """
        Setter for the input_files attribute. Checks that a string/list of strings was passed in.
        And then builds a list from that.
        """
        if isinstance(file, str):
            self._input_files = [file]
        else:
            B2ERROR("Something other than string passed in as an input file.")

    @input_files.fset.register(tuple)
    @input_files.fset.register(list)
    def _(self, value):
        """
        Alternate input_files setter for lists and tuples of strings
        """
        if value:
            self._input_files = []
            for file in value:
                if isinstance(file, str):
                    self._input_files.append(file)
                else:
                    B2ERROR("Something other than string passed in as an input file.")

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
        No default backend is set! This is to prevent unnecessary process pools
        being allocated. You should set it directly beofre calling caf.run()
        e.g. caf.backend = Local()

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
            B2ERROR("Tried to use $BELLE2_LOCAL_DIR but it wasn't there. Is basf2 set up?")

        #: Dictionary of calibrations for this CAF instance
        self.calibrations = {}
        #: OrderedDictionary of dependenciesof calibration objects
        self.dependencies = OrderedDict()
        #: Output path to store results of calibration and bookkeeping information
        self.output_dir = self.config['CAF_DEFAULTS']['ResultsDir']
        #: Polling frequencywhile waiting for jobs to finish
        self.heartbeat = decode_json_string(self.config['CAF_DEFAULTS']['HeartBeat'])

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
        # Gives us a list of A (not THE) valid ordering and checks for cyclic dependencies
        order = topological_sort(self.dependencies)
        if order:
            # We want to put the most critical (most overall dependencies) near the start
            # First get an ordered dictionary of the sort order but including the dependencies.
            # This time the dependencies will be in the key=calibration, value=[all dependent calibrations]
            # format.
            ordered_full_dependencies = all_dependencies(self.dependencies, order)
            # Need to implement an ordering algorithm here, based on number of future dependents?
            order = ordered_full_dependencies

        return ordered_full_dependencies

    def configure_jobs(self, calibrations):
        """
        Creates a Job object for each collector to pass to the Backend.
        """
        jobs = {}
        for calibration_name in calibrations:
            job = Job('_'.join([calibration_name, 'Collector']))
            self._make_calibration_dir(calibration_name)
            collector_path_file = self._make_collector_path(calibration_name)
            job.output_dir = os.path.join(self.output_dir, calibration_name, 'output')
            job.working_dir = os.path.join(self.output_dir, calibration_name, 'input')
            job.cmd = ['basf2', 'run_collector_path.py']
            job.input_sandbox_files.append(collector_steering_file_path)
            job.input_sandbox_files.append(collector_path_file)
            job.input_files = self.calibrations[calibration_name].input_files
            job.output_files = ['*.mille', 'RootOutput.root']
            jobs[calibration_name] = job

        return jobs

    def run(self):
        """
        - Runs the overall calibration job, saves the outputs to the output_dir directory,
        and creates database payloads.
        - Upload may be moved to another method or function entirely to give the option of
        monitoring output before committing to database.
        """
        # Creates the ordering of calibrations, including any dependencies that we must wait for
        # before continuing.
        order = self.order_calibrations()
        if not order:
            B2ERROR("Couldn't order the calibrations properly. Probably a cyclic dependency.")

        # Creates list of first set of calibrations to submit
        col_to_submit = list(find_sources(order))
        # remove the submitting calibrations from the overall order
        for calibration_name in col_to_submit:
            order.pop(calibration_name)

        # Creates the overall output directory and reset the attribute to use an absolute path to it.
        self.output_dir = self._make_output_dir()

        # Main running loop, continues until we're completely finished
        while col_to_submit:
            # Create job objects for collectors
            col_jobs = self.configure_jobs(col_to_submit)
            # Submit collection jobs that have no dependencies
            results = self.backend.submit(list(col_jobs.values()))
            # Event loop waiting for results
            while True:
                # We sleep initially since we only just submitted the collector jobs
                sleep(self.heartbeat)
                # loop to check which results are finished and remove from col_to_submit
                # Iterates over a copy of col_to_submit as we're editing it.
                for calibration_name, result in zip(col_to_submit[:], results):
                    ready = result.ready()
                    B2DEBUG(100, '{0} is ready: {1}'.format(calibration_name, ready))
                    if ready:
                        col_to_submit.remove(calibration_name)

                # Once the collectors are all done, run the algorithms for them
                if not col_to_submit:
                    # We pass in the col_jobs dictionary as it contains the calibration names
                    # and job objects (output directories) that we need.
                    self._run_algorithms(col_jobs)

                    # Find new sources if they exist
                    col_to_submit = list(find_sources(order))
                    # Remove new sources from order
                    for calibration_name in col_to_submit:
                        order.pop(calibration_name)
                    # Go round and submit next calibrations
                    break

        # Close down our processing pool nicely
        if isinstance(self.backend, caf.backends.Local):
            self.backend.join()

    def _run_algorithms(self, jobs):
        """
        Runs the Calibration Algorithms for all of the calibration names
        in the list argument.

        Will run them sequentially locally (possible benefits to using a
        processing pool for low memory algorithms later on.)
        """
        B2INFO("""Running the Calibration Algorithms:\n{0}""".format([calibration_name for calibration_name in jobs.keys()]))

        # prepare a multiprocessing context which uses fork
        ctx = multiprocessing.get_context("fork")

        for calibration_name, job in jobs.items():
            calibration = self.calibrations[calibration_name]
            algorithms = calibration.algorithms
            for algorithm in algorithms:
                child = ctx.Process(target=self._run_algorithm, args=(calibration, algorithm, job.output_dir))
                child.start()
                # wait for it to finish
                child.join()
                print("Exit code was {0}".format(child.exitcode))

    def _run_algorithm(self, calibration, algorithm, working_dir):
        """
        Runs a single algorithm of a calibration in the output directory of the collector
        and first runs a setup function passed in.
        """
        logging.reset()
        # Now that we're in a subprocess we can change working directory without affecting
        # the main process.
        os.chdir(working_dir)
        # Get a nicer version of the algorithm name
        algorithm_name = algorithm.Class_Name().replace('Belle2::', '')
        # Create a directory to store the output of this algorithm
        os.mkdir(algorithm_name)

        # add logfile for output
        logging.add_file(algorithm_name+'/log')

        # Fall back to previous databases if no payloads are found
        use_database_chain(True)

        #
        #
        # HERE IS WHERE WE SHOULD ADD PREVIOUS ALGORITHM DATABASE PAYLOADS
        #
        #

        # add local database to save payloads
        use_local_database(algorithm_name+"/database.txt", algorithm_name, False, LogLevel.INFO)

        B2INFO("Running {0} in working directory {1}".format(algorithm_name, working_dir))
        B2INFO("Output folder contents of collector was"+str(glob.glob('./*')))
        if calibration.pre_algorithm:
            calibration.pre_algorithm()

        # Sorting the run list should not be necessary as the CalibrationAlgorithm does this during
        # the getRunListFromAllData() function.
        # Get a vector of all the experiments and runs passed into the algorithm via the output of the collector
        exprun_vector = algorithm.getRunListFromAllData()
#        for exprun in exprun_vector:
#            print(exprun.first, exprun.second)
        # Create empty IoV vector to execute
        iov_to_execute = ROOT.vector("std::pair<int,int>")()
        for exprun in exprun_vector:
            # Add each exprun to the vector in turn
            print("Adding Exp:Run = {0}:{1} to execution request".format(exprun.first, exprun.second))
            iov_to_execute.push_back(exprun)
            # Perform the algorithm over the requested IoVs
            alg_result = algorithm.execute(iov_to_execute, 1)  # Need to pass in the iteration
            # Commit to the local database if we've got a success or iteration requested
            if alg_result == CalibrationAlgorithm.c_OK or alg_result == CalibrationAlgorithm.c_Iterate:
                print("Finished with highest Exp:Run = {0}:{1} ".format(exprun.first, exprun.second))
                algorithm.commit()
                iov_to_execute.clear()
        logging.reset()
        return 0

#    def beginRun(self):
#        """Collect all runs we have seen"""
#        event = PyStoreObj("EventMetaData").obj()
#        self.runs.add((event.getExperiment(), event.getRun()))
#
#    def execute(self, runs):
#        """Execute the algorithm over list of runs"""
#        # create std::vector<ExpRun> for the argument
#        iov_vec = ROOT.vector("std::pair<int,int>")()
#        pair = ROOT.pair("int", "int")()
#        for run in runs:
#            pair.first, pair.second = run
#            iov_vec.push_back(pair)
#        # run the algorithm
#        result = self.algorithm.execute(iov_vec, 1)  # Use 1 iteration as default for now
#        return result
#
#    def terminate(self):
#        """Run the calibration algorithm at the end of the process"""
#        runs = []  # Start with no runs
#        for run in sorted(self.runs):
#            runs.append(run)  # Add in some seen runs and execute over them
#            result = self.execute(runs)
#            # if anything else then NotEnoughData is returned then we
#            # empty the list of runs for the next call.
#            if result != CalibrationAlgorithm.c_NotEnoughData:
#                runs = []
#

#        if isinstance(self.backend, caf.backends.Local):
#            pool = backends.pool
#
#        else:
#            pool mp.Pool(max_processes=1)
#

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
            B2ERROR('{0} output directory already exists.'.format(self.output_dir))
        else:
            os.mkdir(self.output_dir)
            abs_output_dir = os.path.join(os.getcwd(), self.output_dir)
            if os.path.exists(abs_output_dir):
                return abs_output_dir
            else:
                B2ERROR("Attempted to create output_dir {0}, but it didn't work.".format(abs_output_dir))

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
        #: Input root files to basf2 job
        self.input_files = []

    def __repr__(self):
        """
        Representation of Job class (what happens when you print a Job() instance)
        """
        return self.name
