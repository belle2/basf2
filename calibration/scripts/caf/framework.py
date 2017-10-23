#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This module implements several objects/functions to configure and run calibrations.
These classes are used to construct the workflow of the calibration job.
The actual processing code is mostly in the `caf.state_machines` module.
"""

__all__ = ["Calibration", "Algorithm", "CAF"]

import os
import sys
from threading import Thread

from basf2 import B2ERROR, B2WARNING, B2INFO, B2FATAL

import ROOT

from caf.utils import past_from_future_dependencies
from caf.utils import topological_sort
from caf.utils import all_dependencies
from caf.utils import decode_json_string
from caf.utils import method_dispatch
from caf.utils import find_sources
from caf.utils import AlgResult
from caf.utils import temporary_workdir
from caf.utils import IoV
from caf.utils import IoV_Result

from caf import strategies
from caf import runners
import caf.backends
from caf.state_machines import CalibrationMachine, MachineError, ConditionError, TransitionError


class Calibration():
    """
    Every Calibration object must have a collector and at least one algorithm.
    You have the option to add in your collector/algorithm by argument here, or
    later by changing the properties.

    :param name: A string to name this calibration. It should be unique for use in the `CAF`
    :param collector: Should be set to a CalibrationCollectorModule or a string with the module name.
    :param algorithms: Should be set to a CalibrationAlgorithm or a list of them.
    :param input_files: A string/list of strings. May contain wildcards useable by glob

    A Calibration won't be valid in the `CAF` until it has all of these four attributes set.

    Example:

    >>> cal = Calibration('TestCalibration1')
    >>> col1 = register_module('CaTest')
    >>> cal.collector = col1

    or equivalently

    >>> cal.collector = 'CaTest'

    If you want to run a basf2 path before your collector module when running over data

    >>> cal.pre_collector_path = my_basf2_path

    You don't have to put a RootInput module in this pre-collection path, but you can if
    you need some special parameters. The inputFileNames parameter will be set by the `CAF` directly.

    Adding the CalibrationAlgorithm(s) is easy

    >>> alg1 = TestAlgo()
    >>> cal.algorithms = alg1

    Or equivalently

    >>> cal.algorithms = [alg1]

    Or for multiple algorithms for one collector

    >>> alg2 = TestAlgo()
    >>> cal.algorithms = [alg1, alg2]

    Note that when you set the algorithms, they are automatically wrapped and stored as
    `Algorithm` instances. To access the algorithm underneath directly do:

    >>> cal.algorithms[i].algorithm

    If you have a setup function that you want to run before each of the algorithms, set that with

    >>> cal.pre_algorithms = my_function_object

    OR, if you want a different setup for each algorithm use a list with the same number of elements
    as your algorithm list.

    >>> cal.pre_algorithms = [my_function1, my_function2, ...]

    Can use optional arguments to pass in some/all during initialisation

    >>> cal = Calibration( 'TestCalibration1', 'CaTest', [alg1,alg2], ['/path/to/file.root'])

    Change the input file list later on, before running with `CAF`

    >>> cal.input_files = ['path/to/*.root', 'other/path/to/file2.root']

    You can also specify the dependencies of the calibration on others

    >>> cal.depends_on(cal2)

    By doing this, the `CAF` will respect the ordering of the dependencies and pass the
    calibration constants created by earlier calibrations to dependent ones.
    """

    def __init__(self, name, collector=None, algorithms=None, input_files=None):
        """
        """
        #: Name of calibration object. This must be unique when adding into the `CAF`.
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
        #: default RootInput module + HistoManager setup. If this path contains RootInput then it's params are used instead,
        #: except for the input_files.
        self.pre_collector_path = None
        #: Output results of algorithms for each iteration
        self.results = {}
        #: Output patterns of files produced by collector and which need to be saved in the output directory
        self.output_patterns = ['CollectorOutput.root']
        #: List of calibration objects that will depend on this one.
        self.future_dependencies = []
        #: OrderedDictionary of dependencies of calibration objects, where value is the calibrations
        #: that the key depends on.
        self.dependencies = []
        #: File -> Iov dictionary, should be
        #:
        #: >>> {absolute_file_path:iov}
        #:
        #: Where iov is a `caf.utils.IoV` object.
        self.files_to_iovs = dict()
        #: Variable to define the maximum number of iterations for this calibration specifically.
        #: It overrides tha CAF calibration_defaults value if set.
        self.max_iterations = None
        #: Maximum number of input files per subjob during the collector step (passed to a `caf.backends.Job` object).
        #: -1 is the default meaning that all input files are run in one big collector job.
        self.max_files_per_collector_job = -1
        #: Dictionary passed to the collector Job object to configure how the `caf.backends.Backend` instance should treat
        #: the collector job. Generally only useful for setting the 'queue' of the batch system backends that the collector
        #: jobs are submitted to e.g. cal.backend_args = {"queue":"short"}
        self.backend_args = {}

        #: The strategy that the algorithm(s) will be run against. Assign a list of strategies the same length as the number of
        #: algorithms, assign a single strategy to apply it to all algorithms in this `Calibration`. You can sse the choices
        #: in `strategies`.
        self.strategies = strategies.SingleIOV()

        #: The `runners.Runner` type to run this overall `Calibration`. You should assign an object inheriting from
        #: `runners.CalibrationRunner`
        self.runner = runners.CalibrationRunner()

        self._local_database_chain = []

        import ROOT
        config_file_path = ROOT.Belle2.FileSystem.findFile('calibration/data/caf.cfg')
        if config_file_path:
            import configparser
            config = configparser.ConfigParser()
            config.read(config_file_path)
            self.use_central_database(config["CAF_DEFAULTS"]["GlobalTag"])
        else:
            B2FATAL("Tried to find the default CAF config file but it wasn't there. Is basf2 set up?")

    def _apply_calibration_defaults(self, defaults):
        """
        We pass in default calibration options from the `CAF` instance here if called.
        Won't overwrite any options already set.
        """
        for key, value in defaults.items():
            if not getattr(self, key):
                setattr(self, key, value)

    def is_valid(self):
        """A full calibration consists of a collector AND an associated algorithm AND input_files.
        This returns False if any are missing or if the collector and algorithm are mismatched."""
        if (not self.collector or not self.algorithms or not self.input_files):
            return False
        else:
            for alg in self.algorithms:
                alg_type = type(alg.algorithm).__name__
                if self.collector.name() != alg.algorithm.getCollectorName():
                    B2WARNING(("Algorithm '%s' requested collector '%s' but got '%s'"
                               % (alg_type, alg.algorithm.getCollectorName(), self.collector.name())))
                    return False
            else:
                return True

    def use_central_database(self, global_tag):
        """
        Central database global tag to use for this calibration. Default is set from data/caf.cfg
        If this is set manually it will override the default. To turn off central database usage you
        should set this global tag to an empty string.
        """
        self._global_tag = global_tag

    def use_local_database(self, filename, directory=""):
        """
        Append a local database to the chain for this calibration. You can call this function multiple times and
        each database will be added to the chain IN ORDER. The databases are applied to this calibration ONLY.

        They are applied to the collector job and algorithm step as a database chain of the form:

        central DB Global tag (if used) -> these local databases -> CAF created local database constants
        """
        self._local_database_chain.append((filename, directory))

    def depends_on(self, calibration):
        """Adds dependency of this calibration on another i.e. This calibration
        will not run until the dependency has completed, and the constants produced
        will be used via the database chain.

        You can define multiple dependencies for a single calibration simply
        by calling this multiple times. Be careful when adding the calibration into
        the `CAF` not to add a circular/cyclic dependency. If you do the sort will return an
        empty order and the `CAF` processing  will fail.

        This function appens to the `Calibration.dependencies` and `Calibration.future_dependencies` attributes of this
        `Calibration` and the input one respectively. This prevents us having to do too much recalculation later on.
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
            from basf2 import Module
            if isinstance(collector, str):
                from basf2 import register_module
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
        from ROOT.Belle2 import CalibrationAlgorithm
        if isinstance(value, CalibrationAlgorithm):
            self._algorithms = [Algorithm(value)]
        else:
            B2ERROR(("Something other than CalibrationAlgorithm instance passed in ({0})."
                     "Algorithm needs to inherit from Belle2::CalibrationAlgorithm".format(type(value))))

    @algorithms.fset.register(tuple)
    @algorithms.fset.register(list)
    def _(self, value):
        """
        Alternate algorithms setter for lists and tuples of CalibrationAlgorithms
        """
        from ROOT.Belle2 import CalibrationAlgorithm
        if value:
            self._algorithms = []
            for alg in value:
                if isinstance(alg, CalibrationAlgorithm):
                    self._algorithms.append(Algorithm(alg))
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

    @property
    def pre_algorithms(self):
        """
        Getter for the pre_algorithm attribute. Notice how we avoid the user needing to interact with the
        `Algorithm` wrapper class.
        """
        return [alg.pre_algorithm for alg in self.algorithms]

    @pre_algorithms.setter
    @method_dispatch
    def pre_algorithms(self, func):
        """
        Setter for the pre_algorithms attribute.
        """
        if func:
            for alg in self.algorithms:
                alg.pre_algorithm = func
        else:
            B2ERROR("Something evaluated as False passed in as pre_algorithm function.")

    @pre_algorithms.fset.register(tuple)
    @pre_algorithms.fset.register(list)
    def _(self, values):
        """
        Alternate pre_algorithms setter for lists and tuples of functions, should be one per algorithm.
        """
        if values:
            if len(values) == len(self.algorithms):
                for func, alg in zip(values, self.algorithms):
                    alg.pre_algorithm = func
            else:
                B2ERROR("Number of functions and number of algorithms doesn't match.")
        else:
            B2ERROR("Empty container passed in for pre_algorithm functions")

    @property
    def strategies(self):
        """
        Getter for the strategies attribute.
        """
        return [alg.strategy for alg in self.algorithms]

    @strategies.setter
    @method_dispatch
    def strategies(self, strategy):
        """
        Setter for the strategies attribute.
        """
        if strategy:
            for alg in self.algorithms:
                alg.strategy = strategy
        else:
            B2ERROR("Something evaluated as False passed in as a strategy.")

    @strategies.fset.register(tuple)
    @strategies.fset.register(list)
    def _(self, values):
        """
        Alternate strategies setter for lists and tuples of functions, should be one per algorithm.
        """
        if values:
            if len(values) == len(self.algorithms):
                for strategy, alg in zip(strategies, self.algorithms):
                    alg.strategy = strategy
            else:
                B2ERROR("Number of strategies and number of algorithms doesn't match.")
        else:
            B2ERROR("Empty container passed in for strategies list")

    def __repr__(self):
        """
        """
        return self.name


