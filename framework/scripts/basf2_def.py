#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2_env import *

import textwrap
import pickle
import os
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


def pretty_print_table(table, column_widths, first_row_is_heading=True):
    """
    Pretty print a given table, by using available terminal size and
    word wrapping fields as needed.

    table: A 2d list of table fields. Each row must have the same length.
    column_width: list of column widths, needs to be of same length as rows
                  in 'table'. Available fields are:
                  -n  as needed, up to n characters, word wrap if longer
                  n   n characters (fixed)
                  *   use all available space, good for description fields
                      (can only be used ONCE)
    first_row_is_heading: header specifies if we should take the first row
                          as table header and offset it a bit
    """

    # figure out how much space we need for each column (without separators)
    act_column_widths = [len(cell) for cell in table[0]]
    for row in table:
        for (col, cell) in enumerate(row):
            act_column_widths[col] = max(len(str(cell)), act_column_widths[col])

    # adjust act_column_widths to comply with user-specified widths
    total_used_width = 0
    long_column = -1  # index of * column, if found
    for (col, opt) in enumerate(column_widths):
        if opt == '*':
            if long_column >= 0:
                print 'column_widths option "*" can only be used once!'
                return

            # handled after other fields are set
            long_column = col
            continue
        elif type(opt) is int and opt > 0:
            # fixed width
            act_column_widths[col] = opt
        elif type(opt) is int and opt < 0:
            # width may be at most 'opt'
            act_column_widths[col] = min(act_column_widths[col], -opt)
        else:
            print 'Invalid column_widths option "' + str(opt) + '"'
            return
        total_used_width += act_column_widths[col]

    # add separators
    total_used_width += len(act_column_widths) - 1

    term_width = get_terminal_width()
    if long_column >= 0:
        # TODO: add option for minimum widh?
        remaining_space = max(term_width - total_used_width, 10)
        act_column_widths[long_column] = remaining_space

    format_string = ' '.join(['%%-%ss' % length for length in
                             act_column_widths[:-1]])
    # don't print extra spaces at end of each line
    format_string += ' %s'

    # print table
    if first_row_is_heading:
        print term_width * '-'

    header_shown = False
    for row in table:
        # use automatic word wrapping on module description (last field)
        wrapped_row = [textwrap.wrap(str(row[i]), width) for (i, width) in
                       enumerate(act_column_widths)]
        max_lines = max([len(col) for col in wrapped_row])
        for line in xrange(max_lines):
            for (i, cell) in enumerate(row):
                if line < len(wrapped_row[i]):
                    row[i] = wrapped_row[i][line]
                else:
                    row[i] = ''

            print format_string % tuple(row)

        if not header_shown and first_row_is_heading:
            print term_width * '-'
            header_shown = True


def register_module(name, shared_lib_path=None):
    """
    Register the module 'name' and return it (e.g. for adding to a path)

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
    Creates a new path and returns it
    """

    return fw.create_path()


def process(path, max_event=0):
    """
    This function processes the events

    path: The path with which the processing starts
    max_event:  The maximal number of events which will be processed,
                0 for no limit
    """

    # If a pickle path is set via  --dump-path or --execute-path we do something special
    pickle_path = fw.get_pickle_path()
    if pickle_path != '':
        # If the given path is None and the picklePath is valid we load a path from the pickle file
        if os.path.isfile(pickle_path) and path is None:
            path = deserialize_path(pickle.load(open(pickle_path, 'r')))
        # Otherwise we dump the given path into the pickle file and exit
        elif not path is None:
            pickle.dump(serialize_path(path), open(pickle_path, 'w'))
            return
        else:
            B2FATAL("Couldn't open path-file '" + pickle_path + "' and no steering file provided.")

    if max_event != 0:
        fw.process(path, max_event)
    else:
        fw.process(path)


def set_nprocesses(nproc):
    """
    Sets number of processes for parallel processing

    nproc: number of processes
    """

    fw.set_nprocesses(nproc)


def print_all_modules(moduleList, package=''):
    """
    Loop over the list of available modules,
    register them and print their information
    """

    term_width = get_terminal_width()

    table = []
    for (moduleName, sharedLib) in sorted(moduleList.iteritems()):
        try:
            current_module = register_module(moduleName)
            if package == '' or current_module.package() == package:
                table.append([moduleName, current_module.description()])
        except:
            B2ERROR('The module could not be loaded. This is most likely '
                    + 'caused by a library with missing links.')
    if package != '' and len(table) == 0:
        B2FATAL('Print module information: No module or package named "'
                + package + '" found!')

    print term_width * '-'
    print ''

    pretty_print_table(table, [25, '*'], first_row_is_heading=False)

    print ''
    print term_width * '-'
    print ''
    print 'To show detailed information on a module, including its parameters,'
    print "type \'basf2 -m ModuleName\'. Use \'basf2 -m package\' to only list"
    print 'modules belonging to a given package.'


def print_params(module, print_values=True, shared_lib_path=None):
    """
    This function prints parameter information

    module: Print the parameter information of this module
    print_values: Set it to True to print the current values of the parameters
    shared_lib_path: The path of the shared library from which the module was
                     loaded
    """

    print ''
    print '=' * (len(module.name()) + 4)
    print '  %s' % module.name()
    print '=' * (len(module.name()) + 4)
    print 'Description: %s' % module.description()
    if shared_lib_path is not None:
        print 'Found in:    %s' % shared_lib_path
    print 'Package:     %s' % module.package()

    # gather output data in table
    output = []
    if print_values:
        output.append([
            'Parameter',
            'Type',
            'Default',
            'Current',
            'Steering',
            'Description'])
    else:
        output.append(['Parameter', 'Type', 'Default', 'Description'])

    has_forced_params = False
    paramList = module.available_params()
    for paramItem in paramList:
        defaultStr = str(paramItem.default)
        valueStr = str(paramItem.values)
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
                paramItem.description])
        else:
            output.append([forceString + paramItem.name, paramItem.type,
                          defaultStr, paramItem.description])

    column_widths = [-25] * len(output[0])
    column_widths[2] = -20  # default values
    column_widths[-1] = '*'  # description

    pretty_print_table(output, column_widths)
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
            out = '      %s=%s' % (param.name, param.values)
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
