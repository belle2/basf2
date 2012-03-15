#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
logging.log_level = LogLevel.WARNING

# -------------------------
# here we register modules
# -------------------------

# creates event meta data
# evtmetagen = register_module('EvtMetaGen')
# Load XML parameters
# Module to save the data in DataStore into root file
# evtmetagen = register_module('EvtMetaGen')
progress = register_module('Progress')
paramloader = register_module('Gearbox')
geometry = register_module('Geometry')
geometry.param('Components', ['TOP'])
input = register_module('SimpleInput')
topdigi = register_module('TOPDigi')
param_digi = {'PhotonFraction': 0.3}
topdigi.param(param_digi)
topreco = register_module('TOPReco')
# --------------------------------------
# here we set the parameters of modules
# --------------------------------------

input.param('inputFileName', 'EvtGenOutput.root')

# evtmetagen.param('EvtNumList', [100])
# set number of runs
# evtmetagen.param('RunList', [1])

# set number of events in each run
# evtmetagen.param('EvtNumList', [10000])
# set number of runs
# evtmetagen.param('RunList', [1])

# create path
main = create_path()

# add modules to path
main.add_module(progress)
main.add_module(paramloader)
main.add_module(geometry)
main.add_module(input)
main.add_module(topdigi)
main.add_module(topreco)

process(main)

print statistics
