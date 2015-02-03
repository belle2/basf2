#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import signal
from pybasf2 import *

# workaround for possible hang with PyROOT on SL5
# see https://belle2.cc.kek.jp/redmine/issues/1236
# note: platform.libc_ver() is _really_ broken, so i'm checking the
# version via ldd (ships with libc)
import subprocess
ldd_ver = subprocess.check_output(['ldd', '--version'])
sl5_libc_string = "ldd (GNU libc) 2.5"
if sl5_libc_string in ldd_ver:
    try:
        from ROOT import PyConfig
        PyConfig.StartGuiThread = False
    except:
        print "PyRoot not set up, this will cause problems."


# -----------------------------------------------
#             Set basf2 information
# -----------------------------------------------
basf2label = 'BASF2 (Belle Analysis Framework 2)'
basf2version = os.environ.get('BELLE2_RELEASE', 'unknown')
basf2copyright = 'Copyright(C) 2010-2015  Belle II Collaboration'

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


def _add_module(self, module, logLevel=None, debugLevel=None, **kwargs):
    """
    Add given module (either object or name) at the end of this path.
    All unknown arguments are passed as module parameters.

    >>> path = create_path()
    >>> path.add_module('EventInfoSetter', evtNumList=100, logLevel=LogLevel.ERROR)
    <pybasf2.Module at 0x1e356e0>

    >>> path = create_path()
    >>> eventinfosetter = register_module('EventInfoSetter')
    >>> path.add_module(eventinfosetter)
    <pybasf2.Module at 0x2289de8>
    """
    if isinstance(module, Module):
        module = module
    else:
        module = fw.register_module(module)

    if kwargs:
        module.param(kwargs)
    if logLevel is not None:
        module.set_log_level(logLevel)
    if debugLevel is not None:
        module.set_debug_level(debugLevel)
    self._add_module_object(module)
    return module

Path.add_module = _add_module


def serialize_value(module, parameter):
    if parameter.name == 'path' and module.type() == 'SubEvent':
        return serialize_path(parameter.values)
    else:
        return parameter.values


def deserialize_value(module, parameter_state):
    if parameter_state['name'] == 'path' and module.type() == 'SubEvent':
        return deserialize_path(parameter_state['values'])
    else:
        return parameter_state['values']


def serialize_module(module):
    if module.type() == '':
        raise RuntimeError("Module '%s' doesn't have a type! Note that --dump-path cannot work properly with basf2 modules written in Python." % (module.name()))
    if module.has_condition():
        raise RuntimeError("Module '%s' has a condition set. --dump-path currently does not support this" % (module.name()))
    return {
        'name': module.name(),
        'type': module.type(),
        'flag': module.has_properties(ModulePropFlags.PARALLELPROCESSINGCERTIFIED),
        'parameters': [{'name': parameter.name, 'values': serialize_value(module, parameter)} for parameter in module.available_params() if parameter.setInSteering or module.type() == 'SubEvent']}


def deserialize_module(module_state):
    module = fw.register_module(module_state['type'])
    module.set_name(module_state['name'])
    if module_state['flag']:
        # for some modules, this flag might be changed from the default
        module.set_property_flags(ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
    for parameter_state in module_state['parameters']:
        module.param(parameter_state['name'],
                     deserialize_value(module, parameter_state))
    return module


def serialize_path(path):
    return {'modules': [serialize_module(module) for module in path.modules()]}


def deserialize_path(path_state):
    path = fw.create_path()
    for module_state in path_state['modules']:
        module = deserialize_module(module_state)
        path.add_module(module)
    return path


def get_path_from_file(path_filename):
    import pickle
    return deserialize_path(pickle.load(open(path_filename, 'r')))


def write_path_to_file(path, filename):
    import pickle
    pickle.dump(serialize_path(path), open(filename, 'w'))


# -----------------------------------------------
#             Print output
# -----------------------------------------------
print '>>> basf2 Python environment set'
print '>>> Framework object created: fw'
