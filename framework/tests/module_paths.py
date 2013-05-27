#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import random
from basf2 import *
from ROOT import Belle2


class SelectOddEvents(Module):

    """For events with an odd event number, set module return value to False"""

    def __init__(self):
        """constructor."""

        super(SelectOddEvents, self).__init__()

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


# register necessary modules
evtmetagen = register_module('EvtMetaGen')
# generate three events
evtmetagen.param('expList', [0, 1])
evtmetagen.param('runList', [1, 2])
evtmetagen.param('evtNumList', [2, 1])

evtmetainfo = register_module('EvtMetaInfo')
progress = register_module('Progress')
printcollections = register_module('PrintCollections')

# create main path
main = create_path()

emptypath = create_path()
main.add_path(emptypath)

main.add_module(evtmetagen)

anotherpath = create_path()
main.add_path(anotherpath)  # added here, filled later
main.add_module(printcollections)

subsubpath = create_path()
subsubpath.add_module(progress)
subsubpath.add_path(emptypath)

# fill anotherpath now
module_with_condition = SelectOddEvents()
anotherpath.add_module(module_with_condition)
anotherpath.add_module(evtmetainfo)
anotherpath.add_path(subsubpath)

# check printing of paths, should be:
# [] -> evtmetagen -> [SelectOddEvents -> evtmetainfo -> [Progress -> []]]
# -> printcollections
print main

# when the module returns false/0 (odd events), we jump to Progress instead:
# [] -> evtmetagen -> [SelectOddEvents -> [Progress -> []]]
module_with_condition.condition(subsubpath)
# this is equivalent to:
# module_with_condition.condition('<1', subsubpath)

process(main)

print main
