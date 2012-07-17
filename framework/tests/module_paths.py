#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import random
from basf2 import *

# register necessary modules
evtmetagen = register_module('EvtMetaGen')
# generate three events
evtmetagen.param('ExpList', [0, 1])
evtmetagen.param('RunList', [1, 2])
evtmetagen.param('EvtNumList', [2, 1])

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

#fill anotherpath now
anotherpath.add_module(evtmetainfo)
anotherpath.add_path(subsubpath)

#conditionalpath = create_path()
#evtmetainfo.condition("<1", conditionalpath)

# check printing of paths, should be:
# [] -> evtmetagen -> [evtmetainfo -> [Progress -> []]] -> printcollections
print main

process(main)

print main
