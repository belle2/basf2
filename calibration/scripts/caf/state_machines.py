from .utils import method_dispatch
from functools import partial


class Machine():
    """
    Base class for a final state machine wrapper.
    Implements the framwork that a more complex machine can inherit from.

    * 'model' attribute represents another class instance that contains information
    that will set up your Machine and allow your machine to make descisions. Currently
    this base class won't use the attribute.

    * 'states' attribute is a list of possible states of the machine. (strings)

    * 'transitions' attribute is a dictionary of trigger name keys, each value of
    which is another dictionary of 'source' states, 'dest' states, and 'condition'
    methods. Currently only one of each allowed. A transition is valid if it goes from
    an allowed state to an allowed state. A condition is optional but must be a callable
    that returns True or False based on some inputs/state of the machine.
    """
    def __init__(self, model=None, states=None, transitions=None, initial_state=None):
        """
        Basic Setup
        """
        self.model = model
        self.states = []
        self.state = initial_state
        self.transitions = {}
        if states:
            self.states = states
        if transitions:
            self.transitions = transitions

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
        self.transitions[trigger] = transition_dict

    def add_state(self, state):
        """
        Adds a single state to the list of possible ones.
        Should be a unique string.
        """
        self.states.append(state)

    def __getattr__(self, name):
        """
        Allows us to create a new method for each trigger on the fly.
        If there is no trigger name in the machine to match, then th normal
        AttributeError is called.
        """
        try:
            transition_dict = self.transitions[name]
        except KeyError:
            raise AttributeError("{0} does not exist in transitions".format(name))
        else:
            return partial(self._trigger, name, transition_dict)

    def _trigger(self, name, transition_dict):
        """
        Runs the transition logic
        """
        if self.state == transition_dict["source"]:
            condition = transition_dict["condition"]
            if condition():
                self.state = transition_dict["dest"]

    def save_graph(self, filename, graphname):
        with open(filename, "w") as dotfile:
            dotfile.write("digraph "+graphname+" {\n")
            for state in self.states:
                dotfile.write('"'+state+'" [shape=ellipse, color=black]\n')
            for transition, transition_dict in self.transitions.items():
                dotfile.write('"'+transition_dict["source"]+'" -> "'+transition_dict["dest"]+'" [label="'+transition+'"]\n')
            dotfile.write("}\n")


class CalibrationMachine(Machine):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.states.extend(["init",
                            "running_collector",
                            "collector_failed",
                            "collector_completed",
                            "running_algorithm",
                            "algorithm_failed",
                            "algorithm_completed",
                            "completed"])

        self.add_transition("submit_collector", "init", "running_collector")
        self.add_transition("fail_collector", "running_collector", "collector_failed")
        self.add_transition("complete_collector", "running_collector", "collector_completed")
        self.add_transition("run_algorithm", "collector_completed", "running_algorithm")
        self.add_transition("complete_algorithm", "running_algorithm", "algorithm_completed")
        self.add_transition("fail_algorithm", "running_algorithm", "algorithm_failed")
        self.add_transition("iterate", "algorithm_completed", "init")
        self.add_transition("finish", "algorithm_completed", "completed")
