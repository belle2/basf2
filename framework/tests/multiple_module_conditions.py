##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2

# @cond internal_test


class ReturnTrueFalse(basf2.Module):

    def __init__(self):
        super().__init__()

        self.event_counter = 0

    def event(self):
        self.return_value(int(self.event_counter % 3))
        self.event_counter += 1


class Counter(basf2.Module):

    def __init__(self):
        super().__init__()

        self.counter = 0

    def event(self):
        self.counter += 1


counter_0 = Counter()
counter_1 = Counter()
counter_2 = Counter()
counter_end = Counter()

counter_path_0 = basf2.create_path()
counter_path_0.add_module(counter_0)
counter_path_1 = basf2.create_path()
counter_path_1.add_module(counter_1)
counter_path_2 = basf2.create_path()
counter_path_2.add_module(counter_2)

path = basf2.create_path()
path.add_module("EventInfoSetter", evtNumList=[3])

module_with_conditions = path.add_module(ReturnTrueFalse())
module_with_conditions.if_value("=0", counter_path_0)
module_with_conditions.if_value("=1", counter_path_1)
module_with_conditions.if_value("=2", counter_path_2, basf2.AfterConditionPath.CONTINUE)

path.add_module(counter_end)

basf2.process(path)

assert counter_0.counter == 1
assert counter_1.counter == 1
assert counter_2.counter == 1
assert counter_end.counter == 1

# @endcond
