#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This module implements several objects/functions to configure and run calibrations.
These classes are used to construct the workflow of the calibration job.
The actual processing code is mostly in the `caf.state_machines` module.
"""

__all__ = ["CalibrationBase", "Calibration", "Algorithm", "CAF"]

import os
import sys
from threading import Thread
from time import sleep
from pathlib import Path
import sqlite3
from contextlib import closing

from basf2 import B2ERROR, B2WARNING, B2INFO, B2FATAL, B2DEBUG
from basf2 import get_default_global_tags

from abc import ABC, abstractmethod
import ROOT

from .utils import B2INFO_MULTILINE
from .utils import past_from_future_dependencies
from .utils import topological_sort
from .utils import all_dependencies
from .utils import decode_json_string
from .utils import method_dispatch
from .utils import find_sources
from .utils import AlgResult
from .utils import temporary_workdir
from .utils import IoV
from .utils import IoV_Result

from caf import strategies
from caf import runners
import caf.backends
from .state_machines import CalibrationMachine, MachineError, ConditionError, TransitionError
from caf.database import CAFDB


class CalibrationBase(ABC, Thread):
    """
    Abstract base class of Calibration types. The CAF implements the :py:class:`Calibration` class which inherits from
    this and runs the C++ CalibrationCollectorModule and CalibrationAlgorithm classes. But by inheriting from this
    class and providing the minimal necessary methods/attributes you could plug in your own Calibration types
    that doesn't depend on the C++ CAF at all and run everything in your own way.

    .. warning:: Writing your own class inheriting from :py:class:`CalibrationBase` class is not recommended!
                             But it's there if you really need it.

    Parameters:
        name (str): Name of this calibration object. Should be unique if you are going to run it.

    Keyword Arguments:
        input_files (list[str]): Input files for this calibration. May contain wildcard expressions useable by `glob.glob`.
    """
    #: The name of the successful completion state.
    #: The :py:class:`CAF` will use this as the state to decide when the Calibration is completed.
    end_state = "completed"
    #: The name of the failure state.
    #: The :py:class:`CAF` will use this as the state to decide when the Calibration failed.
    fail_state = "failed"

    def __init__(self, name, input_files=None):
        """
        """
        super().__init__()
        #: Name of calibration object. This must be unique when adding into the py:class:`CAF`.
        self.name = name
        #: List of calibration objects that depend on this one.
        self.future_dependencies = []
        #: List of calibration objects, where each one is a dependency of this one.
        self.dependencies = []
        #: File -> Iov dictionary, should be
        #:
        #: >>> {absolute_file_path:iov}
        #:
        #: Where iov is a :py:class:`IoV <caf.utils.IoV>` object. Will be filled during `CAF.run()` if empty.
        #: To improve performance you can fill this yourself before calling `CAF.run()`
        self.files_to_iovs = {}
        if input_files:
            #: Files used for collection procedure
            self.input_files = input_files
        else:
            self.input_files = []

        #: IoV which will be calibrated. This is set by the `CAF` itself when calling `CAF.run()`
        self.iov = None
        #: The directory where we'll store the local database payloads from this calibration
        self.output_database_dir = ""

    @abstractmethod
    def run(self):
        """The most important method. Runs inside a new Thread and is called from `CalibrationBase.start`
        once the dependencies of this `CalibrationBase` have returned with state == end_state i.e. "completed"
        """
        pass

    @abstractmethod
    def is_valid(self):
        """A simple method you should implement that will return True or False depending on whether
        the Calibration has been set up correctly and can be run safely."""
        pass

    def depends_on(self, calibration):
        """
        Parameters:
            calibration (`CalibrationBase`): The Calibration object which will produce constants that this one depends on.

        Adds dependency of this calibration on another i.e. This calibration
        will not run until the dependency has completed, and the constants produced
        will be used via the database chain.

        You can define multiple dependencies for a single calibration simply
        by calling this multiple times. Be careful when adding the calibration into
        the `CAF` not to add a circular/cyclic dependency. If you do the sort will return an
        empty order and the `CAF` processing  will fail.

        This function appens to the `CalibrationBase.dependencies` and `CalibrationBase.future_dependencies` attributes of this
        `CalibrationBase` and the input one respectively. This prevents us having to do too much recalculation later on.
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

    def dependencies_met(self):
        """
        Checks if all of the Calibrations that this one depends on have reached a successful end state
        """
        return all(map(lambda x: x.state == x.end_state, self.dependencies))

    def failed_dependencies(self):
        """
        Returns the list of calibrations in our dependency list that have failed.
        """
        failed = []
        for calibration in self.dependencies:
            if calibration.state == self.fail_state:
                failed.append(calibration)
        return failed

    def _apply_calibration_defaults(self, defaults):
        """
        We pass in default calibration options from the `CAF` instance here if called.
        Won't overwrite any options already set.
        """
        for key, value in defaults.items():
            try:
                if not getattr(self, key):
                    setattr(self, key, value)
            except AttributeError:
                print("The calibration", self.name, "does not support the attribute", key)


