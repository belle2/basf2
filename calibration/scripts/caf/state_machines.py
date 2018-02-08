#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from functools import partial
from collections import defaultdict

import configparser
import pickle
import multiprocessing
import glob
import shutil

from basf2 import *
import ROOT
from ROOT.Belle2 import PyStoreObj, CalibrationAlgorithm, IntervalOfValidity

from .utils import method_dispatch
from .utils import merge_local_databases
from .utils import decode_json_string
from .utils import iov_from_runs
from .utils import IoV_Result
from .utils import IoV
from .utils import AlgResult
from .utils import get_iov_from_file
from .utils import find_absolute_file_paths
from .utils import runs_overlapping_iov
from .utils import runs_from_vector
from .backends import Job
from .backends import LSF
from .backends import PBS
from .backends import Local


class State():
    """
    Basic State object that can take enter and exit state methods and records
    the state of a machine.

    You should assign the self.on_enter or self.on_exit attributes to callback functions
    or lists of them, if you need them.
    """

    def __init__(self, name, enter=None, exit=None):
        """
        Initialise State with a name and optional lists of callbacks
        """
        #: Name of the State
        self.name = name
        #: Callback list when entering state
        self.on_enter = enter
        #: Callback list when exiting state
        self.on_exit = exit

    @property
    def on_enter(self):
        """
        Runs callbacks when a state is entered.
        """
        return self._on_enter

    @on_enter.setter
    def on_enter(self, callbacks):
        """
        """
        self._on_enter = []
        if callbacks:
            self._add_callbacks(callbacks, self._on_enter)

    @property
    def on_exit(self):
        """
        Runs callbacks when a state is exited.
        """
        return self._on_exit

    @on_exit.setter
    def on_exit(self, callbacks):
        """
        """
        self._on_exit = []
        if callbacks:
            self._add_callbacks(callbacks, self._on_exit)

    @method_dispatch
    def _add_callbacks(self, callback, attribute):
        """
        Adds callback to a property.
        """
        if callable(callback):
            attribute.append(callback)
        else:
            B2ERROR("Something other than a function (callable) passed into State {0}.".format(self.name))

    @_add_callbacks.register(tuple)
    @_add_callbacks.register(list)
    def _(self, callbacks, attribute):
        """
        Alternate method for lists and tuples of function objects
        """
        if callbacks:
            for function in callbacks:
                if callable(function):
                    attribute.append(function)
                else:
                    B2ERROR("Something other than a function (callable) passed into State {0}.".format(self.name))

    def __str__(self):
        """
        """
        return self.name

    def __repr__(self):
        """
        """
        return "State(name={0})".format(self.name)

    def __eq__(self, other):
        """
        """
        if isinstance(other, str):
            return self.name == other
        else:
            return self.name == other.name

    def __hash__(self):
        """
        """
        return hash(self.name)


