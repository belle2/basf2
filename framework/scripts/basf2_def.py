#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2_env import *

import textwrap
from subprocess import Popen, PIPE


def get_terminal_width():
    """
    Returns width of terminal in characters, or 80 if unknown.
    """
    try:
        pipe = Popen('stty size', shell=True, stdout=PIPE, stderr=PIPE)
        return int(pipe.stdout.read().split()[1])
    except:
        return 80


def register_module(name, shared_lib_path=None):
    """
    This function registers a new module

    name: The name of the module type
    shared_lib_path: An optional path to a shared library from which the
                     module should be loaded
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


def print_all_modules(moduleList):
    """
    Loop over the list of available modules,
    register them and print their information
    """

    term_width = get_terminal_width()
    if term_width < 50:
        term_width = 50

    for (moduleName, sharedLib) in sorted(moduleList.iteritems()):
        try:
            current_module = register_module(moduleName)
        except:
            B2ERROR('The module could not be loaded. This is most likely '
                    + 'caused by a library with missing links.')

        # use automatic word wrapping on module description
        description = textwrap.wrap(current_module.description(),
                term_width - 22)
        continued = False
        for text in description:
            if continued:
                moduleName = ''
            print '%-21s %-s' % (moduleName, text)
            # ommit module name on all following lines
            continued = True

    print ''
    print term_width * '-'
    print ''
    print 'To show detailed information on a module, including its parameters,'
    print 'type \'basf2 -m ModuleName\'.'


def print_params(module, print_values=True, shared_lib_path=None):
    """
    This function prints parameter information

    module: Print the parameter information of this module
    print_values: Set it to True to print the current values of the parameters
    shared_lib_path: The path of the shared library from which the module was
                     loaded
    """

    term_width = get_terminal_width()

    print ''
    print '==================='
    print '%s' % module.name()
    print '==================='
    print 'Description: %s' % module.description()
    if shared_lib_path is not None:
        print 'Found in:    %s' % shared_lib_path
    print term_width * '-'

    #gather output data in table
    output = []
    if print_values:
        output.append([
            'Parameter',
            'Type',
            'Default',
            'Current',
            'Steering',
            'Description',
            ])
    else:
        output.append(['Parameter', 'Type', 'Default', 'Description'])

    has_forced_params = False
    paramList = module.available_params()
    for paramItem in paramList:
        defaultStr = ', '.join(['%s' % defaultItem for defaultItem in
                               paramItem.default])
        valueStr = ', '.join(['%s' % valueItem for valueItem in
                             paramItem.values])
        forceString = ''
        if paramItem.forceInSteering:
            forceString = '*'
            has_forced_params = True
            defaultStr = ''
        if print_values:
            output.append([
                forceString + paramItem.name,
                paramItem.type,
                defaultStr,
                valueStr,
                paramItem.setInSteering,
                paramItem.description,
                ])
        else:
            output.append([forceString + paramItem.name, paramItem.type,
                defaultStr, paramItem.description])

    #figure out how much space we need for each column (without separators)
    column_lengths = [len(cell) for cell in output[0]]
    for row in output:
        for col, cell in enumerate(row):
            column_lengths[col] = max(len(str(cell)), column_lengths[col])

    #both description and default value might be fairly long, set a limit
    default_value_width = min(column_lengths[2], 20)
    column_lengths[2] = default_value_width

    #total width of printed table, minus description
    total_width = sum(column_lengths[:-1]) + len(column_lengths) - 1
    description_width = max(term_width - total_width, 10)
    column_lengths[-1] = description_width

    format_string = ' '.join(['%%-%ss' % length
            for length in column_lengths[:-1]])
    format_string += ' %s'

    header_shown = False
    for row in output:
        # use automatic word wrapping on module description (last field)
        default_lines = textwrap.wrap(row[2], default_value_width)
        description_lines = textwrap.wrap(row[-1], description_width)
        for line in xrange(max(len(default_lines), len(description_lines))):
            if line < len(default_lines):
                row[2] = default_lines[line]
            if line < len(description_lines):
                row[-1] = description_lines[line]

            print format_string % tuple(row)

            # ommit other cols on all following lines
            for i in xrange(len(row)):
                row[i] = ''

        if not header_shown:
            print term_width * '-'
            header_shown = True
    print ''
    if has_forced_params:
        print ' * denotes a required parameter.'


def print_path(path, defaults=False, description=False):
    """
    This function prints the modules in the given path and the module
    parameters.
    Parameters that are not set by the user are suppressed by default.

    defaults: Set it to True to print also the parameters with default values
    description: Set to True to print the descriptions of modules and
    parameters
    """

    B2INFO('Modules and parameter settings in the path:')
    index = 1
    for module in path.modules():
        out = '%2d. %s' % (index, module.name())
        if description:
            out += '  #%s' % module.description()
        print out
        index += 1
        for param in module.available_params():
            if not defaults and param.values == param.default:
                continue
            values = param.values
            if not param.type.startswith('List'):
                if len(values) > 0:
                    values = values[0]
                else:
                    values = ''
            out = '      %s=%s' % (param.name, values)
            if description:
                out += '  #%s' % param.description
            print out


def set_log_level(level):
    """
    Sets the global log level which specifies up to which level the
    logging messages will be shown

    level: LogLevel.DEBUG/INFO/WARNING/ERROR/FATAL
    """

    logging.log_level = level


def set_debug_level(level):
    """
    Sets the global debug level which specifies up to which level the
    debug messages should be shown

    level: The debug level. The default value is 100
    """

    logging.debug_level = level


def log_to_console(color=False):
    """
    Adds the standard output stream to the list of logging destinations.
    The shell logging destination is
    added to the list by the framework by default.
    """

    logging.add_console(color)


def log_to_file(filename, append=False):
    """
    Adds a text file to the list of logging destinations.

    filename: The path and filename of the text file
    append: Should the logging system append the messages to the end of the
    file (True) or create a new file for each event processing session (False).
    Default is False.
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
    This method can be used to redirect the default data directory to a
    user specified one.
    datapath: The direcotry in which the data for the framework is located.
    """

    fw.set_data_search_path(datapath)
