#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2_env import *


def register_module(name, shared_lib_path=None):
    """"
    This function registers a new module

    name: The name of the module type
    shared_lib_path: An optional path to a shared library from which the module should be loaded
    """

    if shared_lib_path is not None:
        return fw.register_module(name, shared_lib_path)
    else:
        return fw.register_module(name)


def create_path():
    """
    This function creates a new path
    """

    return fw.create_path()


def process(path, max_event=0, run_number=None):
    """
    This function processes the events

    path: The path with which the processing starts
    max_event:  The max_event number of events which will be processed
    run_number: The run number
    """

    if run_number is not None:
        fw.process(path, max_event, run_number)
    elif max_event != 0:
        fw.process(path, max_event)
    else:
        fw.process(path)


def nprocess(nproc):
    """
    Sets number of processes for parallel processing

    nproc: number of processes
    """

    fw.set_nprocess(nproc)


def print_params(module, print_values=True, shared_lib_path=None):
    """
    This function prints parameter information

    module: Print the parameter information of this module
    print_values: Set it to True to print the current values of the parameters
    shared_lib_path: The path of the shared library from which the module was loaded
    """

    print ''
    print '==================='
    print '%s' % module.name()
    print '==================='
    print 'Description: %s' % module.description()
    if shared_lib_path is not None:
        print 'Found in:    %s' % shared_lib_path
    print '-----------------------------------------------------------------------------'
    if print_values:
        print '  %-20s %-20s %-20s %-30s %-10s %s' % (
            'Name',
            'Type',
            'Default',
            'Current',
            'Steering',
            'Description',
            )
    else:
        print '  %-20s %-20s %-20s %s' % ('Name', 'Type', 'Default',
                'Description')
    print '-----------------------------------------------------------------------------'

    paramList = module.available_params()
    for paramItem in paramList:
        defaultStr = ', '.join(['%s' % defaultItem for defaultItem in
                               paramItem.default])
        valueStr = ', '.join(['%s' % valueItem for valueItem in
                             paramItem.values])
        forceString = ''
        if paramItem.forceInSteering:
            forceString = '*'
        if print_values:
            print '%-1s %-20s %-20s %-20s %-30s %-10s %s' % (
                forceString,
                paramItem.name,
                paramItem.type,
                defaultStr,
                valueStr,
                paramItem.setInSteering,
                paramItem.description,
                )
        else:
            print '%-1s %-20s %-20s %-20s %s' % (forceString, paramItem.name,
                    paramItem.type, defaultStr, paramItem.description)
    print ''


def set_log_level(level):
    """
    Sets the global log level which specifies up to which level the logging messages will be shown

    level: LogLevel.DEBUG/INFO/WARNING/ERROR/FATAL
    """

    logging.log_level = level


def set_debug_level(level):
    """
    Sets the global debug level which specifies up to which level the debug messages should be shown

    level: The debug level. The default value is 100
    """

    logging.debug_level = level


def log_to_console(color=False):
    """
    Adds the standard output stream to the list of logging destinations. The shell logging destination is
    added to the list by the framework by default.
    """

    logging.add_console(color)


def log_to_file(filename, append=False):
    """
    Adds a text file to the list of logging destinations.

    filename: The path and filename of the text file
    append: Should the logging system append the messages to the end of the file (True) or create a new file for each event processing session (False). Default is False.
    """

    logging.add_file(filename, append)


def reset_log():
    """
    Resets the logging by removing all logging destinations
    """

    logging.reset()


def set_data_search_path(datapath):
    """
    Sets the path which points to the data directory of the framework.
    This method can be used to redirect the default data directory to a user specified one.
    datapath: The direcotry in which the data for the framework is located.
    """

    fw.set_data_search_path(datapath)


def set_random_seed(seed):
    """
    Sets the seed of the random number generator
    """

    fw.set_random_seed(seed)