class Machine():
    """
    Parameters:
      states (list[str]): A list of possible states of the machine.
      initial_state (str):

    Base class for a final state machine wrapper.
    Implements the framwork that a more complex machine can inherit from.

    The `transitions` attribute is a dictionary of trigger name keys, each value of
    which is another dictionary of 'source' states, 'dest' states, and 'conditions'
    methods. 'conditions' should be a list of callables or a single one. A transition is
    valid if it goes from an allowed state to an allowed state.
    Conditions are optional but must be a callable that returns True or False based
    on some state of the machine. They cannot have input arguments currently.

    Every condition/before/after callback function MUST take ``**kwargs`` as the only
    argument (except ``self`` if it's a class method). This is because it's basically
    impossible to determine which arguments to pass to which functions for a transition.
    Therefore this machine just enforces that every function should simply take ``**kwargs``
    and use the dictionary of arguments (even if it doesn't need any arguments).

    This also means that if you call a trigger with arguments e.g. ``machine.walk(speed=5)``
    you MUST use the keyword arguments rather than positional ones. So ``machine.walk(5)``
    will *not* work.
    """

    def __init__(self, states=None, initial_state="default_initial"):
        """
        Basic Setup of states and initial_state
        """
        #: Valid states for this machine
        self.states = {}
        if states:
            for state in states:
                self.add_state(state)
        if initial_state != "default_initial":
            #: Initial state for this machine. Is actually a `property` decorator
            self.initial_state = initial_state
        else:
            self.add_state(initial_state)
            #: Actual attribute holding initial state for this machine
            self._initial_state = State(initial_state)

        #: Actual attribute holding the Current state
        self._state = self.initial_state
        #: Allowed transitions between states
        self.transitions = defaultdict(list)

    def add_state(self, state, enter=None, exit=None):
        """
        Adds a single state to the list of possible ones.
        Should be a unique string or a State object with a unique name.
        """
        if isinstance(state, str):
            self.add_state(State(state, enter, exit))
        elif isinstance(state, State):
            if state.name not in self.states.keys():
                self.states[state.name] = state
            else:
                B2WARNING("You asked to add a state {0} but it was already in the machine states.".format(state))
        else:
            B2WARNING("You asked to add a state ({0}) but it wasn't a State or str object".format(state))

    @property
    def initial_state(self):
        """
        The initial state of the machine. Needs a special property to prevent trying to run on_enter callbacks when set.
        """
        return self._initial_state

    @initial_state.setter
    def initial_state(self, state):
        """
        """
        if state in self.states.keys():
            self._initial_state = self.states[state]
            #: Current state (private)
            self._state = self.states[state]
        else:
            raise KeyError("Attempted to set state to '{0}' which is not in the 'states' attribute!".format(state))

    @property
    def state(self):
        """
                The current state of the machine. Actually a `property` decorator. It will call the exit method of the
                current state and enter method of the new one. To get around the behaviour e.g. for setting initial states,
                either use the `initial_state` property or directly set the _state attribute itself (at your own risk!).
        """
        return self._state

    @state.setter
    def state(self, state):
        """
        """
        if isinstance(state, str):
            state_name = state
        else:
            state_name = state.name

        try:
            state = self.states[state_name]
            # Run exit callbacks of current state
            for callback in self.state.on_exit:
                callback(prior_state=self.state, new_state=state)
            # Run enter callbacks of new state
            for callback in state.on_enter:
                callback(prior_state=self.state, new_state=state)
            # Set the state
            self._state = state
        except KeyError:
            raise MachineError("Attempted to set state to '{0}' which not in the 'states' attribute!".format(state))

    @staticmethod
    def default_condition(**kwargs):
        """
        Method to always return True.
        """
        return True

    def add_transition(self, trigger, source, dest, conditions=None, before=None, after=None):
        """
        Adds a single transition to the dictionary of possible ones.
        Trigger is the method name that begins the transtion between the
        source state and the destination state.

        The condition is an optional function that returns True or False
        depending on the current state/input.
        """
        transition_dict = {}
        try:
            source = self.states[source]
            dest = self.states[dest]
            transition_dict["source"] = source
            transition_dict["dest"] = dest
        except KeyError as err:
            B2WARNING("Tried to add a transition where the source or dest isn't in the list of states")
            raise err
        if conditions:
            if isinstance(conditions, (list, tuple, set)):
                transition_dict["conditions"] = list(conditions)
            else:
                transition_dict["conditions"] = [conditions]
        else:
            transition_dict["conditions"] = [Machine.default_condition]

        if not before:
            before = []
        if isinstance(before, (list, tuple, set)):
            transition_dict["before"] = list(before)
        else:
            transition_dict["before"] = [before]

        if not after:
            after = []
        if isinstance(after, (list, tuple, set)):
            transition_dict["after"] = list(after)
        else:
            transition_dict["after"] = [after]

        self.transitions[trigger].append(transition_dict)

    def __getattr__(self, name, **kwargs):
        """
        Allows us to create a new method for each trigger on the fly.
        If there is no trigger name in the machine to match, then the normal
        AttributeError is called.
        """
        possible_transitions = self.get_transitions(self.state)
        if name not in possible_transitions:
            raise AttributeError("{0} does not exist in transitions for state {1}".format(name, self.state))
        transition_dict = self.get_transition_dict(self.state, name)
        return partial(self._trigger, name, transition_dict, **kwargs)

    def _trigger(self, transition_name, transition_dict, **kwargs):
        """
        Runs the transition logic. Callbacks are evaluated in the order:
        conditions -> before -> <new state set here> -> after
        """
        source, dest, conditions, before_callbacks, after_callbacks = (transition_dict["source"],
                                                                       transition_dict["dest"],
                                                                       transition_dict["conditions"],
                                                                       transition_dict["before"],
                                                                       transition_dict["after"])
        # Returns True only if every condition returns True when called
        if all(map(lambda condition: self._callback(condition, **kwargs), conditions)):
            for before_func in before_callbacks:
                self._callback(before_func, **kwargs)
            #: Current State of machine
            self.state = dest
            for after_func in after_callbacks:
                self._callback(after_func, **kwargs)
        else:
            raise ConditionError(("Transition '{0}' called for but one or more conditions "
                                  "evaluated False".format(transition_name)))

    @staticmethod
    def _callback(func, **kwargs):
        """
        Calls a condition/before/after.. function using arguments passed (or not)
        """
        return func(**kwargs)

    def get_transitions(self, source):
        """
        Returns allowed transitions from a given state
        """
        possible_transitions = []
        for transition, transition_dicts in self.transitions.items():
            for transition_dict in transition_dicts:
                if transition_dict["source"] == source:
                    possible_transitions.append(transition)
        return possible_transitions

    def get_transition_dict(self, state, transition):
        """
        Returns the transition dictionary for a state and transition out of it
        """
        transition_dicts = self.transitions[transition]
        for transition_dict in transition_dicts:
            if transition_dict["source"] == state:
                return transition_dict
        else:
            raise KeyError("No transition from state {0} with the name {1}".format(state, transition))

    def save_graph(self, filename, graphname):
        """
        Does a simple dot file creation to visualise states and transiitons
        """
        with open(filename, "w") as dotfile:
            dotfile.write("digraph " + graphname + " {\n")
            for state in self.states.keys():
                dotfile.write('"' + state + '" [shape=ellipse, color=black]\n')
            for trigger, transition_dicts in self.transitions.items():
                for transition in transition_dicts:
                    dotfile.write('"' + transition["source"].name + '" -> "' +
                                  transition["dest"].name + '" [label="' + trigger + '"]\n')
            dotfile.write("}\n")