class Algorithm():
    """
    Simple wrapper class around the C++ CalibrationAlgorithm class. Helps to add
    functionality to algorithms for use by the Calibration and CAF classes rather
    than separating the logic into those classes directly.

    This is NOT currently a class that a user should interact with much during `CAF`
    setup. The `Calibration` class should be doing the creation of the defaults for
    these objects.
    """

    def __init__(self, algorithm, data_input=None, pre_algorithm=None):
        """
        """
        #: CalibrationAlgorithm instance (assumed to be true since the Calibration class checks)
        self.algorithm = algorithm
        #: Function called before the pre_algorithm method to setup the input data that the CalibrationAlgorithm uses.
        #: The list of input files from the collector output will be passed to it
        self.data_input = data_input
        if not self.data_input:
            self.data_input = self.default_rootinput_setup
        #: Function called after data_input but before algorithm.execute to do any remaining setup
        #: IT MUST ONLY HAVE TWO ARGUMENTS pre_algorithm(algorithm, iteration)  where algorithm can be
        #: assumed to be the CalibrationAlgorithm instance, and iteration is an int e.g. 0, 1, 2...
        self.pre_algorithm = pre_algorithm
        #: The algorithm stratgey that will be used when running over the collected data
        self.strategy = strategies.SingleIOV()

    def default_rootinput_setup(self, input_file_paths):
        """
        Simple setup to set the input file names to the algorithm. Applied to the data_input attribute
        by default.
        """
        self.algorithm.setInputFileNames(input_file_paths)


