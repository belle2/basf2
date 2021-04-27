#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This module implements several objects/functions to configure and run calibrations.
These classes are used to construct the workflow of the calibration job.
The actual processing code is mostly in the `caf.state_machines` module.
"""

__all__ = ["CalibrationBase", "Calibration", "Algorithm", "CAF"]

import os
from threading import Thread
from time import sleep
from pathlib import Path
import shutil
from glob import glob

from basf2 import B2ERROR, B2WARNING, B2INFO, B2FATAL, B2DEBUG
from basf2 import find_file
from basf2 import conditions as b2conditions

from abc import ABC, abstractmethod

import caf
from caf.utils import B2INFO_MULTILINE
from caf.utils import past_from_future_dependencies
from caf.utils import topological_sort
from caf.utils import all_dependencies
from caf.utils import method_dispatch
from caf.utils import temporary_workdir
from caf.utils import find_int_dirs
from caf.utils import LocalDatabase
from caf.utils import CentralDatabase
from caf.utils import parse_file_uri

import caf.strategies as strategies
import caf.runners as runners
from caf.backends import MaxSubjobsSplitter, MaxFilesSplitter
from caf.state_machines import CalibrationMachine, ConditionError, MachineError
from caf.database import CAFDB


class Collection():
    """
    Keyword Arguments:
        collector (str, basf2.Module): The collector module  or module name for this `Collection`.
        input_files (list[str]): The input files to be used for only this `Collection`.
        pre_collection_path (basf2.Path): The reconstruction `basf2.Path` to be run prior to the Collector module.
        database_chain (list[CentralDatabase, LocalDatabase]): The database chain to be used initially for this `Collection`.
        output_patterns (list[str]): Output patterns of files produced by collector which will be used to pass to the
            `Algorithm.data_input` function. Setting this here, replaces the default completely.
        max_files_for_collector_job (int): Maximum number of input files sent to each collector subjob for this `Collection`.
            Technically this sets the SubjobSplitter to be used, not compatible with max_collector_jobs.
        max_collector_jobs (int): Maximum number of collector subjobs for this `Collection`.
            Input files are split evenly between them. Technically this sets the SubjobSplitter to be used. Not compatible with
            max_files_for_collector_job.
        backend_args (dict): The args for the backend submission of this `Collection`.
    """
    #: The default maximum number of collector jobs to create. Only used if max_collector_jobs or max_files_per_collector_job
    #  are not set.
    default_max_collector_jobs = 1000
    #: The name of the file containing the collector Job's dictionary. Useful for recovery of the job
    #  configuration of the ones that ran previously.
    job_config = "collector_job.json"

    def __init__(self,
                 collector=None,
                 input_files=None,
                 pre_collector_path=None,
                 database_chain=None,
                 output_patterns=None,
                 max_files_per_collector_job=None,
                 max_collector_jobs=None,
                 backend_args=None
                 ):
        #: Collector module of this collection
        self.collector = collector
        #: Internal input_files stored for this calibration
        self.input_files = []
        if input_files:
            self.input_files = input_files
        #: File -> Iov dictionary, should be
        #:
        #: >>> {absolute_file_path:iov}
        #:
        #: Where iov is a :py:class:`IoV <caf.utils.IoV>` object. Will be filled during `CAF.run()` if empty.
        #: To improve performance you can fill this yourself before calling `CAF.run()`
        self.files_to_iovs = {}
        #: Since many collectors require some different setup, if you set this attribute to a `basf2.Path` it will be run before
        #: the collector and after the default RootInput module + HistoManager setup.
        #: If this path contains RootInput then it's params are used in the RootInput module instead, except for the input_files
        #: parameter which is set to whichever files are passed to the collector subjob.
        self.pre_collector_path = None
        if pre_collector_path:
            self.pre_collector_path = pre_collector_path
        #: Output patterns of files produced by collector which will be used to pass to the `Algorithm.data_input` function.
        #: You can set these to anything understood by `glob.glob`, but if you want to specify this you should also specify
        #: the `Algorithm.data_input` function to handle the different types of files and call the
        #: CalibrationAlgorithm.setInputFiles() with the correct ones.
        self.output_patterns = ["CollectorOutput.root"]
        if output_patterns:
            self.output_patterns = output_patterns

        #: The SubjobSplitter to use when constructing collector subjobs from the overall Job object. If this is not set
        #  then your collector will run as one big job with all input files included.
        self.splitter = None
        if max_files_per_collector_job and max_collector_jobs:
            B2FATAL("Cannot set both 'max_files_per_collector_job' and 'max_collector_jobs' of a collection!")
        elif max_files_per_collector_job:
            self.max_files_per_collector_job = max_files_per_collector_job
        elif max_collector_jobs:
            self.max_collector_jobs = max_collector_jobs
        else:
            self.max_collector_jobs = self.default_max_collector_jobs

        #: Dictionary passed to the collector Job object to configure how the `caf.backends.Backend` instance should treat
        #: the collector job when submitting. The choice of arguments here depends on which backend you plan on using.
        self.backend_args = {}
        if backend_args:
            self.backend_args = backend_args

        if database_chain:
            #: The database chain used for this Collection. NOT necessarily the same database chain used for the algorithm
            #: step! Since the algorithm will merge the collected data into one process it has to use a single DB chain set from
            #: the overall Calibration.
            self.database_chain = database_chain
        else:
            self.database_chain = []
            # This may seem weird but the changes to the DB interface mean that they have effectively swapped from being
            # described well by appending to a list to a deque. So we do bit of reversal to translate it back and make the
            # most important GT the last one encountered.
            for tag in reversed(b2conditions.default_globaltags):
                self.use_central_database(tag)

        self.job_script = Path(find_file("calibration/scripts/caf/run_collector_path.py")).absolute()
        """The basf2 steering file that will be used for Collector jobs run by this collection.