class CalibrationMachine(Machine):
    """
    A state machine to handle `Calibration` objects and the flow of
    processing for them.
    """

    def __init__(self, calibration, iov_to_calibrate=None, initial_state="init", iteration=0):
        """
        Takes a Calibration object from the caf framework and lets you
        set the initial state
        """
        #: States that are defaults to the `CalibrationMachine` (could override later)
        self.default_states = [State("init"),
                               State("running_collector", enter=self._log_new_state),
                               State("collector_failed", enter=self._log_new_state),
                               State("collector_completed", enter=self._log_new_state),
                               State("running_algorithms", enter=self._log_new_state),
                               State("algorithms_failed", enter=self._log_new_state),
                               State("algorithms_completed", enter=self._log_new_state),
                               State("completed", enter=self._log_new_state)]

        super().__init__(self.default_states, initial_state)
        self.setup_defaults()

        #: Calibration object whose state we are modelling
        self.calibration = calibration
        # Monkey Patching for the win!
        #: Allows calibration object to hold a refernce to the machine controlling it
        self.calibration.machine = self
        #: Which iteration step are we in
        self.iteration = iteration
        #: Backend used for this calibration machine collector
        self.collector_backend = None
        #: Results of each iteration for all algorithms of this calibration
        self._algorithm_results = {}
        #: IoV to be executed, currently will loop over all runs in IoV
        self.iov_to_calibrate = iov_to_calibrate
        #: root directory for this Calibration
        self.root_dir = os.path.join(os.getcwd(), calibration.name)

        self.add_transition("submit_collector", "init", "running_collector",
                            conditions=self.dependencies_completed,
                            before=[self._make_output_dir,
                                    self._resolve_file_paths,
                                    self._build_iov_dict,
                                    self._create_collector_job,
                                    self._submit_collector])
        self.add_transition("fail", "running_collector", "collector_failed")
        self.add_transition("complete", "running_collector", "collector_completed",
                            conditions=self._collector_ready,
                            before=self._post_process_collector)
        self.add_transition("run_algorithms", "collector_completed", "running_algorithms",
                            after=[self._run_algorithms,
                                   self.automatic_transition])
        self.add_transition("complete", "running_algorithms", "algorithms_completed",
                            after=self.automatic_transition)
        self.add_transition("fail", "running_algorithms", "algorithms_failed",
                            conditions=self._any_failed_iov)
        self.add_transition("iterate", "algorithms_completed", "init",
                            conditions=[self._require_iteration,
                                        self._below_max_iterations],
                            after=self._increment_iteration)
        self.add_transition("finish", "algorithms_completed", "completed",
                            conditions=self._no_require_iteration,
                            before=self._prepare_final_db)

    def files_containing_iov(self, iov):
        """
        Lookup function that takes an IoV and returns all files from the input_files that
        overlap with this IoV
        """
        # Files that contain an Exp,Run range that overlaps with given IoV
        overlapping_files = []

        for file_path, file_iov in self.calibration.files_to_iovs.items():
            if file_iov.overlaps(iov):
                overlapping_files.append(file_path)
        return overlapping_files

    def _iov_requested(self):
        """
        """
        if self.iov_to_calibrate:
            B2DEBUG(100, "Overall IoV {0} requested for calibration: {1}".format(str(self.iov_to_calibrate), self.calibration.name))
            return True
        else:
            B2DEBUG(100, "No overall IoV requested for calibration: {0}".format(self.calibration.name))
            return False

    def _resolve_file_paths(self):
        """
        """
        if isinstance(self.collector_backend, Local) or \
           isinstance(self.collector_backend, PBS) or \
           isinstance(self.collector_backend, LSF):
            B2INFO("Resolving absolute paths of input files for calibration: {0}".format(self.calibration.name))
            self.calibration.input_files = find_absolute_file_paths(self.calibration.input_files)

    def _build_iov_dict(self):
        """
        Build IoV file dictionary for each calibration if required.
        """
        iov_requested = self._iov_requested()
        if iov_requested and not self.calibration.files_to_iovs:
            B2INFO(("Creating IoV dictionaries to map files to (Exp,Run) ranges for calibration: {0}."
                    " Filling dictionary from input file metadata."
                    " If this is slow, set the 'files_to_iovs' attribute of each "
                    "calibration before running.".format(self.calibration.name)))

            files_to_iovs = {}
            for file_path in self.calibration.input_files:
                files_to_iovs[file_path] = get_iov_from_file(file_path)
            self.calibration.files_to_iovs = files_to_iovs
        elif iov_requested and self.calibration.files_to_iovs:
            B2INFO(("Using File to IoV mapping from 'files_to_iovs' attribute for calibration: {0}."
                    "".format(self.calibration.name)))

    def _below_max_iterations(self):
        """
        """
        return self.iteration < self.calibration.max_iterations

    def _increment_iteration(self):
        """
        """
        self.iteration += 1

    def _any_failed_iov(self):
        """
        """
        iteration_results = self._algorithm_results[self.iteration]
        return False

    def _post_process_collector(self):
        """
        Runs a merging job on the collector output ROOT files
        """
        pass

    def _collector_ready(self):
        """
        """
        return self._collector_job.ready()

    def _submit_collector(self):
        """
        """
        self.collector_backend.submit(self._collector_job)

    def _no_require_iteration(self):
        """
        """
        if self._require_iteration() and self._below_max_iterations():
            return False
        elif self._require_iteration() and not self._below_max_iterations():
            B2INFO("Reached maximum number of iterations ({0}), will complete now.".format(self.calibration.max_iterations))
            return True
        elif not self._require_iteration():
            return True

    def _require_iteration(self):
        """
        """
        iteration_called = False
        for alg_name, results in self._algorithm_results[self.iteration].items():
            for result in results:
                if result.result == CalibrationAlgorithm.c_Iterate:
                    iteration_called = True
                    break
            if iteration_called:
                break
        return iteration_called

    def setup_defaults(self):
        """
        Anything that is setup by outside config files by default goes here.
        """
        config_file_path = ROOT.Belle2.FileSystem.findFile('calibration/data/caf.cfg')
        if config_file_path:
            config = configparser.ConfigParser()
            config.read(config_file_path)
        else:
            B2FATAL("Tried to find the default CAF config file but it wasn't there. Is basf2 set up?")
        #: Default location of the generic collector steering file
        self.default_collector_steering_file_path = ROOT.Belle2.FileSystem.findFile('calibration/scripts/caf/run_collector_path.py')

    def _log_new_state(self, **kwargs):
        """
        """
        B2INFO("Calibration {0} moved to state {1}".format(self.calibration.name, kwargs["new_state"].name))

    def dependencies_completed(self):
        """
        Condition function to check that the dependencies of our calibration are in the 'completed' state.
        Technically only need to check explicit dependencies.
        """
        for calibration in self.calibration.dependencies:
            if not calibration.machine.state == "completed":
                return False
        else:
            return True

    def automatic_transition(self):
        """
        Automatically try all transitions out of this state once. Tries fail last.
        """
        possible_transitions = self.get_transitions(self.state)
        for transition in possible_transitions:
            try:
                if transition != "fail":
                    getattr(self, transition)()
                    break
            except ConditionError:
                continue
        else:
            if "fail" in possible_transitions:
                getattr(self, "fail")()
            else:
                raise MachineError(("Failed to automatically transition out of {0} state.".format(self.state)))

    def _make_output_dir(self):
        """
        """
        try:
            os.mkdir(self.root_dir)
        except FileExistsError:
            pass

    def _make_collector_path(self):
        """
        Creates a basf2 path for the correct collector and serializes it in the
        self.output_dir/<calibration_name>/<iteration>/paths directory
        """
        path_output_dir = os.path.join(self.root_dir, str(self.iteration), 'paths')
        # Should work fine as we previously make the other directories
        os.makedirs(path_output_dir)
        # Create empty path and add collector to it
        path = create_path()
        path.add_module(self.calibration.collector)
        # Dump the basf2 path to file
        path_file_name = self.calibration.collector.name() + '.path'
        path_file_name = os.path.join(path_output_dir, path_file_name)
        with open(path_file_name, 'bw') as serialized_path_file:
            pickle.dump(serialize_path(path), serialized_path_file)
        # Return the pickle file path for addition to the input sandbox
        return path_file_name

    def _make_pre_collector_path(self):
        """
        Creates a basf2 path for the collectors setup path (Calibration.pre_collector_path) and serializes it in the
        self.output_dir/<calibration_name>/<iteration>/paths directory
        """
        path_output_dir = os.path.join(self.root_dir, str(self.iteration), 'paths')
        path = self.calibration.pre_collector_path
        # Dump the basf2 path to file
        path_file_name = 'pre_collector.path'
        path_file_name = os.path.join(path_output_dir, path_file_name)
        with open(path_file_name, 'bw') as serialized_path_file:
            pickle.dump(serialize_path(path), serialized_path_file)
        # Return the pickle file path for addition to the input sandbox
        return path_file_name

    def _create_collector_job(self):
        """
        Creates a Job object for the collector for this iteration, ready for submission
        to backend
        """
        iteration_dir = os.path.join(self.root_dir, str(self.iteration))
        job = Job('_'.join([self.calibration.name, 'Collector', 'Iteration', str(self.iteration)]))
        job.output_dir = os.path.join(iteration_dir, 'collector_output')
        job.working_dir = os.path.join(iteration_dir, 'collector_output')
        job.cmd = ['basf2', 'run_collector_path.py']
        job.input_sandbox_files.append(self.default_collector_steering_file_path)
        collector_path_file = self._make_collector_path()
        job.input_sandbox_files.append(collector_path_file)
        if self.calibration.pre_collector_path:
            pre_collector_path_file = self._make_pre_collector_path()
            job.input_sandbox_files.append(pre_collector_path_file)

        # Want to figure out which local databases are required for this job and their paths
        list_dependent_databases = []

        # Here we add the finished databases of previous calibrations that we depend on.
        # We can assume that the databases exist as we can't be here until they have returned
        for dependency in self.calibration.dependencies:
            database_dir = os.path.join(os.getcwd(), dependency.name, 'outputdb')
            B2INFO('Adding local database from {0} for use by {1}'.format(dependency.name, self.calibration.name))
            list_dependent_databases.append((os.path.join(database_dir, 'database.txt'), database_dir))

        # Add previous iteration databases from this calibration
        if self.iteration > 0:
            previous_iteration_dir = os.path.join(self.root_dir, str(self.iteration - 1))
            database_dir = os.path.join(previous_iteration_dir, self.calibration.alg_output_dir, 'outputdb')
            list_dependent_databases.append((os.path.join(database_dir, 'database.txt'), database_dir))
            B2INFO('Adding local database from previous iteration of {}'.format(self.calibration.name))

        # Let's make a directory to store some files later to the collector jobs
        input_data_directory = os.path.join(self.root_dir, str(self.iteration), 'collector_input')
        os.mkdir(input_data_directory)

        # Need to pass setup info to collector which would be tricky as arguments
        # We make a dictionary and pass it in as json
        job_config = {}
        # Pass in the central database global tag
        job_config['global_tag'] = self.calibration._global_tag
        # Add chain of local databases to be used
        job_config['local_database_chain'] = []
        # User defined ones
        job_config['local_database_chain'].extend(self.calibration._local_database_chain)
        # CAF created ones for dependent calibrations and previous iteratioons of this calibration
        job_config['local_database_chain'].extend(list_dependent_databases)

        import json
        job_config_file_path = os.path.join(input_data_directory, 'collector_config.json')
        with open(job_config_file_path, 'w') as job_config_file:
            json.dump(job_config, job_config_file)
        job.input_sandbox_files.append(job_config_file_path)

        # Define the input file list
        input_data_files = self.calibration.input_files
        # Reduce the input data files to only those that overlap with the optionally requested IoV
        if self.iov_to_calibrate:
            iov = self.iov_to_calibrate
            input_data_files = self.files_containing_iov(iov)
        job.input_files = input_data_files
        job.max_files_per_subjob = self.calibration.max_files_per_collector_job
        # In PBS it seems like the environment needs to be set up again
        if isinstance(self.collector_backend, PBS):
            if "BELLE2_LOCAL_DIR" in os.environ:
                job.setup_cmds = [
                    "CAF_TOOLS_LOCATION=" + os.path.join(os.environ["BELLE2_TOOLS"], "setup_belle2"),
                    "CAF_RELEASE_LOCATION=" + os.environ["BELLE2_LOCAL_DIR"],
                    "export CAF_TOOLS_LOCATION",
                    "export CAF_RELEASE_LOCATION",
                    "source $CAF_TOOLS_LOCATION",
                    "pushd $CAF_RELEASE_LOCATION > /dev/null",
                    "setuprel",
                    "popd > /dev/null"
                ]
            else:
                job.basf2_release = os.environ["BELLE2_RELEASE"]
                job.basf2_tools = os.path.join(os.environ["BELLE2_TOOLS"], "setup_belle2")
                job.add_basf2_setup()
        job.backend_args = self.calibration.backend_args
        # Output patterns to be returned from collector job
        job.output_patterns = self.calibration.output_patterns
        B2DEBUG(100, "Collector job for {0}:\n{1}".format(self.calibration.name, str(job)))
        self._collector_job = job

    def _run_algorithms(self):
        """
        Runs the Calibration Algorithms for this calibration machine.

        Will run them sequentially locally (possible benefits to using a
        processing pool for low memory algorithms later on.)
        """
        # Get an instance of the Runner for these algorithms and run it
        algs_runner = self.calibration.algorithms_runner(name=self.calibration.name)
        algs_runner.algorithms = self.calibration.algorithms
        output_database_dir = os.path.join(self.root_dir, str(self.iteration), self.calibration.alg_output_dir, "outputdb")
        B2INFO("Output local database for {} will be stored at {}".format(self.calibration.name, output_database_dir))
        algs_runner.output_database_dir = output_database_dir
        algs_runner.output_dir = os.path.join(self.root_dir, str(self.iteration), self.calibration.alg_output_dir)
        input_files = []
        if self._collector_job.subjobs:
            for subjob in self._collector_job.subjobs.values():
                input_file = os.path.join(subjob.output_dir, "CollectorOutput.root")
                input_files.append(input_file)
        else:
            input_files.append(os.path.join(self._collector_job.output_dir, "CollectorOutput.root"))
        algs_runner.input_files = input_files

        # Add any user defined local database chain for this calibration
        for filename, directory in self.calibration._local_database_chain:
            B2INFO("Adding local database {0} for calibration algorithms of {1}".format(filename, self.calibration.name))
            algs_runner.local_database_chain.append((filename, directory))

        # Here we add the finished databases of previous calibrations that we depend on.
        # We can assume that the databases exist as we can't be here until they have returned
        list_dependent_databases = []
        for dependency in self.calibration.dependencies:
            database_dir = os.path.join(os.getcwd(), dependency.name, 'outputdb')
            B2INFO('Adding local database from {0} for use by {1}'.format(dependency.name, self.calibration.name))
            list_dependent_databases.append((os.path.join(database_dir, 'database.txt'), database_dir))

        # Add previous iteration databases from this calibration
        if self.iteration > 0:
            previous_iteration_dir = os.path.join(self.root_dir, str(self.iteration - 1))
            database_dir = os.path.join(previous_iteration_dir, self.calibration.alg_output_dir, 'outputdb')
            list_dependent_databases.append((os.path.join(database_dir, 'database.txt'), database_dir))
            B2INFO('Adding local database from previous iteration of {}'.format(self.calibration.name))
        algs_runner.dependent_databases = list_dependent_databases
        B2DEBUG(100, "Setting Algortihm Runner {} to use global tag {}".format(algs_runner.name, self.calibration._global_tag))
        algs_runner.global_tag = self.calibration._global_tag
        algs_runner.run(self.iov_to_calibrate, self.iteration)
        self._algorithm_results[self.iteration] = algs_runner.results

    def _prepare_final_db(self):
        """
        Take the last iteration's outputdb and copy it to a more easily findable place.
        """
        database_location = os.path.join(self.root_dir,
                                         str(self.iteration),
                                         self.calibration.alg_output_dir,
                                         'outputdb')
        final_database_location = os.path.join(self.root_dir, 'outputdb')
        shutil.copytree(database_location, final_database_location)


