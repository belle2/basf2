#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Functions necessary to pickle and unpickle a Path and all Modules on it
"""

import pybasf2
import pickle
import inspect
import os
import sys


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


def serialize_conditions(module):
    condition_list = []

    for condition in module.get_all_conditions():
        condition_list.append({'value': condition.get_value(),
                               'operator': int(condition.get_operator()),
                               'path': serialize_path(condition.get_path()),
                               'option': int(condition.get_after_path())})

    return condition_list


def deserialize_conditions(module, module_state):
    conditions = module_state['condition']
    for cond in conditions:
        module.if_value(str(pybasf2.ConditionOperator.values[cond['operator']]) + str(cond['value']),
                        deserialize_path(cond['path']), pybasf2.AfterConditionPath.values[cond['option']])


def serialize_module(module):
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
    return {'modules': [serialize_module(module) for module in path.modules()]}


def deserialize_path(path_state):
    path = pybasf2.Path()
    for module_state in path_state['modules']:
        module = deserialize_module(module_state)
        path.add_module(module)
    return path


def get_path_from_file(path_filename):
    with open(path_filename, 'br') as f:
        return deserialize_path(pickle.load(f))


def write_path_to_file(path, filename):
    with open(filename, 'bw') as f:
        pickle.dump(serialize_path(path), f)


def check_pickle_path(path):
    # If a pickle path is set via  --dump-path or --execute-path we do something special
    pickle_filename = pybasf2.get_pickle_path()
    if pickle_filename == '':
        return path

    # If the given path is None and the picklePath is valid we load a path from the pickle file
    if os.path.isfile(pickle_filename) and path is None:
        path = get_path_from_file(pickle_filename)
        with open(pickle_filename, "br") as f:
            loaded = pickle.load(f)
        if 'state' in loaded:
            pybasf2.B2INFO("Pickled path contains a state object. Activating pickled state.")
            for name, args, kwargs in loaded['state']:
                getattr(sys.modules[__name__], name)(*args, **kwargs)
        return path

    # Otherwise we dump the given path into the pickle file and exit
    elif path is not None:
        write_path_to_file(path, pickle_filename)
        return None
    else:
        pybasf2.B2FATAL("Couldn't open path-file '" + pickle_filename + "' and no steering file provided.")


def is_mod_function(mod, func):
    return inspect.isfunction(func) and inspect.getmodule(func) == mod


def list_functions(mod):
    """
    Returns list of function names defined in the given Python module.
    """
    return [func.__name__ for func in mod.__dict__.values() if is_mod_function(mod, func)]


def make_code_pickable(code):
    """
    Sometimes it is necessary to execute code which won't be pickled if a user dumps the basf2 path
    and wants to execute it later. Using the pickable_basf2 module all calls to basf2 functions
    are recorded. Now if a user has to execute code outside of basf2, e.g. modifying objects in the ROOT namespace,
    this won't be pickled. By wrapping the code in this function it is technically a call to a basf2 function
    and will be pickled again. Problem solved.
    """
    exec(code, globals())