class Calibration(CalibrationBase):
    """
    Every Calibration object must have a collector and at least one algorithm.
    You have the option to add in your collector/algorithm by argument here, or
    later by changing the properties.

    Parameters:
        name (str): Name of this calibration. It should be unique for use in the `CAF`
    Keyword Arguments:
        collector: Should be set to a CalibrationCollectorModule() or a string with the module name.
        algorithms: Should be set to a CalibrationAlgorithm() or a `list` of them.
        input_files (str or list[str]): Input files for use by this Calibration. May contain wildcards useable by `glob.glob`

    A Calibration won't be valid in the `CAF` until it has all of these four attributes set. For example:

    >>> cal = Calibration('TestCalibration1')
    >>> col1 = register_module('CaTest')
    >>> cal.collector = col1

    or equivalently

    >>> cal.collector = 'CaTest'

    If you want to run a basf2 :py:class:`path <basf2.Path>` before your collector module when running over data

    >>> cal.pre_collector_path = my_basf2_path

    You don't have to put a RootInput module in this pre-collection path, but you can if
    you need some special parameters.
    The inputFileNames parameter will be set by the `CAF` automatically for you.

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

    If you want a different setup for each algorithm use a list with the same number of elements
    as your algorithm list.

    >>> cal.pre_algorithms = [my_function1, my_function2, ...]

    You can use optional arguments to pass in some/all during initialisation of the `Calibration` class

    >>> cal = Calibration( 'TestCalibration1', 'CaTest', [alg1,alg2], ['/path/to/file.root'])

    you can change the input file list later on, before running with `CAF`

    >>> cal.input_files = ['path/to/*.root', 'other/path/to/file2.root']

    You can also specify the dependencies of the calibration on others

    >>> cal.depends_on(cal2)

    By doing this, the `CAF` will respect the ordering of the calibrations and will pass the
    calibration constants created by earlier completed calibrations to dependent ones.
    """
    #: Allowed transitions that we will use to progress
    moves = ["submit_collector", "complete", "run_algorithms", "iterate", "fail_fully"]
    #: Subdirectory name for algorithm output
    alg_output_dir = "algorithm_output"
    #: Checkpoint states which we are allowed to restart from.
    checkpoint_states = ["init", "collector_completed", "completed"]

    def __init__(self,
                 name,
                 collector=None,
                 algorithms=None,
                 input_files=None):
        """
        """
        super().__init__(name, input_files)
        #: Internal calibration collector/algorithms/input_files stored for this calibration
        self._collector = None
        #: Internal calibration algorithms stored for this calibration
        self._algorithms = []
        #: Internal input_files stored for this calibration
        self._input_files = []

        if collector:
            #: Collector property, you can assign either a CollectorModule directly, or a string with the CollectorModule
            #: name. It will run `basf2.register_module` for you and assign it to this property.
            self.collector = collector
        if algorithms:
            #: `Algorithm` classes that wil be run by this `Calibration`. You should set this attribute to either a single
            #: CalibrationAlgorithm C++ class, or a `list` of them if you want to run multiple CalibrationAlgorithms using one
            #: CalibrationCollectorModule.
            self.algorithms = algorithms
        #: Since many collectors require some different setup, if you set this attribute to a `basf2.Path` it will be run before
        #: the collector and after the default RootInput module + HistoManager setup.
        #: If this path contains RootInput then it's params are used in the RootInput module instead, except for the input_files
        #: parameter.
        self.pre_collector_path = None
        #: Output results of algorithms for each iteration
        self.results = {}
        #: Output patterns of files produced by collector which will be passed to the `Algorithm.data_input` function.
        #: You can set these to anythuing understood by `glob.glob`, but if you want to specify this you should also specify
        #: the `Algorithm.data_input` function to handle the different types of files and call the
        #: CalibrationAlgorithm.setInputFiles() with hte correct ones.
        self.output_patterns = ['CollectorOutput.root']
        #: Variable to define the maximum number of iterations for this calibration specifically.
        #: It overrides tha CAF calibration_defaults value if set.
        self.max_iterations = None
        #: Maximum number of input files per subjob during the collector step (passed to a `caf.backends.Job` object).
        #: -1 is the default meaning that all input files are run in one big collector job.
        self.max_files_per_collector_job = -1
        #: Dictionary passed to the collector Job object to configure how the `caf.backends.Backend` instance should treat
        #: the collector job. Currently only useful for setting the 'queue' of the batch system backends that the collector
        #: jobs are submitted to e.g. cal.backend_args = {"queue":"short"}
        self.backend_args = {}
        if self.algorithms:
            #: The strategy that the algorithm(s) will be run against. Assign a list of strategies the same length as the number of
            #: algorithms, or assign a single strategy to apply it to all algorithms in this `Calibration`. You can see the choices
            #: in :py:mod:`caf.strategies`.
            self.strategies = strategies.SingleIOV

        self._local_database_chain = []
        self.use_central_database(get_default_global_tags())
        #: The class that runs all the algorithms in this Calibration using their assigned
        #: :py:class:`caf.strategies.AlgorithmStrategy`.
        #: Plugin your own runner class to change how your calibration will run the list of algorithms.
        self.algorithms_runner = runners.SeqAlgorithmsRunner
        #: The `backend <backends.Backend>` we'll use for our collector submission in this calibration.
        #: It will be set by the CAF if not here
        self.backend = None
        #: While checking if the collector is finished we don't bother wastefully checking every subjob's status.
        #: We exit once we find the first subjob that isn't ready.
        #: But after this interval has elapsed we do a full :py:meth:`caf.backends.Job.update_status` call and
        #: print the fraction of SubJobs completed.
        self.collector_full_update_interval = 300
        self.setup_defaults()
        #: The `caf.state_machines.CalibrationMachine` that we will run to process this calibration start to finish.
        self.machine = None
        #: Location of a SQLite database that will save the state of the calibration so that it can be restarted from failure.
        self._db_path = None

    def is_valid(self):
        """A full calibration consists of a collector AND an associated algorithm AND input_files.
        This returns False if any are missing or if the collector and algorithm are mismatched.

        We also check that the strategies of the algorithms match the collector granularity.
        """
        if (not self.collector or not self.algorithms or not self.input_files):
            B2WARNING("Empty collector, algorithm, or input_files for {}.".format(self.name))
            return False
        collector_params = self.collector.available_params()
        for param in collector_params:
            if param.name == "granularity":
                granularity = param.values
        for alg in self.algorithms:
            alg_type = type(alg.algorithm).__name__
            if self.collector.name() != alg.algorithm.getCollectorName():
                B2WARNING(("Algorithm '%s' requested collector '%s' but got '%s'"
                           % (alg_type, alg.algorithm.getCollectorName(), self.collector.name())))
                return False
            if granularity not in alg.strategy.allowed_granularities:
                B2WARNING("Selected strategy for {} does not allow collector "
                          "granularity to be '{}'.".format(alg_type,
                                                           granularity))
                return False
        return True

    def use_central_database(self, global_tag):
        """
        Parameters:
            global_tag (str): The central database global tag to use for this calibration.

        Using this allows you to set the central database for this calibration.
        If you don't call this, the default is set from `basf2.get_default_global_tags`.
        If this is set manually it will override the default.
        To turn off central database completely you should set this global tag to an empty string.
        """
        self._global_tag = global_tag

    def use_local_database(self, filename, directory=""):
        """
        Parameters:
            filename (str): The path to the database.txt of the local database

        Keyword Argumemts:
            directory (str): The path to the payloads directory for this local database.

        Append a local database to the chain for this calibration.
        You can call this function multiple times and each database will be added to the chain IN ORDER.
        The databases are applied to this calibration ONLY.
        They are applied to the collector job and algorithm step as a database chain of the form:

        central DB Global tag (if used) -> these local databases -> CAF created local database constants
        """
        self._local_database_chain.append((filename, directory))

    @property
    def collector(self):
        """
        """
        return self._collector

    @collector.setter
    def collector(self, collector):
        """
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
        """
        return self._algorithms

    @algorithms.setter
    @method_dispatch
    def algorithms(self, value):
        """
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
    def pre_algorithms(self):
        """
        Callback run prior to each algorithm iteration.
        """
        return [alg.pre_algorithm for alg in self.algorithms]

    @pre_algorithms.setter
    @method_dispatch
    def pre_algorithms(self, func):
        """
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
        The `caf.strategies.AlgorithmStrategy` or `list` of them used when running the algorithm(s).
        """
        return [alg.strategy for alg in self.algorithms]

    @strategies.setter
    @method_dispatch
    def strategies(self, strategy):
        """
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

    def run(self):
        """
        Main logic of the Calibration object.
        Will be run in a new Thread by calling the start() method.
        """
        with CAFDB(self._db_path) as db:
            initial_state = db.get_calibration_value(self.name, "checkpoint")
            initial_iteration = db.get_calibration_value(self.name, "iteration")
        B2INFO("Initial status of {} found to be state={}, iteration={}".format(self.name,
                                                                                initial_state,
                                                                                initial_iteration))
        self.machine = CalibrationMachine(self,
                                          iov_to_calibrate=self.iov,
                                          initial_state=initial_state,
                                          iteration=initial_iteration)
        self.state = initial_state
        self.machine.root_dir = Path(os.getcwd(), self.name)
        self.machine.collector_backend = self.backend
        while self.state != self.end_state and self.state != self.fail_state:
            if self.state == "init":
                try:
                    B2INFO("Attempting collector submission for calibration {}.".format(self.name))
                    self.machine.submit_collector()
                except Exception as err:
                    B2FATAL(str(err))

                self._poll_collector()

            # If we failed take us to the final fail state
            if self.state == "collector_failed":
                self.machine.fail_fully()
                return

            # It's possible that we might raise an error while attempting to run due
            # to some problems e.g. Missing collector output files
            # We catch the error and exit with failed state so the CAF will stop
            try:
                self.machine.run_algorithms()
            except MachineError as err:
                B2ERROR(str(err))
                self.machine.fail()

            # If we failed take us to the final fail state
            if self.machine.state == "algorithms_failed":
                self.machine.fail_fully()
                return

    def _poll_collector(self):
        """
        """
        while self.state == "running_collector":
            try:
                B2INFO("Checking if collector jobs for calibration {} have finished successfully.".format(self.name))
                self.machine.complete()
            # ConditionError is thrown when the condtions for the transition have returned false, it's not serious.
            except ConditionError:
                try:
                    B2DEBUG(29, "Checking if collector jobs for calibration {} have failed.".format(self.name))
                    self.machine.fail()
                except ConditionError:
                    pass
            sleep(self.heartbeat)  # Sleep until we want to check again

    def setup_defaults(self):
        """
        Anything that is setup by outside config files by default goes here.
        """
        import configparser
        config_file_path = ROOT.Belle2.FileSystem.findFile('calibration/data/caf.cfg')
        if config_file_path:
            config = configparser.ConfigParser()
            config.read(config_file_path)
        else:
            B2FATAL("Tried to find the default CAF config file but it wasn't there. Is basf2 set up?")
        #: This calibration's sleep time before rechecking to see if it can move state
        self.heartbeat = decode_json_string(config['CAF_DEFAULTS']['Heartbeat'])

    @property
    def state(self):
        """
        The current major state of the calibration in the database file. The machine may have a different state.
        """
        with CAFDB(self._db_path) as db:
            return db.get_calibration_value(self.name, "state")

    @state.setter
    def state(self, state):
        """
        """
        B2DEBUG(29, "Setting {} to state {}".format(self.name, str(state)))
        with CAFDB(self._db_path) as db:
            db.update_calibration_value(self.name, "state", str(state))
            if state in self.checkpoint_states:
                db.update_calibration_value(self.name, "checkpoint", str(state))
        B2DEBUG(29, "{} set to {}".format(self.name, self.state))

    @property
    def iteration(self):
        """
        Retrieves the current iteration number in the database file

        Returns:
            int: The current iteration number
        """
        with CAFDB(self._db_path) as db:
            return db.get_calibration_value(self.name, "iteration")

    @iteration.setter
    def iteration(self, iteration):
        """
        """
        B2DEBUG(29, "Setting {} to {}".format(self.name, iteration))
        with CAFDB(self._db_path) as db:
            db.update_calibration_value(self.name, "iteration", iteration)
        B2DEBUG(29, "{} set to {}".format(self.name, self.iteration))


