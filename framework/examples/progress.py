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

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [30000])
main.add_module(eventinfosetter)

progress = register_module('Progress')
main.add_module(progress)
process(main)
