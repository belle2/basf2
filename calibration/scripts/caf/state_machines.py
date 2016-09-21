from functools import partial
from collections import defaultdict

from .utils import method_dispatch


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
    def default_condition():
        """
        Method to always return True.
        """
        return True

    def add_transition(self, trigger, source, dest, conditions=None):
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
        self.transitions[trigger].append(transition_dict)

    def add_state(self, state):
        """
        Adds a single state to the list of possible ones.
        Should be a unique string.
        """
        self.states.add(state)

    def __getattr__(self, name):
        """
        Allows us to create a new method for each trigger on the fly.
        If there is no trigger name in the machine to match, then th normal
        AttributeError is called.
        """
        try:
            transition_dicts = self.transitions[name]
        except KeyError:
            raise AttributeError("{0} does not exist in transitions".format(name))
        else:
            return partial(self._trigger, name, transition_dicts)

    def _trigger(self, name, transition_dicts):
        """
        Runs the transition logic
        """
        for transition in transition_dicts:
            source, dest, conditions = transition["source"], transition["dest"], transition["conditions"]
            if self.state == source:
                # Returns True only if every condition returns True when called
                if all(map(lambda x: x(), conditions)):
                    self.state = dest
                    break
                else:
                    raise ConditionError(("Transition '{0}' called for but one or more conditions "
                                          "evaluated False".format(name)))
        else:
            raise MachineError(("Transition '{0}' called but there isn't one defined "
                                "for the current state '{1}'".format(name, self.state)))

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

    def __init__(self, calibration, initial_state="init"):
        """
        Takes a Calibration object from the caf framework and lets you
        set the initial state
        """
        super().__init__(CalibrationMachine.default_states, initial_state)

        self.add_transition("submit_collector", "init", "running_collector", conditions=self.dependencies_completed)
        self.add_transition("fail", "running_collector", "collector_failed")
        self.add_transition("complete", "running_collector", "collector_completed")
        self.add_transition("run_algorithm", "collector_completed", "running_algorithm")
        self.add_transition("complete", "running_algorithm", "algorithm_completed")
        self.add_transition("fail", "running_algorithm", "algorithm_failed")
        self.add_transition("iterate", "algorithm_completed", "init")
        self.add_transition("finish", "algorithm_completed", "completed")

        self.calibration = calibration
        self.calibration.state = self.state

    def dependencies_completed(self):
        for calibration in self.calibration.dependencies:
            if not calibration.state == "completed":
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
