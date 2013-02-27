#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
set_log_level(LogLevel.ERROR)

# Events
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('EvtNumList', [1000])  # we want to process 1000 events
evtmetagen.param('RunList', [1])  # from run number 1
evtmetagen.param('ExpList', [1])  # and experiment number 1

# Generator
evtgeninput = register_module('EvtGenInput')
# evtgeninput.param('userDECFile',
# os.path.join(basf2datadir,'generators/belle/MIX.DEC'))

# Gearbox
gearbox = register_module('Gearbox')

# Progress
progress = register_module('Progress')

# Output
output = register_module('RootOutput')
output.param('outputFileName', 'EvtGenOutput.root')

main = create_path()
main.add_module(evtmetagen)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(evtgeninput)
main.add_module(output)

# Process events
process(main)

# Print call statistics
print statistics

