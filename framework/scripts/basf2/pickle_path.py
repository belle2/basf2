#!/usr/bin/env python3

"""
basf2.pickle_path - Functions necessary to pickle and unpickle a Path
=====================================================================

This module contains all the functiones necessary to serialize and deserialize
a full path with all modules, parameters, sub paths, conditions and so on. This
can be used in conjunction with ``basf2 --dump-path`` and ``basf2
--execute-path`` to save a full configuration to file and execute it later.
"""

import pybasf2
import pickle as _pickle
import os as _os
import sys as _sys


def serialize_value(module, parameter):
    """Serialize a single basf2 module parameter"""
    if parameter.name == 'path' and module.type() == 'SubEvent':
        return serialize_path(parameter.values)
    else:
        return parameter.values


def deserialize_value(module, parameter_state):
    """Deserialize a single basf2 module paramater"""
    if parameter_state['name'] == 'path' and module.type() == 'SubEvent':
        return deserialize_path(parameter_state['values'])
    else:
        return parameter_state['values']


def serialize_conditions(module):
    """Serialize all conditions attached to a basf2 module"""
    condition_list = []

    for condition in module.get_all_conditions():
        condition_list.append({'value': condition.get_value(),
                               'operator': int(condition.get_operator()),
                               'path': serialize_path(condition.get_path()),
                               'option': int(condition.get_after_path())})

    return condition_list


def deserialize_conditions(module, module_state):
    """Deserialize all conditions for a given basf2 module"""
    conditions = module_state['condition']
    for cond in conditions:
        module.if_value(str(pybasf2.ConditionOperator.values[cond['operator']]) + str(cond['value']),
                        deserialize_path(cond['path']), pybasf2.AfterConditionPath.values[cond['option']])


def serialize_module(module):
    """Serialize a basf2 module into a python dictionary. Doesn't work for python modules"""
    if module.type() == '' or module.type() == 'PyModule':
        raise RuntimeError("Module '%s' doesn't have a type or is a Python module! Note that --dump-path cannot work"
                           "properly with basf2 modules written in Python." % (module.name()))
    return {
        'name': module.name(),
        'type': module.type(),
        'flag': module.has_properties(pybasf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED),
        'parameters': [{'name': parameter.name, 'values': serialize_value(module, parameter)}
                       for parameter in module.available_params()
                       if parameter.setInSteering or module.type() == 'SubEvent'],
        'condition': serialize_conditions(module) if module.has_condition() else None}


def deserialize_module(module_state):
    """Deserialize a basf2 module from a python dictionary"""
    module = pybasf2._register_module(module_state['type'])
    module.set_name(module_state['name'])
    if 'condition' in module_state and module_state['condition'] is not None:
        deserialize_conditions(module, module_state)
    if 'flag' in module_state and module_state['flag']:
        # for some modules, this flag might be changed from the default
        module.set_property_flags(pybasf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
    for parameter_state in module_state['parameters']:
        module.param(parameter_state['name'],
                     deserialize_value(module, parameter_state))
    return module


def serialize_path(path):
    """Serialize a basf2 Path into a python dictionary"""
    return {'modules': [serialize_module(module) for module in path.modules()]}


def deserialize_path(path_state):
    """Deserialize a basf2 Path from a python dictionary"""
    path = pybasf2.Path()
    for module_state in path_state['modules']:
        module = deserialize_module(module_state)
        path.add_module(module)
    return path


def get_path_from_file(path_filename):
    """Read a path from a given pickle file"""
    with open(path_filename, 'br') as f:
        return deserialize_path(_pickle.load(f))


def write_path_to_file(path, filename):
    """Write a path to a given pickle file"""
    with open(filename, 'bw') as f:
        _pickle.dump(serialize_path(path), f)


def check_pickle_path(path):
    """Check if the path to be executed should be pickled or unpickled.
    This function is used by basf2.process to handle the ``--dump-path`` and
    ``--execute-path`` arguments to ``basf2``
    """
    # If a pickle path is set via  --dump-path or --execute-path we do something special
    pickle_filename = pybasf2.get_pickle_path()
    if pickle_filename == '':
        return path

    # If the given path is None and the picklePath is valid we load a path from the pickle file
    if _os.path.isfile(pickle_filename) and path is None:
        path = get_path_from_file(pickle_filename)
        with open(pickle_filename, "br") as f:
            loaded = _pickle.load(f)
        if 'state' in loaded:
            pybasf2.B2INFO("Pickled path contains a state object. Activating pickled state.")
            for name, args, kwargs in loaded['state']:
                getattr(_sys.modules[__name__], name)(*args, **kwargs)
        return path

    # Otherwise we dump the given path into the pickle file and exit
    elif path is not None:
        write_path_to_file(path, pickle_filename)
        return None
    else:
        pybasf2.B2FATAL("Couldn't open path-file '" + pickle_filename + "' and no steering file provided.")


def make_code_pickable(code):
    """
    Sometimes it is necessary to execute code which won't be pickled if a user dumps the basf2 path
    and wants to execute it later. Using the pickable_basf2 module all calls to basf2 functions
    are recorded. Now if a user has to execute code outside of basf2, e.g. modifying objects in the ROOT namespace,
    this won't be pickled. By wrapping the code in this function it is technically a call to a basf2 function
    and will be pickled again. Problem solved.
    """
    exec(code, globals())
