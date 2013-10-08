#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
# Demonstrate the Progress module
######################################################

from basf2 import *

# Progress output cannot be suppressed
set_log_level(LogLevel.WARNING)

# TODO: this should be done by Progress
currentInfo = logging.get_info(LogLevel.INFO)
logging.set_info(LogLevel.INFO, currentInfo | LogInfo.TIMESTAMP)

main = create_path()

eventnumbers = register_module('EventNumbers')
eventnumbers.param('evtNumList', [30000])
main.add_module(eventnumbers)

progress = register_module('Progress')
main.add_module(progress)
process(main)