This script will be copied into subjob directories as part of the input sandbox."""

        #: The Collector `caf.backends.Job.cmd` attribute. Probably using the `job_script` to run basf2.
        self.job_cmd = ["basf2", self.job_script.name, "--job-information job_info.json"]

    def reset_database(self):
        """
        Remove everything in the database_chain of this Calibration, including the default central database
        tag automatically included from `basf2.conditions.default_globaltags <ConditionsConfiguration.default_globaltags>`.
        """
        self.database_chain = []

    def use_central_database(self, global_tag):
        """
        Parameters:
            global_tag (str): The central database global tag to use for this calibration.

        Using this allows you to add a central database to the head of the global tag database chain for this collection.
        The default database chain is just the central one from
        `basf2.conditions.default_globaltags <ConditionsConfiguration.default_globaltags>`.
        The input file global tag will always be overrided and never used unless explicitly set.

        To turn off central database completely or use a custom tag as the base, you should call `Calibration.reset_database`
        and start adding databases with `Calibration.use_local_database` and `Calibration.use_central_database`.

        Alternatively you could set an empty list as the input database_chain when adding the Collection to the Calibration.

        NOTE!! Since ``release-04-00-00`` the behaviour of basf2 conditions databases has changed.
        All local database files MUST now be at the head of the 'chain', with all central database global tags in their own
        list which will be checked after all local database files have been checked.

        So even if you ask for ``["global_tag1", "localdb/database.txt", "global_tag2"]`` to be the database chain, the real order
        that basf2 will use them is ``["global_tag1", "global_tag2", "localdb/database.txt"]`` where the file is checked first.
        """
        central_db = CentralDatabase(global_tag)
        self.database_chain.append(central_db)

    def use_local_database(self, filename, directory=""):
        """
        Parameters:
            filename (str): The path to the database.txt of the local database
            directory (str): The path to the payloads directory for this local database.

        Append a local database to the chain for this collection.
        You can call this function multiple times and each database will be added to the chain IN ORDER.
        The databases are applied to this collection ONLY.

        NOTE!! Since release-04-00-00 the behaviour of basf2 conditions databases has changed.
        All local database files MUST now be at the head of the 'chain', with all central database global tags in their own
        list which will be checked after all local database files have been checked.

        So even if you ask for ["global_tag1", "localdb/database.txt", "global_tag2"] to be the database chain, the real order
        that basf2 will use them is ["global_tag1", "global_tag2", "localdb/database.txt"] where the file is checked first.
        """
        local_db = LocalDatabase(filename, directory)
        self.database_chain.append(local_db)

    @staticmethod
    def uri_list_from_input_file(input_file):
        """
        Parameters:
            input_file (str): A local file/glob pattern or XROOTD URI

        Returns:
            list: A list of the URIs found from the initial string.
        """
        # By default we assume it is a local file path if no "scheme" is given
        uri = parse_file_uri(input_file)
        if uri.scheme == "file":
            # For local files we also perform a glob just in case it is a wildcard pattern.
            # That way we will have all the uris of files separately
            uris = [parse_file_uri(f).geturl() for f in glob(input_file)]
        else:
            # Just let everything else through and hop the backend can figure it out
            uris = [input_file]
        return uris

    @property
    def input_files(self):
        return self._input_files

    @input_files.setter
    def input_files(self, value):
        if isinstance(value, str):
            # If it's a string, we convert to a list of URIs
            self._input_files = self.uri_list_from_input_file(value)
        elif isinstance(value, list):
            # If it's a list we loop and do the same thing
            total_files = []
            for pattern in value:
                total_files.extend(self.uri_list_from_input_file(pattern))
            self._input_files = total_files
        else:
            raise TypeError("Input files must be a list or string")

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

    def is_valid(self):
        if (not self.collector or not self.input_files):
            return False
        else:
            return True

    @property
    def max_collector_jobs(self):
        if self.splitter:
            return self.splitter.max_subjobs
        else:
            return None

    @max_collector_jobs.setter
    def max_collector_jobs(self, value):
        if value is None:
            self.splitter = None
        else:
            self.splitter = MaxSubjobsSplitter(max_subjobs=value)

    @property
    def max_files_per_collector_job(self):
        if self.splitter:
            return self.splitter.max_files_per_subjob
        else:
            return None

    @max_files_per_collector_job.setter
    def max_files_per_collector_job(self, value):
        if value is None:
            self.splitter = None
        else:
            self.splitter = MaxFilesSplitter(max_files_per_subjob=value)


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
        #: Marks this Calibration as one which has payloads that should be copied and uploaded.
        #: Defaults to True, and should only be False if this is an intermediate Calibration who's payloads are never needed.
        self.save_payloads = True
        #: A simple list of jobs that this Calibration wants submitted at some point.
        self.jobs_to_submit = []

    @abstractmethod
    def run(self):
        """
        The most important method. Runs inside a new Thread and is called from `CalibrationBase.start`
        once the dependencies of this `CalibrationBase` have returned with state == end_state i.e. "completed".
        """

    @abstractmethod
    def is_valid(self):
        """
        A simple method you should implement that will return True or False depending on whether
        the Calibration has been set up correctly and can be run safely.
        """

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
            B2WARNING((f"Tried to add {calibration} as a dependency for {self} but they have the same name."
                       "Dependency was not added."))

    def dependencies_met(self):
        """
        Checks if all of the Calibrations that this one depends on have reached a successful end state.
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
                if getattr(self, key) is None:
                    setattr(self, key, value)
            except AttributeError:
                print(f"The calibration {self.name} does not support the attribute {key}.")


