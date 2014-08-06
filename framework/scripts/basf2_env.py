#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import signal
from pybasf2 import *

# -----------------------------------------------
#             Set basf2 information
# -----------------------------------------------
basf2label = 'BASF2 (Belle Analysis Framework 2)'
basf2version = os.environ.get('BELLE2_RELEASE', 'unknown')
basf2copyright = 'Copyright(C) 2010-2014  Belle II Collaboration'

# -----------------------------------------------
#               Prepare basf2
# -----------------------------------------------

# Reset the signal handler to allow the framework execution
# to be stopped with Ctrl-c (Python installs own handler)
signal.signal(signal.SIGINT, signal.SIG_DFL)


# -----------------------------------------------
#       Create default framework object
# -----------------------------------------------

# Create default framework, also initialises environment
fw = Framework()


def serialize_module(module):
    return {'name': module.name(), 'type': module.type(), 'parameters': [{'name': parameter.name, 'values': parameter.values} for parameter in module.available_params()]}


def deserialize_module(module_state):
    module = fw.register_module(module_state['type'])
    module.set_name(module_state['name'])
    for parameter_state in module_state['parameters']:
        module.param(parameter_state['name'], parameter_state['values'])
    return module


def serialize_path(path):
    return {'modules': [serialize_module(module) for module in path.modules()]}


def deserialize_path(path_state):
    path = fw.create_path()
    for module_state in path_state['modules']:
        module = deserialize_module(module_state)
        path.add_module(module)
    return path


# -----------------------------------------------
#             Print output
# -----------------------------------------------
print '>>> basf2 Python environment set'
print '>>> Framework object created: fw'
