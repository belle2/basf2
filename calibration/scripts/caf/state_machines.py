from functools import partial
from collections import defaultdict

import ROOT
from .utils import method_dispatch
from .utils import decode_json_string


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
    def __init__(self, states=set(), initial_state=""):
        """
        Basic Setup
        """
        if states:
            self.states = set(states)
            if initial_state:
                self.state = initial_state
        else:
            self.states = set()
        self.transitions = defaultdict(list)

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
        transition_dict["source"] = source
        transition_dict["dest"] = dest
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

    def add_state(self, state):
        """
        Adds a single state to the list of possible ones.
        Should be a unique string.
        """
        self.states.add(state)

    def __getattr__(self, name, **kwargs):
        """
        Allows us to create a new method for each trigger on the fly.
        If there is no trigger name in the machine to match, then the normal
        AttributeError is called.
        """
        try:
            transition_dicts = self.transitions[name]
        except KeyError:
            raise AttributeError("{0} does not exist in transitions".format(name))
        else:
            return partial(self._trigger, name, transition_dicts, **kwargs)

    def _trigger(self, name, transition_dicts, **kwargs):
        """
        Runs the transition logic. Callbacks are evaluated in the order:
        conditions -> before -> <new state set here> -> after
        """
        for transition in transition_dicts:
            source, dest, conditions, before_callbacks, after_callbacks = (transition["source"],
                                                                           transition["dest"],
                                                                           transition["conditions"],
                                                                           transition["before"],
                                                                           transition["after"])
            if self.state == source:
                # Returns True only if every condition returns True when called
                if all(map(lambda condition: self._callback(condition, **kwargs), conditions)):
                    for before_func in before_callbacks:
                        self._callback(before_func, **kwargs)
                    self.state = dest
                    for after_func in after_callbacks:
                        self._callback(after_func, **kwargs)
                    break
                else:
                    raise ConditionError(("Transition '{0}' called for but one or more conditions "
                                          "evaluated False".format(name)))
        else:
            raise TransitionError(("Transition '{0}' called but there isn't one defined "
                                   "for the current state '{1}'".format(name, self.state)))

    @staticmethod
    def _callback(func, **kwargs):
        """
        Calls a condition/before/after.. function using arguments passed (or not)
        """
        return func(**kwargs)

    @property
    def state(self):
        return self._state

    @state.setter
    def state(self, value):
        if value in self.states:
            self._state = value
        else:
            raise MachineError("Attempted to set state to '{0}' which not in the 'states' attribute!".format(value))

    def save_graph(self, filename, graphname):
        with open(filename, "w") as dotfile:
            dotfile.write("digraph "+graphname+" {\n")
            for state in self.states:
                dotfile.write('"'+state+'" [shape=ellipse, color=black]\n')
            for trigger, transition_dicts in self.transitions.items():
                for transition in transition_dicts:
                    dotfile.write('"'+transition["source"]+'" -> "'+transition["dest"]+'" [label="'+trigger+'"]\n')
            dotfile.write("}\n")


class CalibrationMachine(Machine):
    """
    A state machine to handle Calibration objects and the flow of
    processing for them.
    """
    default_states = {"init",
                      "running_collector",
                      "collector_failed",
                      "collector_completed",
                      "running_algorithm",
                      "algorithm_failed",
                      "algorithm_completed",
                      "completed"}

    perfect_order = ["submit_collector",
                     "complete",
                     "run_algorithm",
                     "complete",
                     "finish"]

    def __init__(self, calibration, initial_state="init", iteration=0):
        """
        Takes a Calibration object from the caf framework and lets you
        set the initial state
        """
        super().__init__(CalibrationMachine.default_states, initial_state)
        self.setup_defaults()

        self.add_transition("submit_collector", "init", "running_collector", conditions=self.dependencies_completed)
        self.add_transition("fail", "running_collector", "collector_failed")
        self.add_transition("complete", "running_collector", "collector_completed")
        self.add_transition("run_algorithm", "collector_completed", "running_algorithm")
        self.add_transition("complete", "running_algorithm", "algorithm_completed")
        self.add_transition("fail", "running_algorithm", "algorithm_failed")
        self.add_transition("iterate", "algorithm_completed", "init")
        self.add_transition("finish", "algorithm_completed", "completed")

        #: Calibration object whose state we are modelling
        self.calibration = calibration
        # Monkey Patching for the win!
        #: Allows calibration object to hold a refernce to the machine controlling it
        self.calibration.machine = self
        #: Which iteration step are we in
        self.iteration = iteration
        #: Maximum allowed iterations
        self.max_iterations = self.default_max_iterations

    @classmethod
    def setup_defaults(self):
        """
        Anything that is setup by outside config files by default goes here.
        """
        config_file_path = ROOT.Belle2.FileSystem.findFile('calibration/data/caf.cfg')
        if config_file_path:
            import configparser
            config = configparser.ConfigParser()
            config.read(config_file_path)
        else:
            B2FATAL("Tried to find the default CAF config file but it wasn't there. Is basf2 set up?")
        CalibrationMachine.default_max_iterations = decode_json_string(config['CAF_DEFAULTS']['MaxIterations'])

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
