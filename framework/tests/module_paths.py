#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import random
import basf2  # also test non-polluting import
from ROOT import Belle2

basf2.set_random_seed("something important")


class SelectOddEvents(basf2.Module):

    """For events with an odd event number, set module return value to False"""

    def event(self):
        """reimplementation of Module::event()."""

        evtmetadata = Belle2.PyStoreObj('EventMetaData')
        if not evtmetadata:
            basf2.B2ERROR('No EventMetaData found')
        else:
            event = evtmetadata.obj().getEvent()
            basf2.B2INFO('Setting return value to ' + str(event % 2 == 0))
            self.return_value(event % 2 == 0)

    def terminate(self):
        """reimplementation of Module::terminate()."""

        basf2.B2INFO('terminating SelectOddEvents')


class ReturnFalse(basf2.Module):

    """Always return false"""

    def event(self):
        """reimplementation of Module::event()."""
        self.return_value(False)


class PrintName(basf2.Module):

    """Print name in event"""

    def __init__(self, name):
        """constructor."""

        super(PrintName, self).__init__()
        self.set_name(name)

    def event(self):
        """reimplementation of Module::event()."""
        # error level to check that this doesn't prevent execution
        basf2.B2ERROR("In module " + self.name())

main = basf2.create_path()

# register necessary modules

emptypath = basf2.create_path()
emptypath.add_path(basf2.create_path())
main.add_path(emptypath)

# generate three events
main.add_module('EventInfoSetter', expList=[0, 1], runList=[1, 2], evtNumList=[2, 1])

anotherpath = basf2.create_path()
main.add_path(anotherpath)  # added here, filled later
main.add_module('PrintCollections')

subsubpath = basf2.create_path()
subsubpath.add_module('Progress')
subsubpath.add_path(emptypath)

# fill anotherpath now
module_with_condition = SelectOddEvents()
anotherpath.add_module(module_with_condition)
anotherpath.add_module('EventInfoPrinter')

# check printing of paths, should be:
# [] -> eventinfosetter -> [SelectOddEvents -> eventinfo]
# -> printcollections
print(main)

# when the module returns true/1 (even events), we jump to Progress instead:
# [] -> eventinfosetter -> [SelectOddEvents -> [Progress -> []]]
module_with_condition.if_true(subsubpath)
# this is equivalent to:
# module_with_condition.if_value('<1', subsubpath)

# test continuing after conditional path
returnfalse1 = ReturnFalse()
returnfalse1_condition_path = basf2.create_path()
returnfalse1_condition_path.add_module(PrintName('ReturnFalse1Condition'))
returnfalse1.if_false(returnfalse1_condition_path, basf2.AfterConditionPath.CONTINUE)
main.add_module(returnfalse1)

# test more complicated conditions (in this case, it is never met)
returnfalse2 = ReturnFalse()
returnfalse2_condition_path = basf2.create_path()
returnfalse2_condition_path.add_module(PrintName('ReturnFalse2Condition'))
returnfalse2.if_true(returnfalse2_condition_path)

main.add_module(PrintName("final"))

basf2.process(main)

basf2.B2INFO("second process() call follows...")
basf2.process(main)

print(main)
