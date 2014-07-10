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


def module_getstate(module):
    return {'name': module.name(), 'parameters': [{'name': parameter.name, 'values': parameter.values} for parameter in module.available_params()]}


def module_setstate(module, module_state):
    module.set_name(module_state['name'])
    for parameter_state in module_state['parameters']:
        module.param(parameter_state['name'], parameter_state['values'])
    return module


Module.__getstate_manages_dict = False
Module.__getstate__ = module_getstate
Module.__setstate__ = module_setstate


def path_getstate(path):
    return {'modules': [module_getstate(module) for module in path.modules()]}


def path_setstate(path, path_state):
    for module_state in path_state['modules']:
        module = module_setstate(Module(), module_state)
        path.add_module(module)
    return path


Path.__getstate_manages_dict = False
Path.__getstate__ = path_getstate
Path.__setstate__ = path_setstate

# -----------------------------------------------
#             Print output
# -----------------------------------------------
print '>>> basf2 Python environment set'
print '>>> Framework object created: fw'
