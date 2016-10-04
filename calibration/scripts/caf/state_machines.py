from functools import partial
from collections import defaultdict

import configparser
from threading import Thread, Lock, RLock
from time import sleep
import pickle

from basf2 import *
import ROOT
from .utils import method_dispatch
from .utils import merge_local_databases
from .utils import decode_json_string
from .backends import Job


class State():
    """
    Basic State object that can take enter and exit state methods and records
    the state of a machine.

    You should assign the self.on_enter or self.on_exit attributes to callback functions
    or lists of them, if you need them.
    """

    def __init__(self, name, enter=None, exit=None):
        self.name = name
        self.on_enter = enter
        self.on_exit = exit

    @property
    def on_enter(self):
        """
        Getter for on_enter attribute
        """
        return self._on_enter

    @on_enter.setter
    def on_enter(self, callbacks):
        """
        Setter for on_enter attribute
        """
        self._on_enter = []
        if callbacks:
            self._add_callbacks(callbacks, self._on_enter)

    @property
    def on_exit(self):
        """
        Getter for on_exit attribute
        """
        return self._on_exit

    @on_exit.setter
    def on_exit(self, callbacks):
        """
        Setter for on_exit attribute
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
        return self.name

    def __repr__(self):
        return "State(name={0})".format(self.name)

    def __eq__(self, other):
        if isinstance(other, str):
            return self.name == other
        else:
            return self.name == other.name

    def __hash__(self):
        return hash(self.name)


class Machine():
    """
    Base class for a final state machine wrapper.
    Implements the framwork that a more complex machine can inherit from.

    * 'states' attribute is a list of possible states of the machine. (strings)

    * 'transitions' attribute is a dictionary of trigger name keys, each value of
    which is another dictionary of 'source' states, 'dest' states, and 'conditions'
    methods. 'conditions' should be a list of callables or a single one.
    A transition is valid if it goes from an allowed state to an allowed state.
    Conditions are optional but must be a callable that returns True or False based
    on some state of the machine. They cannot have input arguments currently.

    * Every condition/before/after callback function MUST take **kwargs as the only
    argument (except 'self' if it's a clas method). This is because it's basically
    impossible to determine which arguments to pass to which functions for a transition.
    Therefore this machine just enforces that every function should simply take **kwargs
    and use the dictionary of arguments (even if it doesn't need any arguments).

    This also means that if you call a trigger with arguments e.g. machine.walk(speed=5)
    you MUST use the keyword arguments rather than positional ones e.g. machine.walk(5)
    """
    def __init__(self, states=None, initial_state="default_initial"):
        """
        Basic Setup of states and initial_state
        """
        self.states = {}
        if states:
            for state in states:
                self.add_state(state)
        if initial_state != "default_initial":
            self.initial_state = initial_state
        else:
            self.add_state(initial_state)
            self._initial_state = State(initial_state)

        self._state = self.initial_state
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
        return self._initial_state

    @initial_state.setter
    def initial_state(self, state):
        if state in self.states.keys():
            self._initial_state = self.states[state]
            self._state = self.states[state]
        else:
            raise KeyError("Attempted to set state to '{0}' which is not in the 'states' attribute!".format(state))

    @property
    def state(self):
        return self._state

    @state.setter
    def state(self, state):
        """
        Setter for a state. Will call the exit method of the current state and enter method of the
        new one. to get around the behaviour e.g. for setting initial states, either use the initial_state
        property or directly set the _state attribute itself (at your own risk!)
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
        possible_transitions = []
        for transition, transition_dicts in self.transitions.items():
            for transition_dict in transition_dicts:
                if transition_dict["source"] == source:
                    possible_transitions.append(transition)
        return possible_transitions

    def get_transition_dict(self, state, transition):
        transition_dicts = self.transitions[transition]
        for transition_dict in transition_dicts:
            if transition_dict["source"] == state:
                return transition_dict
        else:
            raise KeyError("No transition from state {0} with the name {1}".format(state, transition))

    def save_graph(self, filename, graphname):
        with open(filename, "w") as dotfile:
            dotfile.write("digraph "+graphname+" {\n")
            for state in self.states.keys():
                dotfile.write('"'+state+'" [shape=ellipse, color=black]\n')
            for trigger, transition_dicts in self.transitions.items():
                for transition in transition_dicts:
                    dotfile.write('"'+transition["source"].name+'" -> "'+transition["dest"].name+'" [label="'+trigger+'"]\n')
            dotfile.write("}\n")


class CalibrationMachine(Machine):
    """
    A state machine to handle Calibration objects and the flow of
    processing for them.
    """

    def __init__(self, calibration, initial_state="init", iteration=0):
        """
        Takes a Calibration object from the caf framework and lets you
        set the initial state
        """
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

        self.add_transition("submit_collector", "init", "running_collector",
                            conditions=self.dependencies_completed,
                            before=[self._make_output_dir,
                                    self._create_collector_job])
        self.add_transition("fail", "running_collector", "collector_failed")
        self.add_transition("complete", "running_collector", "collector_completed")
        self.add_transition("run_algorithms", "collector_completed", "running_algorithms")
        self.add_transition("complete", "running_algorithms", "algorithms_completed",
                            after=self.automatic_transition)
        self.add_transition("fail", "running_algorithms", "algorithms_failed")
        self.add_transition("iterate", "algorithms_completed", "init",
                            conditions=self.require_iteration)
        self.add_transition("finish", "algorithms_completed", "completed")

        #: Calibration object whose state we are modelling
        self.calibration = calibration
        # Monkey Patching for the win!
        #: Allows calibration object to hold a refernce to the machine controlling it
        self.calibration.machine = self
        #: Which iteration step are we in
        self.iteration = iteration
        #: Maximum allowed iterations
        self.max_iterations = self.default_max_iterations

    def require_iteration(self):
        return False

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
        self.default_max_iterations = decode_json_string(config['CAF_DEFAULTS']['MaxIterations'])
        self.default_collector_steering_file_path = ROOT.Belle2.FileSystem.findFile('calibration/scripts/caf/run_collector_path.py')

    def _log_new_state(self, **kwargs):
        B2INFO("Calibration {0} moved to state {1}".format(self.calibration.name, kwargs["new_state"].name))

    def dependencies_completed(self, **kwargs):
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
        possible_transitions = self.get_transitions(self.state)
        for transition in possible_transitions:
            try:
                if transition != "fail":
                    getattr(self, transition)()
                    break
            except ConditionError:
                continue
        else:
            raise RunnerError(("Failed to automatically transition out of {0} state."
                               " No conditions succeeded.".format(self.state)))

    def _make_output_dir(self):
        os.mkdir(self.calibration.name)

    def _make_collector_path(self):
        """
        Creates a basf2 path for the correct collector and serializes it in the
        self.output_dir/<calibration_name>/<iteration>/paths directory
        """
        path_output_dir = os.path.join(os.getcwd(), self.calibration.name, str(self.iteration), 'paths')
        # Should work fine as we previously make the other directories
        os.makedirs(path_output_dir)
        # Create empty path and add collector to it
        path = create_path()
        path.add_module(self.calibration.collector)
        # Dump the basf2 path to file
        path_file_name = self.calibration.collector.name()+'.path'
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
        path_output_dir = os.path.join(os.getcwd(), self.calibration.name, str(self.iteration), 'paths')
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
        job = Job('_'.join([self.calibration.name, 'Collector', 'Iteration', str(self.iteration)]))
        job.output_dir = os.path.join(os.getcwd(), self.calibration.name, str(self.iteration), 'output')
        job.working_dir = os.path.join(os.getcwd(), self.calibration.name, str(self.iteration), 'input')
        job.cmd = ['basf2', 'run_collector_path.py']
        job.input_sandbox_files.append(self.default_collector_steering_file_path)
        collector_path_file = self._make_collector_path()
        job.input_sandbox_files.append(collector_path_file)
        if self.calibration.pre_collector_path:
            pre_collector_path_file = self._make_pre_collector_path()
            job.input_sandbox_files.append(pre_collector_path_file)

        # Want to figure out which local databases are required for this job and their paths
        list_dependent_databases = []
        # Add previous iteration databases from this calibration
        if self.iteration > 0:
            for algorithm in self.calibration.algorithms:
                algorithm_name = algorithm.algorithm.Class_Name().replace('Belle2::', '')
                database_dir = os.path.join(os.getcwd(), self.calibration.name, str(self.iteration-1), 'output', 'outputdb')
                list_dependent_databases.append(database_dir)
                B2INFO('Adding local database from previous iteration of {0} for use by {1}'.format(algorithm_name,
                       self.calibration.name))

        # Here we add the finished databases of previous calibrations that we depend on.
        # We can assume that the databases exist as we can't be here until they have returned
        # (THIS MAY CHANGE!)
        for dependency in self.calibration.dependencies:
            database_dir = os.path.join(os.getcwd(), dependency.name, 'outputdb')
            B2INFO('Adding local database from {0} for use by {1}'.format(database_dir, self.calibration.name))
            list_dependent_databases.append(database_dir)

        # Set the location where we will store the merged set of databases.
        dependent_database_dir = os.path.join(os.getcwd(), self.calibration.name, str(self.iteration), 'inputdb')
        # Merge all of the local databases that are required for this calibration into a single directory
        if list_dependent_databases:
            merge_local_databases(list_dependent_databases, dependent_database_dir)
        job.input_sandbox_files.append(dependent_database_dir)
        # Define the input file list and output patterns to be returned from collector job
        job.input_files = self.calibration.input_files
        job.output_patterns = self.calibration.output_patterns

        return job


class CalibrationRunner(Thread):
    """
    Runs a CalibrationMachine in a Thread. Will process from intial state
    to the final state.
    """

    def __init__(self, machine, heartbeat=None):
        super().__init__()
        self.machine = machine
        self.moves = ["submit_collector", "complete", "run_algorithms"]
        self.setup_defaults()
        if heartbeat:
            self.heartbeat = heartbeat

    def run(self):
        """
        Will be run in a new Thread by calling the start() method
        """
        while self.machine.state != "completed":
            for trigger in self.moves:
                try:
                    if trigger in self.machine.get_transitions(self.machine.state):
                        getattr(self.machine, trigger)()
                    sleep(self.heartbeat)  # Only sleeps if transition completed
                except ConditionError:
                    continue

            if self.machine.state == "failed":
                break

    def setup_defaults(self):
        """
        Anything that is setup by outside config files by default goes here.
        """
        # Not certain if configparser requires that we lock here, but to be on the safe side we do
        config_file_path = ROOT.Belle2.FileSystem.findFile('calibration/data/caf.cfg')
        if config_file_path:
            config = configparser.ConfigParser()
            config.read(config_file_path)
        else:
            B2FATAL("Tried to find the default CAF config file but it wasn't there. Is basf2 set up?")
        self.heartbeat = decode_json_string(config['CAF_DEFAULTS']['Heartbeat'])


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


class RunnerError(Exception):
    """
    Base exception class for Machine Runner
    """
    pass
