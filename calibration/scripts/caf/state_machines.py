from functools import partial
from collections import defaultdict

from .utils import method_dispatch


class Machine():
    """
    Base class for a final state machine wrapper.
    Implements the framwork that a more complex machine can inherit from.

    * 'states' attribute is a list of possible states of the machine. (strings)

    * 'transitions' attribute is a dictionary of trigger name keys, each value of
    which is another dictionary of 'source' states, 'dest' states, and 'condition'
    methods. Currently only one of each allowed. A transition is valid if it goes from
    an allowed state to an allowed state. A condition is optional but must be a callable
    that returns True or False based on some inputs/state of the machine.
    """
    def __init__(self):
        """
        Basic Setup
        """
        self.states = set()
        self.transitions = defaultdict(list)

    @staticmethod
    def default_condition():
        """
        Method to always return True.
        """
        return True

    def add_transition(self, trigger, source, dest, condition=None):
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
        if condition:
            transition_dict["condition"] = condition
        else:
            transition_dict["condition"] = Machine.default_condition
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
            return partial(self._trigger, transition_dicts)

    def _trigger(self, transition_dicts):
        """
        Runs the transition logic
        """
        for transition in transition_dicts:
            source, dest, condition = transition["source"], transition["dest"], transition["condition"]
            if self.state == source:
                if condition():
                    self.state = dest
                    break
        else:
            print("Implement Trigger Error Here")

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
    def __init__(self, calibration, initial_state="init"):
        super().__init__()
        self.states.update(["init",
                            "running_collector",
                            "collector_failed",
                            "collector_completed",
                            "running_algorithm",
                            "algorithm_failed",
                            "algorithm_completed",
                            "completed"])

        self.add_transition("submit_collector", "init", "running_collector")
        self.add_transition("fail", "running_collector", "collector_failed")
        self.add_transition("complete", "running_collector", "collector_completed")
        self.add_transition("run_algorithm", "collector_completed", "running_algorithm")
        self.add_transition("complete", "running_algorithm", "algorithm_completed")
        self.add_transition("fail", "running_algorithm", "algorithm_failed")
        self.add_transition("iterate", "algorithm_completed", "init")
        self.add_transition("finish", "algorithm_completed", "completed")

        self.state = initial_state


class MachineError(Exception):
    """
    Base exception class for this module
    """
    pass