class Algorithm():
    """
    Parameters:
        algorithm: The CalibrationAlgorithm instance that we want to execute.
    Keyword Arguments:
        data_input (types.FunctionType): An optional function that sets the input files of the algorithm.
        pre_algorithm (types.FunctionType): An optional function that runs just prior to execution of the algorithm.
            Useful for set up e.g. module initialisation

    This is a simple wrapper class around the C++ CalibrationAlgorithm class.
    It helps to add functionality to algorithms for use by the Calibration and CAF classes rather
    than separating the logic into those classes directly.

    This is **not** currently a class that a user should interact with much during `CAF`
    setup (unless you're doing something advanced).
    The `Calibration` class should be doing the most of the creation of the defaults for these objects.

    Setting the `data_input` function might be necessary if you have set the `Calibration.output_patterns`.
    Also, setting the `pre_algorithm` to a function that should execute prior to each `strategies.AlgorithmStrategy`
    is often useful i.e. by calling for the Geometry module to initialise.
    """

    def __init__(self, algorithm, data_input=None, pre_algorithm=None):
        """
        """
        #: CalibrationAlgorithm instance (assumed to be true since the Calibration class checks)
        self.algorithm = algorithm
        #: The name of the algorithm, default is the Algorithm class name
        self.name = algorithm.__cppname__.replace('Belle2::', '')
        #: Function called before the pre_algorithm method to setup the input data that the CalibrationAlgorithm uses.
        #: The list of files matching the `Calibration.output_patterns` from the collector output
        #: directories will be passed to it
        self.data_input = data_input
        if not self.data_input:
            self.data_input = self.default_inputdata_setup
        #: Function called after data_input but before algorithm.execute to do any remaining setup.
        #: It *must* have the form ``pre_algorithm(algorithm, iteration)``  where algorithm can be
        #: assumed to be the CalibrationAlgorithm instance about to be executed, and iteration is an int e.g. 0, 1, 2...
        self.pre_algorithm = pre_algorithm
        #: The algorithm stratgey that will be used when running over the collected data.
        #: you can set this here, or from the `Calibration.strategies` attribute.
        self.strategy = strategies.SingleIOV
        #: Parameters that could be used in the execution of the algorithm strategy/runner to modify behaviour.
        #: By default this is empty and not used by the default :py:class:`caf.strategies.SingleIOV` class.
        #: But more complex strategies, or your own custom ones, could use it to configure behaviour.
        #: Note that if you modify this inside a subprocess the modification won't persist outside, you would have to change
        #: it in the parent process (or dump values to a file and read it in next time).
        self.params = {}

    def default_inputdata_setup(self, input_file_paths):
        """
        Simple setup to set the input file names to the algorithm. Applied to the data_input attribute
        by default. This simply takes all files returned from the `Calibration.output_patterns` and filters
        for only the CollectorOutput.root files. Then it sets them as input files to the CalibrationAlgorithm class.
        """
        collector_output_files = list(filter(lambda file_path: "CollectorOutput.root" == Path(file_path).name,
                                             input_file_paths))
        info_lines = ["Input files used in {}:".format(self.name)]
        info_lines.extend(collector_output_files)
        B2INFO_MULTILINE(info_lines)
        self.algorithm.setInputFileNames(collector_output_files)