class AlgorithmMachine(Machine):
    """
    A state machine to handle the logic of running the algorithm on the overall runs contained in the data.
    """

    def __init__(self, algorithm=None, initial_state="init"):
        """
        Takes an Algorithm object from the caf framework and defines the transitions.
        """
        #: Default states for the AlgorithmMachine
        self.default_states = [State("init"),
                               State("ready"),
                               State("running_algorithm"),
                               State("completed"),
                               State("failed")]

        super().__init__(self.default_states, initial_state)

        #: Algorithm() object whose state we are modelling
        self.algorithm = algorithm
        #: Global tag for this calibration
        self.global_tag = ""
        #: Collector output files, will contain all files retured by the output patterns
        self.input_files = []
        #: User defined local database chain i.e. if you have localdb's for custom alignment etc
        self.local_database_chain = []
        #: CAF created local databases from previous calibrations that this calibration/algorithm depends on
        self.dependent_databases = []
        #: The algorithm output directory which is mostly used to store the stdout file
        self.output_dir = ""
        #: The output database directory for the localdb that the algorithm will commit to
        self.output_database_dir = ""
        #: IoV_Result object for a single execution, will be reset upon a new execution
        self.result = None

        self.add_transition("setup_algorithm", "init", "ready",
                            before=[self._create_output_dir,
                                    self._setup_database_chain,
                                    self._setup_logging,
                                    self._set_input_data,
                                    self._pre_algorithm])
        self.add_transition("execute_runs", "ready", "running_algorithm",
                            after=self._execute_over_iov)
        self.add_transition("complete", "running_algorithm", "completed")
        self.add_transition("fail", "running_algorithm", "failed")
        self.add_transition("setup_algorithm", "completed", "ready")
        self.add_transition("setup_algorithm", "failed", "ready")

    def _create_output_dir(self, **kwargs):
        """
        Create working/output directory of algorithm
        """
        if not os.path.exists(self.output_dir):
            os.mkdir(self.output_dir)

    def _setup_database_chain(self, **kwargs):
        """
        Apply all databases in the correct order
        """
        # Clean everything out just in case
        reset_database()
        # Fall back to previous databases if no payloads are found
        use_database_chain(True)
        # Use the central database with global tag if requested
        if self.global_tag:
            use_central_database(self.global_tag)
        # Add any user defined local database chain for this calibration
        for filename, directory in self.local_database_chain:
            B2INFO("Using local database {} for {}".format(directory, self.algorithm.name))
            use_local_database(filename, directory)
        # Here we add the finished databases of previous calibrations that we depend on.
        # We can assume that the databases exist as we can't be here until they have returned
        # with OK status.
        for filename, directory in self.dependent_databases:
            B2INFO("Using local database {} created by a dependent calibration for {}".format(directory, self.algorithm.name))
            use_local_database(filename, directory)

        # Create a directory to store the payloads of this algorithm
        if not os.path.exists(self.output_database_dir):
            os.mkdir(self.output_database_dir)
        # add local database to save payloads
        B2INFO("Output local database for {} stored at {}".format(
               self.algorithm.name,
               self.output_database_dir))
        use_local_database(os.path.join(self.output_database_dir, "database.txt"), self.output_database_dir, False, LogLevel.INFO)

    def _setup_logging(self, **kwargs):
        """
        """
        logging.reset()
        set_log_level(LogLevel.INFO)

        # add logfile for output
        logging.add_file(os.path.join(self.output_dir, self.algorithm.name + '_stdout'))

    def _pre_algorithm(self, **kwargs):
        """
        Call the user defined algorithm setup function
        """
        B2INFO("Running Pre-Algorithm function (if exists)")
        if self.algorithm.pre_algorithm:
            # We have to re-pass in the algorithm here because an outside user has created this method.
            # So the method isn't bound to the instance properly.
            self.algorithm.pre_algorithm(self.algorithm.algorithm, kwargs["iteration"])

    def _execute_over_iov(self, **kwargs):
        """
        Does the actual execute of the algorithm on an IoV and records the result
        """
        B2INFO("Running {0} in working directory {1}".format(self.algorithm.name, os.getcwd()))

        runs_to_execute = kwargs["runs"]
        if runs_to_execute:
            # Create nicer overall IoV for this execution
            iov = iov_from_runs(runs_to_execute)
        else:
            # Get IoV from the collected data
            all_runs_collected = runs_from_vector(self.algorithm.algorithm.getRunListFromAllData())
            iov = iov_from_runs(all_runs_collected)
        B2INFO("Performing execution on {0}".format(iov))
        alg_result = self.algorithm.algorithm.execute(runs_to_execute, kwargs["iteration"])
        self.result = IoV_Result(iov, alg_result)

    def _set_input_data(self, **kwargs):
        self.algorithm.data_input(self.input_files)


class MachineError(Exception):
    """
    Base exception class for this module
    """
    pass


class ConditionError(MachineError):
    """
    Exception for when conditions fail during a transition
    """
    pass


class TransitionError(MachineError):
    """
    Exception for when transitions fail
    """
    pass
