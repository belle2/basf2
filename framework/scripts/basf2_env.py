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


def serialize_value(module, parameter):
    return serialize_path(parameter.values) if parameter.name == 'path' and module.type() == 'SubEvent' else parameter.values


def deserialize_value(module, parameter_state):
    return deserialize_path(parameter_state['values']) if parameter_state['name'] == 'path' and module.type() == 'SubEvent' else parameter_state['values']


def serialize_module(module):
    return {'name': module.name(), 'type': module.type(), 'flag': module.has_properties(ModulePropFlags.PARALLELPROCESSINGCERTIFIED),
            'parameters': [{'name': parameter.name, 'values': serialize_value(module, parameter)} for parameter in module.available_params() if parameter.setInSteering]}


def deserialize_module(module_state):
    module = fw.register_module(module_state['type'])
    module.set_name(module_state['name'])
    if module_state['flag']:
        module.set_property_flags(ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
    for parameter_state in module_state['parameters']:
        module.param(parameter_state['name'], deserialize_value(module, parameter_state))
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
