##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2 as b2
from caf.state_machines import Machine, State

# We set up a bunch of function callbacks to show the ordering of evaluation
# Notice that we always pass **kwargs so that they have access to anything that
# can be passed in.


def print_msg_before(**kwargs):
    """
    Callbacks on transitions have access to keyword arguments that you can
    specify when calling a transition. But remember that all kwargs are passed
    to ALL transition callbacks. Including both the before and after callbacks
    """
    b2.B2INFO(kwargs["msg_before"])


def print_msg_after(**kwargs):
    """
    Have to have a different kwarg name, otherwise the same argument from the
    'before' callback will be used
    """
    b2.B2INFO(kwargs["msg_after"])


def enter_new_state(**kwargs):
    """
    Enter and exit callbacks have access to the prior and new state objects but NOT
    the transition arguments.
    """
    b2.B2INFO(f"Entering state {kwargs['new_state'].name} from state {kwargs['prior_state'].name}")


def exit_old_state(**kwargs):
    b2.B2INFO(f"Exiting state {kwargs['prior_state'].name} to state {kwargs['new_state'].name}")


def condition_true(**kwargs):
    b2.B2INFO("Evaluating conditions")
    return True


# Now build up a machine with some basic states and transitions between them
m1 = Machine()
m1.add_state(State("start", enter=enter_new_state, exit=exit_old_state))  # Can use a state object to add new ones
m1.add_state("running", enter=enter_new_state, exit=exit_old_state)  # Don't have to use a state object though
m1.add_state("end", enter=enter_new_state, exit=exit_old_state)

m1.add_transition(trigger="begin", source="start", dest="running",
                  conditions=condition_true, before=print_msg_before, after=print_msg_after)
m1.add_transition("finish", "running", "end",
                  conditions=condition_true, before=print_msg_before, after=print_msg_after)

# have to set a start point
m1.initial_state = "start"

# Note that because we didn't specify an initial state when creating the Machine we will get an orphaned default_initial
# state in the graph. Easily removable by hand or by adding an initial state in the Machine() initialisation.
m1.save_graph(filename="machine_graph.dot", graphname="Machine")

# Run through the states passing in arguments to callbacks
b2.B2INFO("Currently in " + m1.state.name + " state.")
m1.begin(msg_before="About to run", msg_after="Now Running")
m1.finish(msg_before="Finishing", msg_after="Finished")
b2.B2INFO("Currently in " + m1.state.name + " state.")