class Calibration(CalibrationBase):
    """
    Every Calibration object must have at least one collector at least one algorithm.
    You have the option to add in your collector/algorithm by argument here, or add them
    later by changing the properties.

    If you plan to use multiple `Collection` objects I recommend that you only set the name here and add the Collections
    separately via `add_collection()`.

    Parameters:
        name (str): Name of this calibration. It should be unique for use in the `CAF`
    Keyword Arguments:
        collector (str, `basf2.Module`): Should be set to a CalibrationCollectorModule() or a string with the module name.
        algorithms (list, ``ROOT.Belle2.CalibrationAlgorithm``): The algorithm(s) to use for this `Calibration`.
        input_files (str, list[str]): Input files for use by this Calibration. May contain wildcards useable by `glob.glob`

    A Calibration won't be valid in the `CAF` until it has all of these four attributes set. For example:

    >>> cal = Calibration('TestCalibration1')
    >>> col1 = register_module('CaTest')
    >>> cal.add_collection('TestColl', col1)

    or equivalently

    >>> cal = Calibration('TestCalibration1', 'CaTest')

    If you want to run a basf2 :py:class:`path <basf2.Path>` before your collector module when running over data

    >>> cal.pre_collector_path = my_basf2_path

    You don't have to put a RootInput module in this pre-collection path, but you can if
    you need some special parameters. If you want to process sroot files the you have to explicitly add
    SeqRootInput to your pre-collection path.
    The inputFileNames parameter of (Seq)RootInput will be set by the CAF automatically for you.


    You can use optional arguments to pass in some/all during initialisation of the `Calibration` class

    >>> cal = Calibration( 'TestCalibration1', 'CaTest', [alg1,alg2], ['/path/to/file.root'])

    you can change the input file list later on, before running with `CAF`

    >>> cal.input_files = ['path/to/*.root', 'other/path/to/file2.root']

    If you have multiple collections from calling `add_collection()` then you should instead set the pre_collector_path,
    input_files, database chain etc from there. See `Collection`.

    Adding the CalibrationAlgorithm(s) is easy

    >>> alg1 = TestAlgo()
    >>> cal.algorithms = alg1

    Or equivalently

    >>> cal.algorithms = [alg1]

    Or for multiple algorithms for one collector

    >>> alg2 = TestAlgo()
    >>> cal.algorithms = [alg1, alg2]

    Note that when you set the algorithms, they are automatically wrapped and stored as a Python class
    `Algorithm`. To access the C++ algorithm clas underneath directly do:

    >>> cal.algorithms[i].algorithm

    If you have a setup function that you want to run before each of the algorithms, set that with

    >>> cal.pre_algorithms = my_function_object

    If you want a different setup for each algorithm use a list with the same number of elements
    as your algorithm list.

    >>> cal.pre_algorithms = [my_function1, my_function2, ...]

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
    #: Default collection name
    default_collection_name = "default"

    def __init__(self,
                 name,
                 collector=None,
                 algorithms=None,
                 input_files=None,
                 pre_collector_path=None,
                 database_chain=None,
                 output_patterns=None,
                 max_files_per_collector_job=None,
                 max_collector_jobs=None,
                 backend_args=None
                 ):
        """
        """
        #: Collections stored for this calibration.
        self.collections = {}
        #: Internal calibration algorithms stored for this calibration
        self._algorithms = []

        # Default collection added, will have None type and requires setting later via `self.collector`, or will take the
        # CollectorModule/module name directly.
        self.add_collection(self.default_collection_name,
                            Collection(collector,
                                       input_files,
                                       pre_collector_path,
                                       database_chain,
                                       output_patterns,
                                       max_files_per_collector_job,
                                       max_collector_jobs,
                                       backend_args
                                       ))

        super().__init__(name, input_files)
        if algorithms:
            #: `Algorithm` classes that wil be run by this `Calibration`. You should set this attribute to either a single
            #: CalibrationAlgorithm C++ class, or a `list` of them if you want to run multiple CalibrationAlgorithms using one
            #: CalibrationCollectorModule.
            self.algorithms = algorithms
        #: Output results of algorithms for each iteration
        self.results = {}
        #: Variable to define the maximum number of iterations for this calibration specifically.
        #: It overrides tha CAF calibration_defaults value if set.
        self.max_iterations = None
        #: List of ExpRun that will be ignored by this Calibration. This runs will not have Collector jobs run on
        #: them (if possible). And the algorithm execution will exclude them from a ExpRun list. However, the
        #: algorithm execution may merge IoVs of final payloads to cover the 'gaps' caused by these runs.
        #: You should pay attention to what the AlgorithmStrategy you choose will do in these cases.
        self.ignored_runs = None
        if self.algorithms:
            #: The strategy that the algorithm(s) will be run against. Assign a list of strategies the same length as the number of
            #: algorithms, or assign a single strategy to apply it to all algorithms in this `Calibration`. You can see the choices
            #: in :py:mod:`caf.strategies`.
            self.strategies = strategies.SingleIOV
        if database_chain:
            #: The database chain that is applied to the algorithms.
            #: This is often updated at the same time as the database chain for the default `Collection`.
            self.database_chain = database_chain
        else:
            self.database_chain = []
            # This database is already applied to the `Collection` automatically, so don't do it again
            for tag in reversed(b2conditions.default_globaltags):
                self.use_central_database(tag, apply_to_default_collection=False)
        #: The class that runs all the algorithms in this Calibration using their assigned
        #: :py:class:`caf.strategies.AlgorithmStrategy`.
        #: Plugin your own runner class to change how your calibration will run the list of algorithms.
        self.algorithms_runner = runners.SeqAlgorithmsRunner
        #: The `backend <backends.Backend>` we'll use for our collector submission in this calibration.
        #: If `None` it will be set by the CAF used to run this Calibration (recommended!).
        self.backend = None
        #: While checking if the collector is finished we don't bother wastefully checking every subjob's status.
        #: We exit once we find the first subjob that isn't ready.
        #: But after this interval has elapsed we do a full :py:meth:`caf.backends.Job.update_status` call and
        #: print the fraction of SubJobs completed.
        self.collector_full_update_interval = 30
        #: This calibration's sleep time before rechecking to see if it can move state
        self.heartbeat = 3
        #: The `caf.state_machines.CalibrationMachine` that we will run to process this calibration start to finish.
        self.machine = None
        #: Location of a SQLite database that will save the state of the calibration so that it can be restarted from failure.
        self._db_path = None

    def add_collection(self, name, collection):
        """
        Parameters:
            name (str): Unique name of this `Collection` in the Calibration.
            collection (`Collection`): `Collection` object to use.

        Adds a new `Collection` object to the `Calibration`. Any valid Collection will be used in the Calibration.
        A default Collection is automatically added but isn't valid and won't run unless you have assigned a collector
        + input files.
        You can ignore the default one and only add your own custom Collections. You can configure the default from the
        Calibration(...) arguments or after creating the Calibration object via directly setting the cal.collector, cal.input_files
        attributes.
        """
        if name not in self.collections:
            self.collections[name] = collection
        else:
            B2WARNING(f"A Collection with the name '{name}' already exists in this Calibration. It has not been added."
                      "Please use another name.")

    def is_valid(self):
        """
        A full calibration consists of a collector AND an associated algorithm AND input_files.

        Returns False if:
            1) We are missing any of the above.
            2) There are multiple Collections and the Collectors have mis-matched granularities.
            3) Any of our Collectors have granularities that don't match what our Strategy can use.
        """
        if not self.algorithms:
            B2WARNING(f"Empty algorithm list for {self.name}.")
            return False

        if not any([collection.is_valid() for collection in self.collections.values()]):
            B2WARNING(f"No valid Collections for {self.name}.")
            return False

        granularities = []
        for collection in self.collections.values():
            if collection.is_valid():
                collector_params = collection.collector.available_params()
                for param in collector_params:
                    if param.name == "granularity":
                        granularities.append(param.values)
        if len(set(granularities)) > 1:
            B2WARNING("Multiple different granularities set for the Collections in this Calibration.")
            return False

        for alg in self.algorithms:
            alg_type = type(alg.algorithm).__name__
            incorrect_gran = [granularity not in alg.strategy.allowed_granularities for granularity in granularities]
            if any(incorrect_gran):
                B2WARNING(f"Selected strategy for {alg_type} does not match a collector's granularity.")
                return False
        return True

    def reset_database(self, apply_to_default_collection=True):
        """
        Keyword Arguments:
            apply_to_default_collection (bool): Should we also reset the default collection?

        Remove everything in the database_chain of this Calibration, including the default central database tag automatically
        included from `basf2.conditions.default_globaltags <ConditionsConfiguration.default_globaltags>`. This will NOT affect the
        database chain of any `Collection` other than the default one. You can prevent the default Collection from having its chain
        reset by setting 'apply_to_default_collection' to False.
        """
        self.database_chain = []
        if self.default_collection_name in self.collections and apply_to_default_collection:
            self.collections[self.default_collection_name].reset_database()

    def use_central_database(self, global_tag, apply_to_default_collection=True):
        """
        Parameters:
            global_tag (str): The central database global tag to use for this calibration.

        Keyword Arguments:
            apply_to_default_collection (bool): Should we also call use_central_database on the default collection (if it exists)

        Using this allows you to append a central database to the database chain for this calibration.
        The default database chain is just the central one from
        `basf2.conditions.default_globaltags <ConditionsConfiguration.default_globaltags>`.
        To turn off central database completely or use a custom tag as the base, you should call `Calibration.reset_database`
        and start adding databases with `Calibration.use_local_database` and `Calibration.use_central_database`.

        Note that the database chain attached to the `Calibration` will only affect the default `Collection` (if it exists),
        and the algorithm processes. So calling:

        >> cal.use_central_database("global_tag")

        will modify the database chain used by all the algorithms assigned to this `Calibration`, and modifies the database chain
        assigned to

        >> cal.collections['default'].database_chain

        But calling

        >> cal.use_central_database(file_path, payload_dir, False)

        will add the database to the Algorithm processes, but leave the default Collection database chain untouched.
        So if you have multiple Collections in this Calibration *their database chains are separate*.
        To specify an additional `CentralDatabase` for a different collection, you will have to call:

        >> cal.collections['OtherCollection'].use_central_database("global_tag")
        """
        central_db = CentralDatabase(global_tag)
        self.database_chain.append(central_db)
        if self.default_collection_name in self.collections and apply_to_default_collection:
            self.collections[self.default_collection_name].use_central_database(global_tag)

    def use_local_database(self, filename, directory="", apply_to_default_collection=True):
        """
        Parameters:
            filename (str): The path to the database.txt of the local database

        Keyword Argumemts:
            directory (str): The path to the payloads directory for this local database.
            apply_to_default_collection (bool): Should we also call use_local_database on the default collection (if it exists)

        Append a local database to the chain for this calibration.
        You can call this function multiple times and each database will be added to the chain IN ORDER.
        The databases are applied to this calibration ONLY.
        The Local and Central databases applied via these functions are applied to the algorithm processes and optionally
        the default `Collection` job as a database chain.
        There are other databases applied to the processes later, checked by basf2 in this order:

        1) Local Database from previous iteration of this Calibration.
        2) Local Database chain from output of previous dependent Calibrations.
        3) This chain of Local and Central databases where the last added is checked first.

        Note that this function on the `Calibration` object will only affect the default `Collection` if it exists and if
        'apply_to_default_collection' remains True. So calling:

        >> cal.use_local_database(file_path, payload_dir)

        will modify the database chain used by all the algorithms assigned to this `Calibration`, and modifies the database chain
        assigned to

        >> cal.collections['default'].database_chain

        But calling

        >> cal.use_local_database(file_path, payload_dir, False)

        will add the database to the Algorithm processes, but leave the default Collection database chain untouched.

        If you have multiple Collections in this Calibration *their database chains are separate*.
        To specify an additional `LocalDatabase` for a different collection, you will have to call:

        >> cal.collections['OtherCollection'].use_local_database(file_path, payload_dir)

        """
        local_db = LocalDatabase(filename, directory)
        self.database_chain.append(local_db)
        if self.default_collection_name in self.collections and apply_to_default_collection:
            self.collections[self.default_collection_name].use_local_database(filename, directory)

    def _get_default_collection_attribute(self, attr):
        if self.default_collection_name in self.collections:
            return getattr(self.collections[self.default_collection_name], attr)
        else:
            B2WARNING(f"You tried to get the attribute '{attr}' from the Calibration '{self.name}', "
                      "but the default collection doesn't exist."
                      f"You should use the cal.collections['CollectionName'].{attr} to access a custom "
                      "collection's attributes directly.")
            return None

    def _set_default_collection_attribute(self, attr, value):
        if self.default_collection_name in self.collections:
            setattr(self.collections[self.default_collection_name], attr, value)
        else:
            B2WARNING(f"You tried to set the attribute '{attr}' from the Calibration '{self.name}', "
                      "but the default collection doesn't exist."
                      f"You should use the cal.collections['CollectionName'].{attr} to access a custom "
                      "collection's attributes directly.")

    @property
    def collector(self):
        """
        """
        return self._get_default_collection_attribute("collector")

    @collector.setter
    def collector(self, collector):
        """
        """
        # check if collector is already a module or if we need to create one
        # from the name
        from basf2 import Module
        if isinstance(collector, str):
            from basf2 import register_module
            collector = register_module(collector)
        if not isinstance(collector, Module):
            B2ERROR("Collector needs to be either a Module or the name of such a module")

        self._set_default_collection_attribute("collector", collector)

    @property
    def input_files(self):
        """
        """
        return self._get_default_collection_attribute("input_files")

    @input_files.setter
    def input_files(self, files):
        """
        """
        self._set_default_collection_attribute("input_files", files)

    @property
    def files_to_iovs(self):
        """
        """
        return self._get_default_collection_attribute("files_to_iovs")

    @files_to_iovs.setter
    def files_to_iovs(self, file_map):
        """
        """
        self._set_default_collection_attribute("files_to_iovs", file_map)

    @property
    def pre_collector_path(self):
        """
        """
        return self._get_default_collection_attribute("pre_collector_path")

    @pre_collector_path.setter
    def pre_collector_path(self, path):
        """
        """
        self._set_default_collection_attribute("pre_collector_path", path)

    @property
    def output_patterns(self):
        """
        """
        return self._get_default_collection_attribute("output_patterns")

    @output_patterns.setter
    def output_patterns(self, patterns):
        """
        """
        self._set_default_collection_attribute("output_patterns", patterns)

    @property
    def max_files_per_collector_job(self):
        """
        """
        return self._get_default_collection_attribute("max_files_per_collector_job")

    @max_files_per_collector_job.setter
    def max_files_per_collector_job(self, max_files):
        """
        """
        self._set_default_collection_attribute("max_files_per_collector_job", max_files)

    @property
    def max_collector_jobs(self):
        """
        """
        return self._get_default_collection_attribute("max_collector_jobs")

    @max_collector_jobs.setter
    def max_collector_jobs(self, max_jobs):
        """
        """
        self._set_default_collection_attribute("max_collector_jobs", max_jobs)

    @property
    def backend_args(self):
        """
        """
        return self._get_default_collection_attribute("backend_args")

    @backend_args.setter
    def backend_args(self, args):
        """
        """
        self._set_default_collection_attribute("backend_args", args)

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
            B2ERROR(f"Something other than CalibrationAlgorithm instance passed in ({type(value)}). "
                    "Algorithm needs to inherit from Belle2::CalibrationAlgorithm")

    @algorithms.fset.register(tuple)
    @algorithms.fset.register(list)
    def _(self, value):
        """
        Alternate algorithms setter for lists and tuples of CalibrationAlgorithms.
        """
        from ROOT.Belle2 import CalibrationAlgorithm
        if value:
            self._algorithms = []
            for alg in value:
                if isinstance(alg, CalibrationAlgorithm):
                    self._algorithms.append(Algorithm(alg))
                else:
                    B2ERROR((f"Something other than CalibrationAlgorithm instance passed in {type(value)}."
                             "Algorithm needs to inherit from Belle2::CalibrationAlgorithm"))

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
        with CAFDB(self._db_path, read_only=True) as db:
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

        # Before we start running, let's clean up any iteration directories from iterations above our initial one.
        # Should prevent confusion between attempts if we fail again.
        all_iteration_paths = find_int_dirs(self.machine.root_dir)
        for iteration_path in all_iteration_paths:
            if int(iteration_path.name) > initial_iteration:
                shutil.rmtree(iteration_path)

        while self.state != self.end_state and self.state != self.fail_state:
            if self.state == "init":
                try:
                    B2INFO(f"Attempting collector submission for calibration {self.name}.")
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
                B2INFO(f"Attempting to run algorithms for calibration {self.name}.")
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
                self.machine.complete()
            # ConditionError is thrown when the condtions for the transition have returned false, it's not serious.
            except ConditionError:
                try:
                    B2DEBUG(29, f"Checking if collector jobs for calibration {self.name} have failed.")
                    self.machine.fail()
                except ConditionError:
                    pass
            sleep(self.heartbeat)  # Sleep until we want to check again

    @property
    def state(self):
        """
        The current major state of the calibration in the database file. The machine may have a different state.
        """
        with CAFDB(self._db_path, read_only=True) as db:
            state = db.get_calibration_value(self.name, "state")
        return state

    @state.setter
    def state(self, state):
        """
        """
        B2DEBUG(29, f"Setting {self.name} to state {state}.")
        with CAFDB(self._db_path) as db:
            db.update_calibration_value(self.name, "state", str(state))
            if state in self.checkpoint_states:
                db.update_calibration_value(self.name, "checkpoint", str(state))
        B2DEBUG(29, f"{self.name} set to {state}.")

    @property
    def iteration(self):
        """
        Retrieves the current iteration number in the database file.

        Returns:
            int: The current iteration number
        """
        with CAFDB(self._db_path, read_only=True) as db:
            iteration = db.get_calibration_value(self.name, "iteration")
        return iteration

    @iteration.setter
    def iteration(self, iteration):
        """
        """
        B2DEBUG(29, f"Setting {self.name} to {iteration}.")
        with CAFDB(self._db_path) as db:
            db.update_calibration_value(self.name, "iteration", iteration)
        B2DEBUG(29, f"{self.name} set to {self.iteration}.")


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
        self.name = algorithm.__cppname__[algorithm.__cppname__.rfind('::') + 2:]
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
        info_lines = [f"Input files used in {self.name}:"]
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

    #: The name of the SQLite DB that gets created
    _db_name = "caf_state.db"
    #: The defaults for Calibrations
    default_calibration_config = {
                                  "max_iterations": 5,
                                  "ignored_runs": []
                                 }

    def __init__(self, calibration_defaults=None):
        """
        """
        #: Dictionary of calibrations for this `CAF` instance. You should use `add_calibration` to add to this.
        self.calibrations = {}
        #: Dictionary of future dependencies of `Calibration` objects, where the value is all
        #: calibrations that will depend on the key, filled during self.run()
        self.future_dependencies = {}
        #: Dictionary of dependencies of `Calibration` objects, where value is the list of `Calibration` objects
        #: that the key depends on. This attribute is filled during self.run()
        self.dependencies = {}
        #: Output path to store results of calibration and bookkeeping information
        self.output_dir = "calibration_results"
        #: The ordering and explicit future dependencies of calibrations. Will be filled during `CAF.run()` for you.
        self.order = None
        #: Private backend attribute
        self._backend = None
        #: The heartbeat (seconds) between polling for Calibrations that are finished
        self.heartbeat = 5

        if not calibration_defaults:
            calibration_defaults = {}
        #: Default options applied to each calibration known to the `CAF`, if the `Calibration` has these defined by the user
        #: then the defaults aren't applied. A simple way to define the same configuration to all calibrations in the `CAF`.
        self.calibration_defaults = {**self.default_calibration_config, **calibration_defaults}
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
                B2WARNING(f"Tried to add a calibration with the name {calibration.name} twice.")
        else:
            B2WARNING((f"Tried to add incomplete/invalid calibration ({calibration.name}) to the framwork."
                       "It was not added and will not be part of the final process."))

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
                B2WARNING(f"The calibration {dependency.name} is a required dependency but is not in the CAF."
                          " It has been removed as a dependency.")
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
            # At this point the calibrations have their full dependencies but they aren't in topological
            # sort order. Correct that here
            ordered_dependency_list = []
            for ordered_calibration_name in order:
                if ordered_calibration_name in [dep.name for dep in calibration.dependencies]:
                    ordered_dependency_list.append(self.calibrations[ordered_calibration_name])
            calibration.dependencies = ordered_dependency_list
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

    def _prune_invalid_collections(self):
        """
        Checks all current calibrations and removes any invalid Collections from their collections list.
        """
        B2INFO("Checking for any invalid Collections in Calibrations.")
        for calibration in self.calibrations.values():
            valid_collections = {}
            for name, collection in calibration.collections.items():
                if collection.is_valid():
                    valid_collections[name] = collection
                else:
                    B2WARNING(f"Removing invalid Collection '{name}' from Calibration '{calibration.name}'.")
            calibration.collections = valid_collections

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

        self._prune_invalid_collections()

        # Creates the overall output directory and reset the attribute to use an absolute path to it.
        self.output_dir = self._make_output_dir()

        #  Creates a SQLite DB to save the status of the various calibrations
        self._make_database()

        # Enter the overall output dir during processing and opena  connection to the DB
        with temporary_workdir(self.output_dir):
            db = CAFDB(self._db_path)
            db.open()
            db_initial_calibrations = db.query("select * from calibrations").fetchall()
            for calibration in self.calibrations.values():
                # Apply defaults given to the `CAF` to the calibrations if they aren't set
                calibration._apply_calibration_defaults(self.calibration_defaults)
                calibration._db_path = self._db_path
                calibration.output_database_dir = Path(self.output_dir, calibration.name, "outputdb").as_posix()
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
                    B2INFO(f"Previous entry in database found for {calibration.name}.")
                    B2INFO(f"Setting {calibration.name} state to checkpoint state '{cal_initial_state}'.")
                    calibration.state = cal_initial_state
                    B2INFO(f"Setting {calibration.name} iteration to '{cal_initial_iteration}'.")
                    calibration.iteration = cal_initial_iteration
                # Daemonize so that it exits if the main program exits
                calibration.daemon = True

            db.close()

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
                            B2DEBUG(29, f"Joining {calibration.name}.")
                            calibration.join()
                    else:
                        if calibration.dependencies_met():
                            if not calibration.is_alive():
                                B2DEBUG(29, f"Starting {calibration.name}.")
                                calibration.start()
                            remaining_calibrations.append(calibration)
                        else:
                            if not calibration.failed_dependencies():
                                remaining_calibrations.append(calibration)
                if remaining_calibrations:
                    keep_running = True
                    # Loop over jobs that the calibrations want submitted and submit them.
                    # We do this here because some backends don't like us submitting in parallel from multiple CalibrationThreads
                    # So this is like a mini job queue without getting too clever with it
                    for calibration in remaining_calibrations:
                        for job in calibration.jobs_to_submit[:]:
                            calibration.backend.submit(job)
                            calibration.jobs_to_submit.remove(job)
                sleep(self.heartbeat)

            B2INFO("Printing summary of final CAF status.")
            with CAFDB(self._db_path, read_only=True) as db:
                print(db.output_calibration_table())

    @property
    def backend(self):
        """
        The `backend <backends.Backend>` that runs the collector job.
        When set, this is checked that a `backends.Backend` class instance was passed in.
        """
        return self._backend

    @backend.setter
    def backend(self, backend):
        """
        """
        if isinstance(backend, caf.backends.Backend):
            self._backend = backend
        else:
            B2ERROR('Backend property must inherit from Backend class.')

    def _make_output_dir(self):
        """
        Creates the output directory. If it already exists we are now going to try and restart the program from the last state.

        Returns:
            str: The absolute path of the new output_dir
        """
        p = Path(self.output_dir).resolve()
        if p.is_dir():
            B2INFO(f"{p.as_posix()} output directory already exists. "
                   "We will try to restart from the previous finishing state.")
            return p.as_posix()
        else:
            p.mkdir(parents=True)
            if p.is_dir():
                return p.as_posix()
            else:
                raise FileNotFoundError(f"Attempted to create output_dir {p.as_posix()}, but it didn't work.")

    def _make_database(self):
        """
        Creates the CAF status database. If it already exists we don't overwrite it.
        """
        self._db_path = Path(self.output_dir, self._db_name).absolute()
        if self._db_path.exists():
            B2INFO(f"Previous CAF database found {self._db_path}")
        # Will create a new database + tables, or do nothing but checks we can connect to existing one
        with CAFDB(self._db_path):
            pass
