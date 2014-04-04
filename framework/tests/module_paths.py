#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import random
from basf2 import *
from ROOT import Belle2


class SelectOddEvents(Module):
    """For events with an odd event number, set module return value to False"""

    def event(self):
        """reimplementation of Module::event()."""

        evtmetadata = Belle2.PyStoreObj('EventMetaData')
        if not evtmetadata:
            B2ERROR('No EventMetaData found')
        else:
            event = evtmetadata.obj().getEvent()
            B2INFO('Setting return value to ' + str(event % 2 == 0))
            self.return_value(event % 2 == 0)

    def terminate(self):
        """reimplementation of Module::terminate()."""

        B2INFO('terminating SelectOddEvents')


class ReturnFalse(Module):
    """Always return false"""

    def event(self):
        """reimplementation of Module::event()."""
        self.return_value(False)


class PrintName(Module):
    """Print name in event"""

    def __init__(self, name):
        """constructor."""

        super(PrintName, self).__init__()
        self.set_name(name)

    def event(self):
        """reimplementation of Module::event()."""
        B2INFO("In module " + self.name())


# register necessary modules
eventinfosetter = register_module('EventInfoSetter')
# generate three events
eventinfosetter.param('expList', [0, 1])
eventinfosetter.param('runList', [1, 2])
eventinfosetter.param('evtNumList', [2, 1])

eventinfo = register_module('EventInfoPrinter')
progress = register_module('Progress')
printcollections = register_module('PrintCollections')

# create main path
main = create_path()

emptypath = create_path()
emptypath.add_path(create_path())
main.add_path(emptypath)

main.add_module(eventinfosetter)

anotherpath = create_path()
main.add_path(anotherpath)  # added here, filled later
main.add_module(printcollections)

subsubpath = create_path()
subsubpath.add_module(progress)
subsubpath.add_path(emptypath)

# fill anotherpath now
module_with_condition = SelectOddEvents()
anotherpath.add_module(module_with_condition)
anotherpath.add_module(eventinfo)
anotherpath.add_path(subsubpath)

# check printing of paths, should be:
# [] -> eventinfosetter -> [SelectOddEvents -> eventinfo -> [Progress -> []]]
# -> printcollections
print main

# when the module returns false/0 (odd events), we jump to Progress instead:
# [] -> eventinfosetter -> [SelectOddEvents -> [Progress -> []]]
module_with_condition.if_false(subsubpath)
# this is equivalent to:
# module_with_condition.if_value('<1', subsubpath)

# test continuing after conditional path
returnfalse1 = ReturnFalse()
returnfalse1_condition_path = create_path()
returnfalse1_condition_path.add_module(PrintName('ReturnFalse1Condition'))
returnfalse1.if_false(returnfalse1_condition_path, AfterConditionPath.CONTINUE)
main.add_module(returnfalse1)

# test more complicated conditions (in this case, it is never met)
returnfalse2 = ReturnFalse()
returnfalse2_condition_path = create_path()
returnfalse2_condition_path.add_module(PrintName('ReturnFalse2Condition'))
returnfalse2.if_true(returnfalse2_condition_path)

main.add_module(PrintName("final"))

process(main)

print main
