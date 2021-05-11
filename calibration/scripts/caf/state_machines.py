#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2

from functools import partial
from collections import defaultdict

import pickle
import glob
import shutil
import time
from pathlib import Path
import os
import json

from basf2 import create_path
from basf2 import B2DEBUG, B2ERROR, B2INFO, B2WARNING
from basf2 import conditions as b2conditions
from basf2.pickle_path import serialize_path

from ROOT.Belle2 import CalibrationAlgorithm

from caf.utils import create_directories
from caf.utils import method_dispatch
from caf.utils import iov_from_runs
from caf.utils import IoV_Result
from caf.utils import get_iov_from_file
from caf.backends import Job
from caf.runners import AlgorithmsRunner


class State():
    """
    Basic State object that can take enter and exit state methods and records
    the state of a machine.

    You should assign the self.on_enter or self.on_exit attributes to callback functions
    or lists of them, if you need them.
    """

    def __init__(self, name, enter=None, exit=None):
        """
        Initialise State with a name and optional lists of callbacks.
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
            B2ERROR(f"Something other than a function (callable) passed into State {self.name}.")

    @_add_callbacks.register(tuple)
    @_add_callbacks.register(list)
    def _(self, callbacks, attribute):
        """
        Alternate method for lists and tuples of function objects.
        """
        if callbacks:
            for function in callbacks:
                if callable(function):
                    attribute.append(function)
                else:
                    B2ERROR(f"Something other than a function (callable) passed into State {self.name}.")

    def __str__(self):
        """
        """
        return self.name

    def __repr__(self):
        """
        """
        return f"State(name={self.name})"

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
        Basic Setup of states and initial_state.
        """
        #: Valid states for this machine
        self.states = {}
        if states:
            for state in states:
                self.add_state(state)
        if initial_state != "default_initial":
            #: Pointless docstring since it's a property
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
                B2WARNING(f"You asked to add a state {state} but it was already in the machine states.")
        else:
            B2WARNING(f"You asked to add a state {state} but it wasn't a State or str object")

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
            raise KeyError(f"Attempted to set state to '{state}' which is not in the 'states' attribute!")

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
            raise MachineError(f"Attempted to set state to '{state}' which not in the 'states' attribute!")

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
            raise AttributeError(f"{name} does not exist in transitions for state {self.state}.")
        transition_dict = self.get_transition_dict(self.state, name)
        return partial(self._trigger, name, transition_dict, **kwargs)

    def _trigger(self, transition_name, transition_dict, **kwargs):
        """
        Runs the transition logic. Callbacks are evaluated in the order:
        conditions -> before -> <new state set here> -> after.
        """
        dest, conditions, before_callbacks, after_callbacks = (
            transition_dict["dest"],
            transition_dict["conditions"],
            transition_dict["before"],
            transition_dict["after"]
        )
        # Returns True only if every condition returns True when called
        if all(map(lambda condition: self._callback(condition, **kwargs), conditions)):
            for before_func in before_callbacks:
                self._callback(before_func, **kwargs)
            #: Current State of machine
            self.state = dest
            for after_func in after_callbacks:
                self._callback(after_func, **kwargs)
        else:
            raise ConditionError((f"Transition '{transition_name}' called for but one or more conditions "
                                  "evaluated False"))

    @staticmethod
    def _callback(func, **kwargs):
        """
        Calls a condition/before/after.. function using arguments passed (or not).
        """
        return func(**kwargs)

    def get_transitions(self, source):
        """
        Returns allowed transitions from a given state.
        """
        possible_transitions = []
        for transition, transition_dicts in self.transitions.items():
            for transition_dict in transition_dicts:
                if transition_dict["source"] == source:
                    possible_transitions.append(transition)
        return possible_transitions

    def get_transition_dict(self, state, transition):
        """
        Returns the transition dictionary for a state and transition out of it.
        """
        transition_dicts = self.transitions[transition]
        for transition_dict in transition_dicts:
            if transition_dict["source"] == state:
                return transition_dict
        else:
            raise KeyError(f"No transition from state {state} with the name {transition}.")

    def save_graph(self, filename, graphname):
        """
        Does a simple dot file creation to visualise states and transiitons.
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

    collector_input_dir = 'collector_input'
    collector_output_dir = 'collector_output'
    algorithm_output_dir = 'algorithm_output'

    def __init__(self, calibration, iov_to_calibrate=None, initial_state="init", iteration=0):
        """
        Takes a Calibration object from the caf framework and lets you
        set the initial state.
        """
        #: States that are defaults to the `CalibrationMachine` (could override later)
        self.default_states = [State("init", enter=[self._update_cal_state,
                                                    self._log_new_state]),
                               State("running_collector", enter=[self._update_cal_state,
                                                                 self._log_new_state]),
                               State("collector_failed", enter=[self._update_cal_state,
                                                                self._log_new_state]),
                               State("collector_completed", enter=[self._update_cal_state,
                                                                   self._log_new_state]),
                               State("running_algorithms", enter=[self._update_cal_state,
                                                                  self._log_new_state]),
                               State("algorithms_failed", enter=[self._update_cal_state,
                                                                 self._log_new_state]),
                               State("algorithms_completed", enter=[self._update_cal_state,
                                                                    self._log_new_state]),
                               State("completed", enter=[self._update_cal_state,
                                                         self._log_new_state]),
                               State("failed", enter=[self._update_cal_state,
                                                      self._log_new_state])
                               ]

        super().__init__(self.default_states, initial_state)

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
        #: Final state of the algorithm runner for the current iteration
        self._runner_final_state = None
        #: IoV to be executed, currently will loop over all runs in IoV
        self.iov_to_calibrate = iov_to_calibrate
        #: root directory for this Calibration
        self.root_dir = Path(os.getcwd(), calibration.name)

        #: Times of various useful updates to the collector job e.g. start, elapsed, last update
        #: Used to periodically call update_status on the collector job
        #: and find out an overall number of jobs remaining + estimated remaining time
        self._collector_timing = {}

        #: The collector jobs used for submission
        self._collector_jobs = {}

        self.add_transition("submit_collector", "init", "running_collector",
                            conditions=self.dependencies_completed,
                            before=[self._make_output_dir,
                                    self._resolve_file_paths,
                                    self._build_iov_dicts,
                                    self._create_collector_jobs,
                                    self._submit_collections,
                                    self._dump_job_config])
        self.add_transition("fail", "running_collector", "collector_failed",
                            conditions=self._collection_failed)
        self.add_transition("complete", "running_collector", "collector_completed",
                            conditions=self._collection_completed)
        self.add_transition("run_algorithms", "collector_completed", "running_algorithms",
                            before=self._check_valid_collector_output,
                            after=[self._run_algorithms,
                                   self.automatic_transition])
        self.add_transition("complete", "running_algorithms", "algorithms_completed",
                            after=self.automatic_transition,
                            conditions=self._runner_not_failed)
        self.add_transition("fail", "running_algorithms", "algorithms_failed",
                            conditions=self._runner_failed)
        self.add_transition("iterate", "algorithms_completed", "init",
                            conditions=[self._require_iteration,
                                        self._below_max_iterations],
                            after=self._increment_iteration)
        self.add_transition("finish", "algorithms_completed", "completed",
                            conditions=self._no_require_iteration,
                            before=self._prepare_final_db)
        self.add_transition("fail_fully", "algorithms_failed", "failed")
        self.add_transition("fail_fully", "collector_failed", "failed")

    def _update_cal_state(self, **kwargs):
        self.calibration.state = str(kwargs["new_state"])

    def files_containing_iov(self, file_paths, files_to_iovs, iov):
        """
        Lookup function that returns all files from the file_paths that
        overlap with this IoV.
        """
        # Files that contain an Exp,Run range that overlaps with given IoV
        overlapping_files = set()

        for file_path, file_iov in files_to_iovs.items():
            if file_iov.overlaps(iov) and (file_path in file_paths):
                overlapping_files.add(file_path)
        return overlapping_files

    def _dump_job_config(self):
        """
        Dumps the `Job` object for the collections to JSON files so that it's configuration can be recovered
        later in case of failure.
        """
        # Wait for jobs (+subjobs) to be submitted so that all information is filled. Since the parent CAF object asynchronously
        # submits the jobs this might need to wait a while.
        while any(map(lambda j: j.status == "init", self._collector_jobs.values())):
            B2DEBUG(29, "Some Collector Jobs still in 'init' state. Waiting...")
            time.sleep(5)

        for collection_name, job in self._collector_jobs.items():
            collector_job_output_file_name = self.calibration.collections[collection_name].job_config
            output_file = self.root_dir.joinpath(str(self.iteration), self.collector_input_dir,
                                                 collection_name, collector_job_output_file_name)
            job.dump_to_json(output_file)

    def _recover_collector_jobs(self):
        """
        Recovers the `Job` object for the collector from a JSON file in the event that we are starting from a reset.
        """
        for collection_name, collection in self.calibration.collections.items():
            output_file = self.root_dir.joinpath(str(self.iteration),
                                                 self.collector_input_dir,
                                                 collection_name,
                                                 collection.job_config)
            self._collector_jobs[collection_name] = Job.from_json(output_file)

    def _iov_requested(self):
        """
        """
        if self.iov_to_calibrate:
            B2DEBUG(20, f"Overall IoV {self.iov_to_calibrate} requested for calibration: {self.calibration.name}.")
            return True
        else:
            B2DEBUG(20, f"No overall IoV requested for calibration: {self.calibration.name}.")
            return False

    def _resolve_file_paths(self):
        """
        """
        pass

    def _build_iov_dicts(self):
        """
        Build IoV file dictionary for each collection if required.
        """
        iov_requested = self._iov_requested()
        if iov_requested or self.calibration.ignored_runs:
            for coll_name, collection in self.calibration.collections.items():
                if not collection.files_to_iovs:
                    B2INFO("Creating IoV dictionaries to map files to (Exp,Run) ranges for"
                           f" Calibration '{self.calibration.name} and Collection '{coll_name}'."
                           " Filling dictionary from input file metadata."
                           " If this is slow, set the 'files_to_iovs' attribute of each Collection before running.")

                    files_to_iovs = {}
                    for file_path in collection.input_files:
                        files_to_iovs[file_path] = get_iov_from_file(file_path)
                    collection.files_to_iovs = files_to_iovs
                else:
                    B2INFO("Using File to IoV mapping from 'files_to_iovs' attribute for "
                           f"Calibration '{self.calibration.name}' and Collection '{coll_name}'.")
        else:
            B2INFO("No File to IoV mapping required.")

    def _below_max_iterations(self):
        """
        """
        return self.iteration < self.calibration.max_iterations

    def _increment_iteration(self):
        """
        """
        self.iteration += 1
        self.calibration.iteration = self.iteration

    def _collection_completed(self):
        """
        Did all the collections succeed?
        """
        B2DEBUG(29, "Checking for failed collector job.")
        if self._collector_jobs_ready():
            return all([job.status == "completed" for job in self._collector_jobs.values()])

    def _collection_failed(self):
        """
        Did any of the collections fail?
        """
        B2DEBUG(29, "Checking for failed collector job.")
        if self._collector_jobs_ready():
            return any([job.status == "failed" for job in self._collector_jobs.values()])

    def _runner_not_failed(self):
        """
        Returns:
            bool: If AlgorithmsRunner succeeded return True.
        """
        return not self._runner_failed()

    def _runner_failed(self):
        """
        Returns:
            bool: If AlgorithmsRunner failed return True.
        """
        if self._runner_final_state == AlgorithmsRunner.FAILED:
            return True
        else:
            return False

    def _collector_jobs_ready(self):
        """
        """
        since_last_update = time.time() - self._collector_timing["last_update"]
        if since_last_update > self.calibration.collector_full_update_interval:
            B2INFO("Updating full collector job statuses.")
            for job in self._collector_jobs.values():
                job.update_status()
                self._collector_timing["last_update"] = time.time()
                if job.subjobs:
                    num_completed = sum((subjob.status in subjob.exit_statuses) for subjob in job.subjobs.values())
                    total_subjobs = len(job.subjobs)
                    B2INFO(f"{num_completed}/{total_subjobs} Collector SubJobs finished in"
                           f" Calibration {self.calibration.name} Job {job.name}.")
        return all([job.ready() for job in self._collector_jobs.values()])

    def _submit_collections(self):
        """
        """
        self.calibration.jobs_to_submit.extend(list(self._collector_jobs.values()))
        self._collector_timing["start"] = time.time()
        self._collector_timing["last_update"] = time.time()

    def _no_require_iteration(self):
        """
        """
        if self._require_iteration() and self._below_max_iterations():
            return False
        elif self._require_iteration() and not self._below_max_iterations():
            B2INFO(f"Reached maximum number of iterations ({self.calibration.max_iterations}), will complete now.")
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

    def _log_new_state(self, **kwargs):
        """
        """
        B2INFO(f"Calibration Machine {self.calibration.name} moved to state {kwargs['new_state'].name}.")

    def dependencies_completed(self):
        """
        Condition function to check that the dependencies of our calibration are in the 'completed' state.
        Technically only need to check explicit dependencies.
        """
        for calibration in self.calibration.dependencies:
            if not calibration.state == calibration.end_state:
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
                raise MachineError(f"Failed to automatically transition out of {self.state} state.")

    def _make_output_dir(self):
        """
        Creates the overall root directory of the Calibration. Will not overwrite if it already exists.
        Also creates s
        """
        create_directories(self.root_dir, overwrite=False)

    def _make_collector_path(self, name, collection):
        """
        Creates a basf2 path for the correct collector and serializes it in the
        self.output_dir/<calibration_name>/<iteration>/paths directory
        """
        path_output_dir = self.root_dir.joinpath(str(self.iteration), self.collector_input_dir, name)
        # Automatically overwrite any previous directory
        create_directories(path_output_dir)
        path_file_name = collection.collector.name() + '.path'
        path_file_name = path_output_dir / path_file_name
        # Create empty path and add collector to it
        coll_path = create_path()
        coll_path.add_module(collection.collector)
        # Dump the basf2 path to file
        with open(path_file_name, 'bw') as serialized_path_file:
            pickle.dump(serialize_path(coll_path), serialized_path_file)
        # Return the pickle file path for addition to the input sandbox
        return str(path_file_name.absolute())

    def _make_pre_collector_path(self, name, collection):
        """
        Creates a basf2 path for the collectors setup path (Collection.pre_collector_path) and serializes it in the
        self.output_dir/<calibration_name>/<iteration>/<colector_output>/<name> directory.
        """
        path_output_dir = self.root_dir.joinpath(str(self.iteration), self.collector_input_dir, name)
        coll_path = collection.pre_collector_path
        path_file_name = 'pre_collector.path'
        path_file_name = os.path.join(path_output_dir, path_file_name)
        # Dump the basf2 path to file
        with open(path_file_name, 'bw') as serialized_path_file:
            pickle.dump(serialize_path(coll_path), serialized_path_file)
        # Return the pickle file path for addition to the input sandbox
        return path_file_name

    def _create_collector_jobs(self):
        """
        Creates a Job object for the collections of this iteration, ready for submission
        to backend.
        """
        for collection_name, collection in self.calibration.collections.items():
            iteration_dir = self.root_dir.joinpath(str(self.iteration))
            job = Job('_'.join([self.calibration.name, collection_name, 'Iteration', str(self.iteration)]))
            job.output_dir = iteration_dir.joinpath(self.collector_output_dir, collection_name)
            job.working_dir = iteration_dir.joinpath(self.collector_output_dir, collection_name)
            # Remove previous failed attempt to avoid problems
            if job.output_dir.exists():
                B2INFO(f"Previous output directory for {self.calibration.name} collector {collection_name} exists."
                       f"Deleting {job.output_dir} before re-submitting.")
                shutil.rmtree(job.output_dir)
            job.cmd = collection.job_cmd
            job.append_current_basf2_setup_cmds()
            job.input_sandbox_files.append(collection.job_script)
            collector_path_file = Path(self._make_collector_path(collection_name, collection))
            job.input_sandbox_files.append(collector_path_file)
            if collection.pre_collector_path:
                pre_collector_path_file = Path(self._make_pre_collector_path(collection_name, collection))
                job.input_sandbox_files.append(pre_collector_path_file)

            # Want to figure out which local databases are required for this job and their paths
            list_dependent_databases = []

            # Here we add the finished databases of previous calibrations that we depend on.
            # We can assume that the databases exist as we can't be here until they have returned
            for dependency in self.calibration.dependencies:
                database_dir = os.path.join(os.getcwd(), dependency.name, 'outputdb')
                B2INFO(f"Adding local database from {dependency.name} for use by {self.calibration.name}.")
                list_dependent_databases.append((os.path.join(database_dir, 'database.txt'), database_dir))

            # Add previous iteration databases from this calibration
            if self.iteration > 0:
                previous_iteration_dir = self.root_dir.joinpath(str(self.iteration - 1))
                database_dir = os.path.join(previous_iteration_dir, self.calibration.alg_output_dir, 'outputdb')
                list_dependent_databases.append((os.path.join(database_dir, 'database.txt'), database_dir))
                B2INFO(f"Adding local database from previous iteration of {self.calibration.name}.")

            # Let's use a directory to store some files later for input to the collector jobs. Should already exist from
            # collector path
            input_data_directory = self.root_dir.joinpath(str(self.iteration), self.collector_input_dir, collection_name)

            # Need to pass setup info to collector which would be tricky as arguments
            # We make a dictionary and pass it in as json
            job_config = {}
            # Apply the user-set Calibration database chain to the base of the overall chain.
            json_db_chain = []
            for database in collection.database_chain:
                if database.db_type == 'local':
                    json_db_chain.append(('local', (database.filepath.as_posix(), database.payload_dir.as_posix())))
                elif database.db_type == 'central':
                    json_db_chain.append(('central', database.global_tag))
                else:
                    raise ValueError(f"Unknown database type {database.db_type}.")
            # CAF created ones for dependent calibrations and previous iterations of this calibration
            for database in list_dependent_databases:
                json_db_chain.append(('local', database))
            job_config['database_chain'] = json_db_chain

            job_config_file_path = input_data_directory.joinpath('collector_config.json').absolute()
            with open(job_config_file_path, 'w') as job_config_file:
                json.dump(job_config, job_config_file, indent=2)
            job.input_sandbox_files.append(job_config_file_path)

            # Define the input files
            input_data_files = set(collection.input_files)
            # Reduce the input data files to only those that overlap with the optional requested IoV
            if self.iov_to_calibrate:
                input_data_files = self.files_containing_iov(input_data_files,
                                                             collection.files_to_iovs,
                                                             self.iov_to_calibrate)
            # Remove any files that ONLY contain runs from our optional ignored_runs list
            files_to_ignore = set()
            for exprun in self.calibration.ignored_runs:
                for input_file in input_data_files:
                    file_iov = self.calibration.files_to_iovs[input_file]
                    if file_iov == exprun.make_iov():
                        B2INFO(f"You have asked for {exprun} to be ignored for Calibration '{self.calibration.name}'. "
                               f"Therefore the input file '{input_file}' from Collection '{collection_name}' "
                               "is being removed from input files list.")
                        files_to_ignore.add(input_file)
            input_data_files.difference_update(files_to_ignore)

            if not input_data_files:
                raise MachineError(f"No valid input files for Calibration '{self.calibration.name}' "
                                   f" and Collection '{collection_name}'.")
            job.input_files = list(input_data_files)

            job.splitter = collection.splitter
            job.backend_args = collection.backend_args
            # Output patterns to be returned from collector job
            job.output_patterns = collection.output_patterns
            B2DEBUG(20, f"Collector job for {self.calibration.name}:{collection_name}:\n{job}")
            self._collector_jobs[collection_name] = job

    def _check_valid_collector_output(self):
        B2INFO("Checking that Collector output exists for all colector jobs "
               f"using {self.calibration.name}.output_patterns.")
        if not self._collector_jobs:
            B2INFO("We're restarting so we'll recreate the collector Job object.")
            self._recover_collector_jobs()

        for job in self._collector_jobs.values():
            if not job.subjobs:
                output_files = []
                for pattern in job.output_patterns:
                    output_files.extend(glob.glob(os.path.join(job.output_dir, pattern)))
                if not output_files:
                    raise MachineError("No output files from Collector Job")
            else:
                for subjob in job.subjobs.values():
                    output_files = []
                    for pattern in subjob.output_patterns:
                        output_files.extend(glob.glob(os.path.join(subjob.output_dir, pattern)))
                    if not output_files:
                        raise MachineError(f"No output files from Collector {subjob}")

    def _run_algorithms(self):
        """
        Runs the Calibration Algorithms for this calibration machine.

        Will run them sequentially locally (possible benefits to using a
        processing pool for low memory algorithms later on.)
        """
        # Get an instance of the Runner for these algorithms and run it
        algs_runner = self.calibration.algorithms_runner(name=self.calibration.name)
        algs_runner.algorithms = self.calibration.algorithms
        algorithm_output_dir = self.root_dir.joinpath(str(self.iteration), self.calibration.alg_output_dir)
        output_database_dir = algorithm_output_dir.joinpath("outputdb")
        # Remove it, if we failed previously, to start clean
        if algorithm_output_dir.exists():
            B2INFO(f"Output directory for {self.calibration.name} already exists from a previous CAF attempt. "
                   f"Deleting and recreating {algorithm_output_dir}.")
        create_directories(algorithm_output_dir)
        B2INFO(f"Output local database for {self.calibration.name} will be stored at {output_database_dir}.")
        algs_runner.output_database_dir = output_database_dir
        algs_runner.output_dir = self.root_dir.joinpath(str(self.iteration), self.calibration.alg_output_dir)
        input_files = []

        for job in self._collector_jobs.values():
            if job.subjobs:
                for subjob in job.subjobs.values():
                    for pattern in subjob.output_patterns:
                        input_files.extend(glob.glob(os.path.join(subjob.output_dir, pattern)))
            else:
                for pattern in job.output_patterns:
                    input_files.extend(glob.glob(os.path.join(job.output_dir, pattern)))

        algs_runner.input_files = input_files

        # Add any user defined database chain for this calibration
        algs_runner.database_chain = self.calibration.database_chain

        # Here we add the finished databases of previous calibrations that we depend on.
        # We can assume that the databases exist as we can't be here until they have returned
        list_dependent_databases = []
        for dependency in self.calibration.dependencies:
            database_dir = os.path.join(os.getcwd(), dependency.name, 'outputdb')
            B2INFO(f"Adding local database from {dependency.name} for use by {self.calibration.name}.")
            list_dependent_databases.append((os.path.join(database_dir, 'database.txt'), database_dir))

        # Add previous iteration databases from this calibration
        if self.iteration > 0:
            previous_iteration_dir = self.root_dir.joinpath(str(self.iteration - 1))
            database_dir = os.path.join(previous_iteration_dir, self.calibration.alg_output_dir, 'outputdb')
            list_dependent_databases.append((os.path.join(database_dir, 'database.txt'), database_dir))
            B2INFO(f"Adding local database from previous iteration of {self.calibration.name}.")
        algs_runner.dependent_databases = list_dependent_databases

        algs_runner.ignored_runs = self.calibration.ignored_runs

        try:
            algs_runner.run(self.iov_to_calibrate, self.iteration)
        except Exception as err:
            print(err)
            # We directly set the state without triggering the transition because normally we fail based on checking the algorithm
            # results. But here we had an actual exception so we just force into failure instead.
            self._state = State("algorithms_failed")
        self._algorithm_results[self.iteration] = algs_runner.results
        self._runner_final_state = algs_runner.final_state

    def _prepare_final_db(self):
        """
        Take the last iteration's outputdb and copy it to a more easily findable place.
        """
        database_location = self.root_dir.joinpath(str(self.iteration),
                                                   self.calibration.alg_output_dir,
                                                   'outputdb')
        final_database_location = self.root_dir.joinpath('outputdb')
        if final_database_location.exists():
            B2INFO(f"Removing previous final output database for {self.calibration.name} before copying new one.")
            shutil.rmtree(final_database_location)
        shutil.copytree(database_location, final_database_location)


class AlgorithmMachine(Machine):
    """
    A state machine to handle the logic of running the algorithm on the overall runs contained in the data.
    """

    #: Required attributes that must exist before the machine can run properly.
    #: Some are allowed be values that return False whe tested e.g. "" or []
    required_attrs = ["algorithm",
                      "dependent_databases",
                      "database_chain",
                      "output_dir",
                      "output_database_dir",
                      "input_files"
                      ]

    #: Attributes that must have a value that returns True when tested.
    required_true_attrs = ["algorithm",
                           "output_dir",
                           "output_database_dir",
                           "input_files"
                           ]

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
        #: Collector output files, will contain all files retured by the output patterns
        self.input_files = []
        #: CAF created local databases from previous calibrations that this calibration/algorithm depends on
        self.dependent_databases = []
        #: Assigned database chain to the overall Calibration object, or to the 'default' Collection.
        #: Database chains for manually created Collections have no effect here.
        self.database_chain = []
        #: The algorithm output directory which is mostly used to store the stdout file
        self.output_dir = ""
        #: The output database directory for the localdb that the algorithm will commit to
        self.output_database_dir = ""
        #: IoV_Result object for a single execution, will be reset upon a new execution
        self.result = None

        self.add_transition("setup_algorithm", "init", "ready",
                            before=[self._setup_logging,
                                    self._change_working_dir,
                                    self._setup_database_chain,
                                    self._set_input_data,
                                    self._pre_algorithm])
        self.add_transition("execute_runs", "ready", "running_algorithm",
                            after=self._execute_over_iov)
        self.add_transition("complete", "running_algorithm", "completed")
        self.add_transition("fail", "running_algorithm", "failed")
        self.add_transition("fail", "ready", "failed")
        self.add_transition("setup_algorithm", "completed", "ready")
        self.add_transition("setup_algorithm", "failed", "ready")

    def setup_from_dict(self, params):
        """
        Parameters:
            params (dict): Dictionary containing values to be assigned to the machine's attributes of the same name.
        """
        for attribute_name, value in params.items():
            setattr(self, attribute_name, value)

    def is_valid(self):
        """
        Returns:
            bool: Whether or not this machine has been set up correctly with all its necessary attributes.
        """
        B2INFO("Checking validity of current setup of AlgorithmMachine for {}.".format(self.algorithm.name))
        # Check if we're somehow missing a required attribute (should be impossible since they get initialised in init)
        for attribute_name in self.required_attrs:
            if not hasattr(self, attribute_name):
                B2ERROR(f"AlgorithmMachine attribute {attribute_name} doesn't exist.")
                return False
        # Check if any attributes that need actual values haven't been set or were empty
        for attribute_name in self.required_true_attrs:
            if not getattr(self, attribute_name):
                B2ERROR(f"AlgorithmMachine attribute {attribute_name} returned False.")
                return False
        return True

    def _create_output_dir(self, **kwargs):
        """
        Create working/output directory of algorithm. Any old directory is overwritten.
        """
        create_directories(Path(self.output_dir), overwrite=True)

    def _setup_database_chain(self, **kwargs):
        """
        Apply all databases in the correct order.
        """
        # We deliberately override the normal database ordering because we don't want input files GTs to affect
        # the processing. Only explicit GTs and intermediate local DBs made by the CAF should be added here.
        b2conditions.reset()
        b2conditions.override_globaltags()

        # Apply all the databases in order, starting with the user-set chain for this Calibration
        for database in self.database_chain:
            if database.db_type == 'local':
                B2INFO(f"Adding Local Database {database.filepath.as_posix()} to head of chain of local databases, "
                       f"for {self.algorithm.name}.")
                b2conditions.prepend_testing_payloads(database.filepath.as_posix())
            elif database.db_type == 'central':
                B2INFO(f"Adding Central database tag {database.global_tag} to head of GT chain, "
                       f"for {self.algorithm.name}.")
                b2conditions.prepend_globaltag(database.global_tag)
            else:
                raise ValueError(f"Unknown database type {database.db_type}.")
        # Here we add the finished databases of previous calibrations that we depend on.
        # We can assume that the databases exist as we can't be here until they have returned
        # with OK status.
        for filename, directory in self.dependent_databases:
            B2INFO((f"Adding Local Database {filename} to head of chain of local databases created by"
                    f" a dependent calibration, for {self.algorithm.name}."))
            b2conditions.prepend_testing_payloads(filename)

        # Create a directory to store the payloads of this algorithm
        create_directories(Path(self.output_database_dir), overwrite=False)

        # add local database to save payloads
        B2INFO(f"Output local database for {self.algorithm.name} stored at {self.output_database_dir}.")
        # Things have changed. We now need to do the expert settings to create a database directly.
        # LocalDB is readonly without this but we don't need 'use_local_database' during writing.
        b2conditions.expert_settings(save_payloads=str(self.output_database_dir.joinpath("database.txt")))

    def _setup_logging(self, **kwargs):
        """
        """
        # add logfile for output
        log_file = os.path.join(self.output_dir, self.algorithm.name + '_stdout')
        B2INFO(f"Output log file at {log_file}.")
        basf2.reset_log()
        basf2.set_log_level(basf2.LogLevel.INFO)
        basf2.log_to_file(log_file)

    def _change_working_dir(self, **kwargs):
        """
        """
        B2INFO(f"Changing current working directory to {self.output_dir}.")
        os.chdir(self.output_dir)

    def _pre_algorithm(self, **kwargs):
        """
        Call the user defined algorithm setup function.
        """
        B2INFO("Running Pre-Algorithm function (if exists)")
        if self.algorithm.pre_algorithm:
            # We have to re-pass in the algorithm here because an outside user has created this method.
            # So the method isn't bound to the instance properly.
            self.algorithm.pre_algorithm(self.algorithm.algorithm, kwargs["iteration"])

    def _execute_over_iov(self, **kwargs):
        """
        Does the actual execute of the algorithm on an IoV and records the result.
        """
        B2INFO(f"Running {self.algorithm.name} in working directory {os.getcwd()}.")

        runs_to_execute = kwargs["runs"]
        iov = kwargs["apply_iov"]
        iteration = kwargs["iteration"]
        if not iov:
            iov = iov_from_runs(runs_to_execute)
        B2INFO(f"Execution will use {iov} for labelling payloads by default.")
        alg_result = self.algorithm.algorithm.execute(runs_to_execute, iteration, iov._cpp_iov)
        self.result = IoV_Result(iov, alg_result)

    def _set_input_data(self, **kwargs):
        self.algorithm.data_input(self.input_files)


class MachineError(Exception):
    """
    Base exception class for this module.
    """


class ConditionError(MachineError):
    """
    Exception for when conditions fail during a transition.
    """


class TransitionError(MachineError):
    """
    Exception for when transitions fail.
    """
