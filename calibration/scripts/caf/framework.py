#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Calibration Framework - David Dossett 2016

This module implements several objects/functions to configure and run the calibration.
These can then be used to construct the workflow of the calibration job.
"""

from basf2 import *
import os
import sys
import shutil
from pprint import PrettyPrinter
from datetime import datetime
from collections import OrderedDict
from collections import deque
from collections import namedtuple
import pickle
from time import sleep
import multiprocessing
import glob

from .utils import topological_sort
from .utils import all_dependencies
from .utils import decode_json_string
from .utils import method_dispatch
from .utils import find_sources
from .utils import algorithm_result_names
from .utils import temporary_workdir
from .utils import iov_from_vector

import caf.utils
import caf.backends

import ROOT
from ROOT.Belle2 import PyStoreObj, CalibrationAlgorithm

_collector_steering_file_path = ROOT.Belle2.FileSystem.findFile('calibration/scripts/caf/run_collector_path.py')

pp = PrettyPrinter()
IoV_Result = namedtuple('IoV_Result', ['iov', 'result'])


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

    You can also specify the dependencies of the calibration
    >>> cal.depends_on(cal2)

    The CAF should automatically register these dependencies and prune out any problems.
    """

    def __init__(self, name, collector=None, algorithms=None, input_files=None):
        """
        You have to specify a unique name for the calibration when creating it.
        You can specify the collector/algorithms/input files later on or here but it won't be valid
        until those are set.
        """

        #: Name of calibration object, THIS MUST BE UNIQUE WHEN ADDING INTO CAF!
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
        #: Since many collectors require some different setup, this is a path added before the collector and after the
        #: default RootInput module setup. If this path contains RootInput then it's params are used instead, except for
        #: the input_files.
        self.pre_collector_path = None
        #: Since many algorithms require some different setup, this is a function object to run prior to alg.execute()
        self.pre_algorithm = None
        #: Output results of algorithms for each iteration
        self.results = {}
        #: Output patterns of files produced by collector and which need to be saved in the output directory
        self.output_patterns = ['RootOutput.root']
        #: List of calibration objects that will depend on this one.
        self.future_dependencies = []
        #: OrderedDictionary of dependencies of calibration objects, where value is the calibrations
        #: that the key depends on.
        self.dependencies = []

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

    def depends_on(self, calibration):
        """Adds dependency of this calibration on another i.e. This calibration
        will not run until the dependency has completed, and the constants produced
        will be used via the database chain.

        You can define multiple dependencies for a single calibration simply
        by calling this multiple times. Be careful when adding the calibration into
        the CAF not to add a circular/cyclic dependency. If you do the sort will return an
        empty order and the CAF job will fail.

        This function appens to the dependencies and future_dependencies attributes of this
        calibration and the input one respectively. This prevents us having to do too much
        recalculation later on.
        """
        # Check that we don't have two calibration names that are the same
        if self.name != calibration.name:
            # Tests if we have the calibrations added as dependencies already and adds if not
            if calibration not in self.dependencies:
                self.dependencies.append(calibration)
            if self not in calibration.dependencies:
                calibration.future_dependencies.append(self)
        else:
            B2WARNING(("Tried to add {0} as a dependency for {1} but they have the same name."
                       "Dependency was not added.".format(calibration, self)))

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
        config_file_path = ROOT.Belle2.FileSystem.findFile('calibration/data/caf.cfg')
        if config_file_path:
            import configparser
            #: Configuration object for CAF, can change the defaults through a single config file
            self.config = configparser.ConfigParser()
            self.config.read(config_file_path)
        else:
            B2ERROR("Tried to find the default CAF config file but it wasn't there. Is basf2 set up?")

        #: Dictionary of calibrations for this CAF instance
        self.calibrations = {}
        #: Dictionary of future dependencies of calibration objects, where the value is all
        #: calibrations that will depend on the key, filled during self.run()
        self.future_dependencies = {}
        #: Dictionary of dependencies of calibration objects, where value is the calibrations
        #: that the key depends on, filled during self.run()
        self.dependencies = {}
        #: Output path to store results of calibration and bookkeeping information
        self.output_dir = self.config['CAF_DEFAULTS']['ResultsDir']
        #: Polling frequencywhile waiting for jobs to finish
        self.heartbeat = decode_json_string(self.config['CAF_DEFAULTS']['HeartBeat'])
        #: Maximum number of iterations the CAF will make of all necessary calibrations
        self.max_iterations = decode_json_string(self.config['CAF_DEFAULTS']['MaxIterations'])
        #: The ordering and explicit future dependencies of calibrations. Will be filled during self.run()
        self.order = None

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
            else:
                B2WARNING('Tried to add a calibration with the name '+calibration.name+' twice.')
        else:
            B2WARNING(("Tried to add incomplete/invalid calibration ({0}) to the framwork."
                       "It was not added and will not be part of the final process.".format(calibration.name)))

    def _remove_missing_dependencies(self):
        """
        This checks the future and past dependencies of each calibration in the CAF.
        If any dependencies are not known to the CAF then they are removed from the calibration
        object directly.
        """
        calibration_names = [calibration.name for calibration in self.calibrations.values()]

        def is_dependency_in_caf(dependency):
            """
            Quick function to use with filter() and check dependencies against calibrations known to CAF
            """
            dependency_in_caf = dependency.name in calibration_names
            if not dependency_in_caf:
                B2WARNING(("The calibration {0} is a required dependency but is not in the CAF."
                           "It has been removed as a dependency.").format(dependency.name))
            return dependency_in_caf

        # Check that there aren't dependencies on calibrations not added to the framework
        # Remove them from the calibration objects if there are.
        for calibration in self.calibrations.values():
            filtered_future_dependencies = list(filter(is_dependency_in_caf, calibration.future_dependencies))
            calibration.future_dependencies = filtered_future_dependencies

            filtered_dependencies = list(filter(is_dependency_in_caf, calibration.dependencies))
            calibration.dependencies = filtered_dependencies

    def _order_calibrations(self):
        """
        - Uses dependency atrributes of calibrations to create a dependency dictionary and passes it
        to a sorting algorithm.
        - Returns True if sort was succesful, False if it failed (most likely a cyclic dependency)
        """
        # First remove any dependencies on calibrations not added to the CAF
        self._remove_missing_dependencies()
        # Filling dependencies dictionaries of CAF for sorting, only explicit dependencies for now
        # Note that they currently use the names not the calibration objects.
        for calibration in self.calibrations.values():
            future_dependencies_names = [dependency.name for dependency in calibration.future_dependencies]
            past_dependencies_names = [dependency.name for dependency in calibration.dependencies]

            self.future_dependencies[calibration.name] = future_dependencies_names
            self.dependencies[calibration.name] = past_dependencies_names
        # Gives us a list of A (not THE) valid ordering and checks for cyclic dependencies
        order = topological_sort(self.future_dependencies)
        if order:
            # We want to put the most critical (most overall dependencies) near the start
            # First get an ordered dictionary of the sort order but including all implicit dependencies.
            ordered_full_dependencies = all_dependencies(self.future_dependencies, order)
            ####################################
            #
            # TODO: Need to implement an ordering algorithm here, based on number of future dependents?
            #
            ####################################
            order = ordered_full_dependencies
        return order

    def _configure_jobs(self, calibrations, iteration):
        """
        Creates a Job object for each collector to pass to the Backend.
        """
        jobs = {}
        for calibration_name in calibrations:
            job = Job('_'.join([calibration_name, 'Collector', 'Iteration', str(iteration)]))
            job.output_dir = os.path.join(os.getcwd(), calibration_name, str(iteration), 'output')
            job.working_dir = os.path.join(os.getcwd(), calibration_name, str(iteration), 'input')
            job.cmd = ['basf2', 'run_collector_path.py']
            job.input_sandbox_files.append(_collector_steering_file_path)
            collector_path_file = self._make_collector_path(calibration_name, iteration)
            job.input_sandbox_files.append(collector_path_file)
            if self.calibrations[calibration_name].pre_collector_path:
                pre_collector_path_file = self._make_pre_collector_path(calibration_name, iteration)
                job.input_sandbox_files.append(pre_collector_path_file)
            ###########################################
            #
            # TODO: Add dependent calibration databases
            #
            ###########################################
            # Add previous iteration databases from this calibration
            if iteration > 0:
                for algorithm in self.calibrations[calibration_name].algorithms:
                    algorithm_name = algorithm.Class_Name().replace('Belle2::', '')
                    database_dir = os.path.join(os.getcwd(), calibration_name, str(iteration-1), 'output', 'localdb')
                    job.input_sandbox_files.append(database_dir)
            # Define the input file list and output patterns to be returned from collector job
            job.input_files = self.calibrations[calibration_name].input_files
            job.output_patterns = self.calibrations[calibration_name].output_patterns
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
        self.order = self._order_calibrations()
        if not self.order:
            B2ERROR("Couldn't order the calibrations properly. Probably a cyclic dependency.")

        # Create a shallow copy of self.order that we can remove items from
        order = self.order.copy()

        # Creates list of first set of calibrations to submit
        col_to_submit = list(find_sources(order))
        # remove the submitting calibrations from the overall order
        for calibration_name in col_to_submit:
            order.pop(calibration_name)

        # Creates the overall output directory and reset the attribute to use an absolute path to it.
        self.output_dir = self._make_output_dir()
        # Enter the overall output dir
        with temporary_workdir(self.output_dir):
            # Make the output directory for each calibration
            for calibration_name in self.calibrations.keys():
                os.mkdir(calibration_name)

            # Main running loop, continues until we're completely finished or we fail
            while col_to_submit:
                for iteration in range(self.max_iterations):
                    iteration_needed = False
                    # Create job objects for collectors
                    col_jobs = self._configure_jobs(col_to_submit, iteration)
                    # Submit collection jobs that have no dependencies
                    results = self.backend.submit(list(col_jobs.values()))
                    # Event loop waiting for results
                    waiting_on_results = col_to_submit[:]
                    while waiting_on_results:
                        # We sleep initially since we only just submitted the collector jobs
                        sleep(self.heartbeat)
                        # loop to check which results are finished and remove from waiting list
                        # Iterates over a copy as we're editing it.
                        for calibration_name, result in zip(waiting_on_results[:], results):
                            ready = result.ready()
                            B2DEBUG(100, '{0} is ready: {1}'.format(calibration_name, ready))
                            if ready:
                                waiting_on_results.remove(calibration_name)

                    # Once the collectors are all done, run the algorithms for them
                    # We pass in the col_jobs dictionary as it contains the calibration names
                    # and job objects (output directories) that we need.
                    self._run_algorithms(col_jobs, iteration)

                    for calibration_name in col_to_submit:
                        iteration_needed = False
                        results = self.calibrations[calibration_name].results[iteration]
                        for algorithm_name, iov_results in results.items():
                            for iov_result in iov_results:
                                if CalibrationAlgorithm.c_Iterate == iov_result.result:
                                    iteration_needed = True
                                    print('Iteration called for by {0} on IoV {1}'.format(algorithm_name, iov_result.iov))
                        if not iteration_needed:
                            database_location = os.path.join(self.output_dir, calibration_name, str(iteration), 'output', 'localdb')
                            final_database_location = os.path.join(self.output_dir, calibration_name, 'localdb')
                            shutil.copytree(database_location, final_database_location)
                            col_to_submit.remove(calibration_name)

                    # Once all the algorithms have returned c_OK we can move on to the next collectors
                    if not iteration_needed:
                        # Find new sources if they exist
                        col_to_submit = list(find_sources(order))
                        # Remove new sources from order
                        for calibration_name in col_to_submit:
                            order.pop(calibration_name)
                        # Go round and submit next calibrations
                        break

                else:
                    if iteration_needed:
                        print('Max iterations reached but algorithms still requesting more!')
                        break

        print('Results of all calibrations')
        for calibration in self.calibrations.values():
            pp.pprint(calibration.results)

        # Close down our processing pool nicely if we used one
        if isinstance(self.backend, caf.backends.Local):
            self.backend.join()

    def _run_algorithms(self, jobs, iteration):
        """
        Runs the Calibration Algorithms for all of the calibration names
        in the list argument.

        Will run them sequentially locally (possible benefits to using a
        processing pool for low memory algorithms later on.)
        """
        B2INFO("""Running the Calibration Algorithms:\n{0}""".format([calibration_name for calibration_name in jobs.keys()]))

        # prepare a multiprocessing context which uses fork
        ctx = multiprocessing.get_context("fork")
        # prepare a Pipe to receive the results of the algorithms from inside the process
        parent_conn, child_conn = ctx.Pipe()

        for calibration_name, job in jobs.items():
            calibration = self.calibrations[calibration_name]
            algorithms = calibration.algorithms
            calibration.results[iteration] = {}

            for algorithm in algorithms:
                child = ctx.Process(target=self._run_algorithm,
                                    args=(calibration, algorithm, job.output_dir, iteration, child_conn))
                child.start()
                # wait for it to finish
                child.join()
                # Get a nicer version of the algorithm name
                algorithm_name = algorithm.Class_Name().replace('Belle2::', '')
                # Get the return codes of the algorithm for the IoVs found by the Process
                calibration.results[iteration][algorithm_name] = parent_conn.recv()
                print("Exit code was {0}".format(child.exitcode))

    def _run_algorithm(self, calibration, algorithm, working_dir, iteration, child_conn):
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

        # Create a directory to store the payloads of this algorithm
        os.mkdir('localdb')

        # add logfile for output
        logging.add_file(algorithm_name+'_b2log')

        reset_database()
        # Fall back to previous databases if no payloads are found
        use_database_chain(True)

        # Here we add the finished databases of previous calibrations that we depend on.
        # We can assume that the databases exist as we can't be here until they have returned
        # with OK status.
        for calibration_name in self.order:
            if calibration_name in self.dependencies[calibration.name]:
                database_location = os.path.join(self.output_dir, calibration_name, 'localdb')
                use_local_database(os.path.join(database_location, 'database.txt'), database_location, True, LogLevel.INFO)
                print('Adding local database from {0} for use by {1}'.format(calibration_name, algorithm_name))

        # Here we add the previous iteration's database
        if iteration > 0:
            use_local_database(os.path.join('../../', str(iteration-1), 'output/localdb/database.txt'),
                               os.path.join('../../', str(iteration-1), 'output/localdb'), True, LogLevel.INFO)

        # add local database to save payloads
        use_local_database("localdb/database.txt", 'localdb', False, LogLevel.INFO)

        B2INFO("Running {0} in working directory {1}".format(algorithm_name, working_dir))
        B2INFO("Output folder contents of collector was"+str(glob.glob('./*')))
        if calibration.pre_algorithm:
            calibration.pre_algorithm(iteration)

        # Sorting the run list should not be necessary as the CalibrationAlgorithm does this during
        # the getRunListFromAllData() function.
        # Get a vector of all the experiments and runs passed into the algorithm via the output of the collector
        exprun_vector = algorithm.getRunListFromAllData()
        # Create empty IoV vector to execute
        iov_to_execute = ROOT.vector("std::pair<int,int>")()
        # Create list of result codes and IoVs
        results = []
        # Want to store the payloads so that we only commit them once we've got a new set to hold onto.
        # Because if we'll be merging later and should wait
        last_payloads = None
        for exprun in exprun_vector:
            # Add each exprun to the vector in turn
            print("Adding Exp:Run = {0}:{1} to execution request".format(exprun.first, exprun.second))
            iov_to_execute.push_back(exprun)
            # Perform the algorithm over the requested IoVs
            alg_result = algorithm.execute(iov_to_execute, iteration)
            # Commit to the local database if we've got a success or iteration requested
            if alg_result == CalibrationAlgorithm.c_OK or alg_result == CalibrationAlgorithm.c_Iterate:
                print("Finished execution of {0} with highest Exp:Run = {1}:{2} ".format(algorithm_name,
                      exprun.first, exprun.second))
                if last_payloads:
                    algorithm.commit(last_payloads)
                last_payloads = algorithm.getPayloads()
                # Get readable iov for this one and create a result entry. We can always pop the entry if we'll be merging.
                # Can't do that with the database as easily.
                iov = iov_from_vector(iov_to_execute)
                result = IoV_Result(iov, alg_result)
                print('Result was {0}'.format(result))
                results.append(result)
                iov_to_execute.clear()
            print("Execution of {0} finished with exit code {1} = {2}".format(algorithm_name, alg_result,
                  algorithm_result_names[alg_result]))

        else:
            # Final IoV will probably have returned c_NotEnoughData so we need to merge it with the previous successful
            # IoV if one exists.
            if iov_to_execute.size() and results:
                iov = iov_from_vector(iov_to_execute)
                print('Merging IoV for {0} onto end of previous IoV'.format(iov))
                last_successful_result = results.pop(-1)
                iov_to_execute.clear()
                exprun = ROOT.pair('int, int')
                for exp in range(last_successful_result.iov[0][0], iov[1][0]+1):
                    for run in range(last_successful_result.iov[1][0], iov[1][1]+1):
                        iov_to_execute.push_back(exprun(exp, run))

                alg_result = algorithm.execute(iov_to_execute, iteration)
                algorithm.commit()
                # Get readable iov for this one and create a result entry.
                iov = iov_from_vector(iov_to_execute)
                result = IoV_Result(iov, alg_result)
                print('Result was {0}'.format(result))
                results.append(result)

        logging.reset()
        child_conn.send(results)
        return 0

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

    def _make_collector_path(self, calibration_name, iteration):
        """
        Creates a basf2 path for the correct collector and serializes it in the
        self.output_dir/<calibration_name>/<iteration>/paths directory
        """
        path_output_dir = os.path.join(os.getcwd(), calibration_name, str(iteration), 'paths')
        # Should work fine as we previously make the other directories
        os.makedirs(path_output_dir)
        # Create empty path and add collector to it
        path = create_path()
        calibration = self.calibrations[calibration_name]
        path.add_module(calibration.collector)
        # Dump the basf2 path to file
        path_file_name = calibration.collector.name()+'.path'
        path_file_name = os.path.join(path_output_dir, path_file_name)
        with open(path_file_name, 'bw') as serialized_path_file:
            pickle.dump(serialize_path(path), serialized_path_file)
        # Return the pickle file path for addition to the input sandbox
        return path_file_name

    def _make_pre_collector_path(self, calibration_name, iteration):
        """
        Creates a basf2 path for the collectors setup path (Calibration.pre_collector_path) and serializes it in the
        self.output_dir/<calibration_name>/<iteration>/paths directory
        """
        path_output_dir = os.path.join(os.getcwd(), calibration_name, str(iteration), 'paths')
        path = self.calibrations[calibration_name].pre_collector_path
        # Dump the basf2 path to file
        path_file_name = 'pre_collector.path'
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
        self.output_patterns = []
        #: Command and arguments as a list that wil be run by the job on the backend
        self.cmd = []
        #: Input root files to basf2 job
        self.input_files = []

    def __repr__(self):
        """
        Representation of Job class (what happens when you print a Job() instance)
        """
        return self.name