class CAF():
    """
    Parameters:
      calibration_defaults (dict): A dictionary of default options for calibrations run by this `CAF` instance e.g.

                                   >>> calibration_defaults={"max_iterations":2}

    This class holds `Calibration` objects and processes them. It defines the initial configuration/setup
    for the calibrations. But most of the real processing is done through the `caf.state_machines.CalibrationMachine`.

    The `CAF` class essentially does some initial setup, holds the `CalibrationBase` instances and calls the
    `CalibrationBase.start` when the dependencies are met.

    Much of the checking for consistency is done in this class so that no processing is done with an invalid
    setup. Choosing which files to use as input should be done from outside during the setup of the `CAF` and
    `CalibrationBase` instances.
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

        #: Dictionary of calibrations for this `CAF` instance. You should use `add_calibration` to add to this.
        self.calibrations = {}
        #: Dictionary of future dependencies of `Calibration` objects, where the value is all
        #: calibrations that will depend on the key, filled during self.run()
        self.future_dependencies = {}
        #: Dictionary of dependencies of `Calibration` objects, where value is the list of `Calibration` objects
        #: that the key depends on. This attribute is filled during self.run()
        self.dependencies = {}
        #: Output path to store results of calibration and bookkeeping information
        self.output_dir = self.config['CAF_DEFAULTS']['ResultsDir']
        #: The ordering and explicit future dependencies of calibrations. Will be filled during `CAF.run()` for you.
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
        #: The name of the SQLite DB that gets created
        self._db_name = "caf_state.db"
        #: The path of the SQLite DB
        self._db_path = None

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
        Keyword Arguments:
            iov(`caf.utils.IoV`): IoV to calibrate for this processing run. Only the input files necessary to calibrate
                                  this IoV will be used in the collection step.

        This function runs the overall calibration job, saves the outputs to the output_dir directory,
        and creates database payloads.

        Upload of final databases is not done here. This simply creates the local databases in
        the output directory. You should check the validity of your new local database before uploading
        to the conditions DB via the basf2 tools/interface to the DB.
        """
        if not self.calibrations:
            B2FATAL("There were no Calibration objects to run. Maybe you tried to add invalid ones?")
        # Checks whether the dependencies we've added will give a valid order
        order = self._order_calibrations()
        if not order:
            B2FATAL("Couldn't order the calibrations properly. Could be a cyclic dependency.")

        # Check that a backend has been set and use default Local() one if not
        self._check_backend()

        # Creates the overall output directory and reset the attribute to use an absolute path to it.
        self.output_dir = self._make_output_dir()

        # The sleep heartbeat when checking calibration statuses
        caf_heartbeat = int(self.config["CAF_DEFAULTS"]["HeartBeat"])

        #  Creates a SQLite DB to save the status of the various calibrations
        self._make_database()

        # Enter the overall output dir during processing and opena  connection to the DB
        with temporary_workdir(self.output_dir), CAFDB(self._db_path) as db:
            db_initial_calibrations = db.query("select * from calibrations").fetchall()
            for calibration in self.calibrations.values():
                # Apply defaults given to the `CAF` to the calibrations if they aren't set
                calibration._apply_calibration_defaults(self.calibration_defaults)
                calibration._db_path = self._db_path
                calibration.iov = iov
                if not calibration.backend:
                    calibration.backend = self.backend
                # Do some checking of the db to see if we need to add an entry for this calibration
                if calibration.name not in [db_cal[0] for db_cal in db_initial_calibrations]:
                    db.insert_calibration(calibration.name)
                    db.commit()
                else:
                    for cal_info in db_initial_calibrations:
                        if cal_info[0] == calibration.name:
                            cal_initial_state = cal_info[2]
                            cal_initial_iteration = cal_info[3]
                    B2INFO("Previous entry in database found for {}".format(calibration.name))
                    B2INFO("Setting {} state to checkpoint state '{}'".format(calibration.name, cal_initial_state))
                    calibration.state = cal_initial_state
                    B2INFO("Setting {} iteration to '{}'".format(calibration.name, cal_initial_iteration))
                    calibration.iteration = cal_initial_iteration
                # Daemonize so that it exits if the main program exits
                calibration.daemon = True

            # Is it possible to keep going?
            keep_running = True
            while keep_running:
                keep_running = False
                # Do we have calibrations that may yet complete?
                remaining_calibrations = []

                for calibration in self.calibrations.values():
                    # Find the currently ended calibrations (may not be joined yet)
                    if (calibration.state == CalibrationBase.end_state or calibration.state == CalibrationBase.fail_state):
                        # Search for any alive Calibrations and join them
                        if calibration.is_alive():
                            B2DEBUG(29, "joining {}".format(calibration.name))
                            calibration.join()
                    else:
                        if calibration.dependencies_met():
                            if not calibration.is_alive():
                                B2DEBUG(29, "Starting {}".format(calibration.name))
                                calibration.start()
                            remaining_calibrations.append(calibration)
                        else:
                            if not calibration.failed_dependencies():
                                remaining_calibrations.append(calibration)
                if remaining_calibrations:
                    keep_running = True
                sleep(caf_heartbeat)

            B2INFO("Printing summary of final CAF status.")
            print(db.output_calibration_table())

        # Close down our processing pools nicely
        if isinstance(self.backend, caf.backends.Local):
            self.backend.join()
        else:
            self._algorithm_backend.join()

    @property
    def backend(self):
        """
        The `backend <backends.Backend>` that runs the collector job.
        Whe set, this is checked that a `backends.Backend` class instance was passed in.
        """
        return self._backend

    @backend.setter
    def backend(self, backend):
        """
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
            B2INFO('{0} output directory already exists. '
                   'We will try to restart from the previous finishing state.'.format(self.output_dir))
            abs_output_dir = os.path.join(os.getcwd(), self.output_dir)
            return abs_output_dir
        else:
            os.mkdir(self.output_dir)
            abs_output_dir = os.path.join(os.getcwd(), self.output_dir)
            if os.path.exists(abs_output_dir):
                return abs_output_dir
            else:
                B2ERROR("Attempted to create output_dir {0}, but it didn't work.".format(abs_output_dir))
                sys.exit(1)

    def _make_database(self):
        """
        Creates the CAF status database. If it already exists we don't overwrite it.
        """
        self._db_path = Path(self.output_dir, self._db_name).absolute()
        if self._db_path.exists():
            B2INFO('Previous CAF database found {}'.format(self._db_path))
        # Will create a new database + tables, or do nothing but checks we can connect to existing one
        with CAFDB(self._db_path) as db:
            pass
