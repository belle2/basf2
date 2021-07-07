#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
basf2.utils - Helper functions for printing basf2 objects
---------------------------------------------------------

This modules contains some utility functions used by basf2, mainly for printing
things.
"""

import inspect as _inspect
from shutil import get_terminal_size as _get_terminal_size
import textwrap as _textwrap
import pybasf2


def get_terminal_width():
    """
    Returns width of terminal in characters, or 80 if unknown.
    """
    return _get_terminal_size(fallback=(80, 24)).columns


def pretty_print_table(table, column_widths, first_row_is_heading=True, transform=None, min_flexible_width=10, *,
                       hline_formatter=None):
    """
    Pretty print a given table, by using available terminal size and
    word wrapping fields as needed.

    Parameters:
      table: A 2d list of table fields. Each row must have the same length.

      column_width: list of column widths, needs to be of same length as rows
        in 'table'. Available fields are

        ``-n``
          as needed, up to n characters, word wrap if longer

        ``0``
          as long as needed, no wrapping

        ``n``
          n characters (fixed)

        ``*``
          use all available space, good for description fields.
          If more than one column has a * they all get equal width

        ``+``
          use all available space but at least the actual width. Only useful
          to make the table span the full width of the terminal

        The column width can also start with ``>``, ``<`` or ``^`` in which case
        it will be right, left or center aligned.

      first_row_is_heading: header specifies if we should take the first row
          as table header and offset it a bit

      transform: either None or a callback function which takes three
          arguments

              1. the elements of the row as a list
              2. second the width of each column (without separator)
              3. the preformatted text line.

          It should return a string representing the final line to be
          printed.

      min_flexible_width: the minimum amount of characters for every column
          marked with *

      hline_formatter: A callable function to format horizontal lines (above and
          below the table header). Should be a callback with one parameter for
          the total width of the table in characters and return a string that
          is the horizontal line. If None is returned no line is printed.

          If argument is not given or given as None the default of printing '-'
          signs are printed over the whole table width is used.

    .. versionchanged:: after release 5
       Added support for column alignment
    """

    # figure out how much space we need for each column (without separators)
    act_column_widths = [len(cell) for cell in table[0]]
    for row in table:
        for (col, cell) in enumerate(row):
            act_column_widths[col] = max(len(str(cell)), act_column_widths[col])

    # adjust act_column_widths to comply with user-specified widths
    total_used_width = 0
    long_columns = []  # index of * column, if found
    # alignment character of the column following str.format
    align = []
    for (col, opt) in enumerate(column_widths):
        # check if our column is aligned
        if isinstance(opt, str) and opt[0] in "<>^":
            align.append(opt[0])
            opt = opt[1:]
        else:
            align.append('')
        # and try to convert option to an int
        try:
            opt = int(opt)
        except ValueError:
            pass

        if opt == '*':
            # handled after other fields are set
            long_columns.append(col)
            continue
        elif opt == "+":
            # handled after other fields are set. Distinguish from * by using by
            # using negative indices
            long_columns.append(- col - 1)
            continue
        elif isinstance(opt, int) and opt > 0:
            # fixed width
            act_column_widths[col] = opt
        elif isinstance(opt, int) and opt == 0:
            # as long as needed, nothing to do
            pass
        elif isinstance(opt, int) and opt < 0:
            # width may be at most 'opt'
            act_column_widths[col] = min(act_column_widths[col], -opt)
        else:
            print('Invalid column_widths option "' + str(opt) + '"')
            return
        total_used_width += act_column_widths[col]

    # add separators
    total_used_width += len(act_column_widths) - 1

    if long_columns:
        remaining_space = max(get_terminal_width() - total_used_width, len(long_columns) * min_flexible_width)
        # ok split the table into even parts but divide up the remainder
        col_width, remainder = divmod(remaining_space, len(long_columns))
        for i, col in enumerate(long_columns):
            size = col_width + (1 if i < remainder else 0)
            if col < 0:
                # negative index: a '+' specifier: make column large but at
                # least as wide as content. So convert column to positive and
                # set the width
                col = -1 - col
                act_column_widths[col] = max(size, act_column_widths[col])
                # if we are larger than we should be add the amount to the total
                # table width
                total_used_width += act_column_widths[col] - size
            else:
                act_column_widths[col] = size

        total_used_width += remaining_space

    format_string = ' '.join(['{:%s%d}' % opt for opt in zip(align, act_column_widths[:-1])])
    # don't print extra spaces at end of each line unless it's specifically aligned
    if not align[-1]:
        format_string += ' {}'
    else:
        format_string += ' {:%s%d}' % (align[-1], act_column_widths[-1])

    if hline_formatter is not None:
        hline = hline_formatter(total_used_width)
    else:
        hline = total_used_width * "-"

    # print table
    if first_row_is_heading and hline is not None:
        print(hline)

    header_shown = False
    for row in table:
        # use automatic word wrapping on module description (last field)
        wrapped_row = [_textwrap.wrap(str(row[i]), width) for (i, width) in
                       enumerate(act_column_widths)]
        max_lines = max([len(col) for col in wrapped_row])
        for line in range(max_lines):
            for (i, cell) in enumerate(row):
                if line < len(wrapped_row[i]):
                    row[i] = wrapped_row[i][line]
                else:
                    row[i] = ''
            line = format_string.format(*row)
            if transform is not None:
                line = transform(row, act_column_widths, line)
            print(line)

        if not header_shown and first_row_is_heading and hline is not None:
            print(hline)
            header_shown = True


def pretty_print_description_list(rows):
    """
    Given a list of 2-tuples, print a nicely formatted description list.
    Rows with only one entry are interpreted as sub-headings
    """
    term_width = get_terminal_width()
    # indentation width
    module_width = 24
    # text wrapper class to format description to terminal width
    wrapper = _textwrap.TextWrapper(width=term_width, initial_indent="",
                                    subsequent_indent=" " * (module_width))

    useColors = pybasf2.LogPythonInterface.terminal_supports_colors()

    def bold(text):
        """Use ANSI sequence to show string in bold"""
        if useColors:
            return '\x1b[1m' + text + '\x1b[0m'
        return text

    print('')
    print(term_width * '-')
    # loop over all modules
    for row in rows:
        if len(row) == 1:
            subheading = row[0]
            print('')
            print(bold(subheading).center(term_width))
        else:
            name, description = row
            for i, line in enumerate(description.splitlines()):
                if i == 0:
                    # set indent of the first description line to have enough
                    # space for the module name (at least module_width) and
                    # output a bold module name and the description next to it
                    wrapper.initial_indent = max(module_width, len(name) + 1) * " "
                    print(bold(name.ljust(module_width - 1)), wrapper.fill(line).lstrip())
                else:
                    # not first line anymore, no module name in front so initial
                    # indent is equal to subsequent indent
                    wrapper.initial_indent = wrapper.subsequent_indent
                    print(wrapper.fill(line))

    print(term_width * '-')
    print('')


def print_all_modules(moduleList, package=''):
    """
    Loop over the list of available modules,
    register them and print their information
    """

    fail = False

    modules = []
    for moduleName in moduleList:
        try:
            current_module = pybasf2._register_module(moduleName)
            if package == '' or current_module.package() == package:
                modules.append((current_module.package(), moduleName, current_module.description()))
        except pybasf2.ModuleNotCreatedError:
            pybasf2.B2ERROR(f'The module {moduleName} could not be loaded.')
            fail = True
        except Exception as e:
            pybasf2.B2ERROR(f'An exception occurred when trying to load the module {moduleName}: {e}')
            fail = True

    table = []
    current_package = ''
    for (packageName, moduleName, description) in sorted(modules):
        if current_package != packageName:
            current_package = packageName
            table.append((current_package,))
        table.append((moduleName, description))
    if package != '' and len(table) == 0:
        pybasf2.B2FATAL('Print module information: No module or package named "' +
                        package + '" found!')

    pretty_print_description_list(table)

    print('To show detailed information on a module, including its parameters,')
    print("type \'basf2 -m ModuleName\'. Use \'basf2 -m package\' to only list")
    print('modules belonging to a given package.')

    if fail:
        pybasf2.B2FATAL("One or more modules could not be loaded. Please check the "
                        "following ERROR messages and contact the responsible authors.")


def print_params(module, print_values=True, shared_lib_path=None):
    """
    This function prints parameter information

    Parameters:
      module: Print the parameter information of this module
      print_values: Set it to True to print the current values of the parameters
      shared_lib_path: The path of the shared library from which the module was
          loaded
    """

    print('')
    print('=' * (len(module.name()) + 4))
    print('  %s' % module.name())
    print('=' * (len(module.name()) + 4))
    print('Description: %s' % module.description())
    if shared_lib_path is not None:
        print('Found in:    %s' % shared_lib_path)
    print('Package:     %s' % module.package())

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
    print('')
    if has_forced_params:
        print(' * denotes a required parameter.')


def print_path(path, defaults=False, description=False, indentation=0, title=True):
    """
    This function prints the modules in the given path and the module
    parameters.
    Parameters that are not set by the user are suppressed by default.

    Parameters:
      defaults: Set it to True to print also the parameters with default values
      description: Set to True to print the descriptions of modules and
          parameters
      indentation: an internal parameter to indent the whole output
          (needed for outputting sub-paths)
      title: show the title string or not (defaults to True)
    """

    if title:
        pybasf2.B2INFO('Modules and parameter settings in the path:')
    index = 1

    indentation_string = ' ' * indentation

    for module in path.modules():
        out = indentation_string + ' % 2d. % s' % (index, module.name())
        if description:
            out += '  #%s' % module.description()
        print(out)
        index += 1
        for param in module.available_params():
            if not defaults and param.values == param.default:
                continue
            out = indentation_string + f'      {param.name}={param.values}'
            if description:
                out += '  #%s' % param.description
            print(out)

        for condition in module.get_all_conditions():
            out = "\n" + indentation_string + '      ' + str(condition) + ":"
            print(out)
            print_path(condition.get_path(), defaults=defaults, description=description, indentation=indentation + 6,
                       title=False)


def is_mod_function(mod, func):
    """Return true if ``func`` is a function and defined in the module ``mod``"""
    return _inspect.isfunction(func) and _inspect.getmodule(func) == mod


def list_functions(mod):
    """
    Returns list of function names defined in the given Python module.
    """
    return [func.__name__ for func in mod.__dict__.values() if is_mod_function(mod, func)]


def pretty_print_module(module, module_name, replacements=None):
    """Pretty print the contents of a python module.
    It will print all the functions defined in the given module to the console

    Arguments:
        module: instance of the module or name with which it can be found in
            `sys.modules`
        module_name: readable module name
        replacements (dict): dictionary containing text replacements: Every
            occurrence of any key in the function signature will be replaced by
            its value
    """
    from terminal_utils import Pager
    desc_list = []

    if replacements is None:
        replacements = {}

    # allow mod to be just the name of the module
    if isinstance(module, str):
        import sys
        module = sys.modules[module]

    for function_name in sorted(list_functions(module), key=lambda x: x.lower()):
        function = getattr(module, function_name)
        signature = _inspect.formatargspec(*_inspect.getfullargspec(function))
        for key, value in replacements.items():
            signature = signature.replace(key, value)
        desc_list.append((function.__name__, signature + '\n' + function.__doc__))

    with Pager('List of available functions in ' + module_name, True):
        pretty_print_description_list(desc_list)