class CAF():
    """
    :param calibration_defaults: A dictionary ofdefault options for calibrations run by this `CAF` instance
        e.g. calibration_defaults={"max_iterations":2}. Each calibration can override these options by setting
        them for itself

    This class holds calibration objects and processes them. It defines the initial configuration/setup
    for the calibrations. But most of the real logic is done through the `caf.state_machines.CalibrationMachine`

    The `CAF` class holds the calibration dependencies and sets up the `caf.state_machines.CalibrationRunner` instances.
    Much of the checking for consistency is done in this class so that no processing is done with an invalid
    setup. Choosing which files to use as input should be done from outside during the setup of the `CAF` and
    `Calibration` instances.
    """

    def __init__(self, calibration_defaults=None):
        """
        """
        import ROOT
        config_file_path = ROOT.Belle2.FileSystem.findFile('calibration/data/caf.cfg')
        if config_file_path:
            import configparser
            #: Configuration object for `CAF`, can change the defaults through a single config file
            #: or can directly alter this config object if you want to.
            self.config = configparser.ConfigParser()
            self.config.read(config_file_path)
        else:
            B2FATAL("Tried to find the default CAF config file but it wasn't there. Is basf2 set up?")

        #: Dictionary of calibrations for this `CAF` instance
        self.calibrations = {}
        #: Dictionary of future dependencies of `Calibration` objects, where the value is all
        #: calibrations that will depend on the key, filled during self.run()
        self.future_dependencies = {}
        #: Dictionary of dependencies of `Calibration` objects, where value is the list of `Calibration` objects
        #: that the key depends on. This attribute is filled during self.run()
        self.dependencies = {}
        #: Output path to store results of calibration and bookkeeping information
        self.output_dir = self.config['CAF_DEFAULTS']['ResultsDir']
        #: Polling frequencywhile waiting for jobs to finish
        self.heartbeat = decode_json_string(self.config['CAF_DEFAULTS']['HeartBeat'])
        #: The ordering and explicit future dependencies of calibrations. Will be filled during `CAF.run`
        self.order = None
        #: Private backend attribute
        self._backend = None

        if not calibration_defaults:
            calibration_defaults = {}
        if "max_iterations" not in calibration_defaults:
            calibration_defaults["max_iterations"] = decode_json_string(self.config["CAF_DEFAULTS"]["MaxIterations"])
        #: Default options applied to each calibration known to the `CAF`, if the `Calibration` has these defined by the user
        #: then the defaults aren't applied. A simple way to define the same configuration to all calibrations in the `CAF`.
        self.calibration_defaults = calibration_defaults

    def add_calibration(self, calibration):
        """
        Adds a `Calibration` that is to be used in this program to the list.
        Also adds an empty dependency list to the overall dictionary.
        You should not directly alter a `Calibration` object after it has been
        added here.
        """
        if calibration.is_valid():
            if calibration.name not in self.calibrations:
                self.calibrations[calibration.name] = calibration
            else:
                B2WARNING('Tried to add a calibration with the name ' + calibration.name + ' twice.')
        else:
            B2WARNING(("Tried to add incomplete/invalid calibration ({0}) to the framwork."
                       "It was not added and will not be part of the final process.".format(calibration.name)))

    def _remove_missing_dependencies(self):
        """
        This checks the future and past dependencies of each `Calibration` in the `CAF`.
        If any dependencies are not known to the `CAF` then they are removed from the `Calibration`
        object directly.
        """
        calibration_names = [calibration.name for calibration in self.calibrations.values()]

        def is_dependency_in_caf(dependency):
            """
            Quick function to use with filter() and check dependencies against calibrations known to `CAF`
            """
            dependency_in_caf = dependency.name in calibration_names
            if not dependency_in_caf:
                B2WARNING(("The calibration {0} is a required dependency but is not in the CAF."
                           " It has been removed as a dependency.").format(dependency.name))
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
        - Returns valid OrderedDict if sort was succesful, empty one if it failed (most likely a cyclic dependency)
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
        if not order:
            return False

        # Get an ordered dictionary of the sort order but including all implicit dependencies.
        ordered_full_dependencies = all_dependencies(self.future_dependencies, order)
        # Return all the implicit+explicit past dependencies
        full_past_dependencies = past_from_future_dependencies(ordered_full_dependencies)
        # Correct each calibration's dependency list to reflect the implicit dependencies
        for calibration in self.calibrations.values():
            full_deps = full_past_dependencies[calibration.name]
            explicit_deps = [cal.name for cal in calibration.dependencies]
            for dep in full_deps:
                if dep not in explicit_deps:
                    calibration.dependencies.append(self.calibrations[dep])
        order = ordered_full_dependencies
        # We should also patch in all of the implicit dependencies for the calibrations
        return order

    def _check_backend(self):
        """
        Makes sure that the CAF has a valid backend setup. If one isn't set by the user (or if the
        one that is stored isn't a valid Backend object) we should create a default Local backend.
        """
        if not isinstance(self._backend, caf.backends.Backend):
            #: backend property
            self.backend = caf.backends.Local()
        if isinstance(self._backend, caf.backends.Local):
            self._algorithm_backend = self.backend
        else:
            self._algorithm_backend = caf.backends.Local()

    def run(self, iov=None):
        """
        :param iov: The `caf.utils.IoV` to calibrate for this processing run. Only the input files necessary to calibrate
            this IoV will be used in the collection step. An the algorithm will only run over this IoV.

        This function runs the overall calibration job, saves the outputs to the output_dir directory,
        and creates database payloads.

        Upload of final databases is not done here. This simply creates the local databases in
        the output directory. You should check the validity of your new local database before uploading
        to the conditions DB via the basf2 tools/interface to the DB.
        """
        # Checks whether the dependencies we've added will give a valid order
        order = self._order_calibrations()
        if not order:
            B2FATAL("Couldn't order the calibrations properly. Probably a cyclic dependency.")

        # Check that a backend has been set and use default Local() one if not
        self._check_backend()

        # Creates the overall output directory and reset the attribute to use an absolute path to it.
        self.output_dir = self._make_output_dir()
        # Enter the overall output dir during processing
        with temporary_workdir(self.output_dir):
            runners = []
            # Create Runners to spawn threads for each calibration
            for calibration_name, calibration in self.calibrations.items():
                calibration._apply_calibration_defaults(self.calibration_defaults)
                machine = CalibrationMachine(calibration, iov)
                machine.collector_backend = self.backend
                calibration.runner.machine = machine
                calibration.runner.heartbeat = self.heartbeat
                runners.append(calibration.runner)

            for runner in runners:
                runner.start()

            for runner in runners:
                runner.join()

        # Close down our processing pools nicely
        if isinstance(self.backend, caf.backends.Local):
            self.backend.join()
        else:
            self._algorithm_backend.join()

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
        Creates the output directory. If it already exists we quit the program to prevent horrible
        problems by either overwriting the files in this directory or moving it to a new name.
        It returns the absolute path of the new output_dir
        """
        if os.path.isdir(self.output_dir):
            B2ERROR('{0} output directory already exists.'.format(self.output_dir))
            sys.exit(1)
        else:
            os.mkdir(self.output_dir)
            abs_output_dir = os.path.join(os.getcwd(), self.output_dir)
            if os.path.exists(abs_output_dir):
                return abs_output_dir
            else:
                B2ERROR("Attempted to create output_dir {0}, but it didn't work.".format(abs_output_dir))
                sys.exit(1)
