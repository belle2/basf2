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
    else:
        fw.process(path, max_event)


def print_params(module, print_values=True, shared_lib_path=None):
    """
    This function prints parameter information

    module: Print the parameter information of this module
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


