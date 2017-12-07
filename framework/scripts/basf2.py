#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
The core module of the Belle II Analysis Software Framework.
"""

# now let's make sure we actually run in python 3
import sys

if sys.version_info[0] < 3:
    print("basf2 requires python3. Please run the steering files using basf2 "
          "(or python3), not python")
    sys.exit(1)

# import to override print function
import basf2_print

import os
import signal
from pybasf2 import *
# inspect is also used by LogPythonInterface. Do not remove!
import inspect
# we need to patch PyDBObj so lets import the ROOT cpp backend.
import cppyy
# However importing ROOT.kIsConstMethod and kIsStatic is a bad idea
# here since it triggers final setup of ROOT and thus starts a gui thread
# (probably) and consumes command lines if not disabled *sigh*. So we take them
# as literal values from TDictionary.h

#: EProperty::kIsStatic value from TDictionary.h
_ROOT_kIsStatic = 0x00004000
#: EProperty::kIsConstMethod value from TDictionary.h
_ROOT_kIsConstMethod = 0x10000000


def _avoidPyRootHang():
    """
    workaround for possible hang with PyROOT on SL5
    see https://agira.desy.de/browse/BII-976
    note: platform.libc_ver() is _really_ broken, so i'm checking the
    version via ldd (ships with libc)
    """
    import subprocess
    ldd_ver = subprocess.check_output(['ldd', '--version'])
    sl5_libc_string = b"ldd (GNU libc) 2.5"
    if sl5_libc_string in ldd_ver:
        try:
            from ROOT import PyConfig
            PyConfig.StartGuiThread = False
        except BaseException:
            print("PyRoot not set up, this will cause problems.")


_avoidPyRootHang()

# -----------------------------------------------
#             Set basf2 information
# -----------------------------------------------
basf2label = 'BASF2 (Belle Analysis Software Framework 2)'
basf2copyright = 'Copyright(C) 2010-2017  Belle II Collaboration'

# -----------------------------------------------
#               Prepare basf2
# -----------------------------------------------

# Reset the signal handler to allow the framework execution
# to be stopped with Ctrl-c (Python installs own handler)
# This will again be replaced once process() is called.
signal.signal(signal.SIGINT, signal.SIG_DFL)
# avoid root bug
signal.signal(signal.SIGPIPE, signal.SIG_DFL)

# Create default framework, also initialises environment
fw = Framework()
logging = LogPythonInterface()


# -----------------------------------------------
#      Global basf2 function definitions
# -----------------------------------------------


def get_terminal_width():
    """
    Returns width of terminal in characters, or 80 if unknown.
    """
    from shutil import get_terminal_size
    return get_terminal_size(fallback=(80, 24)).columns


def pretty_print_table(table, column_widths, first_row_is_heading=True, transform=None, min_flexible_width=10):
    """
    Pretty print a given table, by using available terminal size and
    word wrapping fields as needed.

    :param table: A 2d list of table fields. Each row must have the same length.

    :param column_width: list of column widths, needs to be of same length as rows
        in 'table'. Available fields are::

            -n  as needed, up to n characters, word wrap if longer
            0   as long as needed, no wrapping
            n   n characters (fixed)
            *   use all available space, good for description fields.
                If more than one column has a * they all get equal width
            +   use all available space but at least the actual width. Only useful
                to make the table span the full width of the terminal

    :param first_row_is_heading: header specifies if we should take the first row
                          as table header and offset it a bit

    :param transform: either None or a callback function which takes three
        arguments: first the elements of the row as a list, second the width of
        each column (without separator) and finally the preformatted text line.
        It should return a string representing the final line to be printed.

    :param min_flexible_width: the minimum amount of characters for every column marked with *
    """

    import textwrap

    # figure out how much space we need for each column (without separators)
    act_column_widths = [len(cell) for cell in table[0]]
    for row in table:
        for (col, cell) in enumerate(row):
            act_column_widths[col] = max(len(str(cell)), act_column_widths[col])

    # adjust act_column_widths to comply with user-specified widths
    total_used_width = 0
    long_columns = []  # index of * column, if found
    for (col, opt) in enumerate(column_widths):
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

    format_string = ' '.join(['%%-%ss' % length for length in
                              act_column_widths[:-1]])
    # don't print extra spaces at end of each line
    format_string += ' %s'

    # print table
    if first_row_is_heading:
        print(total_used_width * '-')

    header_shown = False
    for row in table:
        # use automatic word wrapping on module description (last field)
        wrapped_row = [textwrap.wrap(str(row[i]), width) for (i, width) in
                       enumerate(act_column_widths)]
        max_lines = max([len(col) for col in wrapped_row])
        for line in range(max_lines):
            for (i, cell) in enumerate(row):
                if line < len(wrapped_row[i]):
                    row[i] = wrapped_row[i][line]
                else:
                    row[i] = ''
            line = format_string % tuple(row)
            if transform is not None:
                line = transform(row, act_column_widths, line)
            print(line)

        if not header_shown and first_row_is_heading:
            print(total_used_width * '-')
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
    import textwrap
    wrapper = textwrap.TextWrapper(width=term_width, initial_indent="",
                                   subsequent_indent=" " * (module_width))

    useColors = LogPythonInterface.terminal_supports_colors()

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


def register_module(name_or_module, shared_lib_path=None, logLevel=None, debugLevel=None, **kwargs):
    """
    Register the module 'name' and return it (e.g. for adding to a path). This
    function is intended to instantiate existing modules. To find out which
    modules exist you can run :program:`basf2 -m` and to get details about the
    parameters for each module you can use :program:`basf2 -m {modulename}`

    Parameters can be passed directly to the module as keyword parameters or can
    be set later using `Module.param`

    >>> module = basf.register_module('EventInfoSetter', evtNumList=100, logLevel=LogLevel.ERROR)
    >>> module.param("evtNumList", 100)

    :param name_or_module: The name of the module type, may also be an existing
           `Module` instance for which parameters should be set
    :param str shared_lib_path: An optional path to a shared library from which the
           module should be loaded
    :param LogLevel logLevel: indicates the minimum severity of log messages
           to be shown from this module. See `Module.set_log_level`
    :param int debugLevel: Number indicating the detail of debug messages, the
           default level is 100. See `Module.set_debug_level`
    :param kwargs: Additional parameters to be passed to the module.

    .. note::

        You can also use `Path.add_module()` directly,
        which accepts the same name, logging and module parameter arguments. There
        is no need to register the module by hand if you will add it to the path in
        any case.
    """

    if isinstance(name_or_module, Module):
        module = name_or_module
    else:
        module_name = name_or_module
        if shared_lib_path is not None:
            module = fw.register_module(module_name, shared_lib_path)
        else:
            module = fw.register_module(module_name)

    if kwargs:
        module.param(kwargs)
    if logLevel is not None:
        module.set_log_level(logLevel)
    if debugLevel is not None:
        module.set_debug_level(debugLevel)

    return module


def set_module_parameters(path, name, recursive=False, **kwargs):
    """Set the given set of parameters for all `modules <Module>` in a path which
    have the given ``name`` (see `Module.set_name`)

    Usage is similar to `register_module()` but this function will not create
    new modules but just adjust parameters for modules already in a `Path`

    >>> set_module_parameters(path, "Geometry", components=["PXD"], logLevel=LogLevel.WARNING)

    :param path: The path to search for the modules
    :param name: Then name of the module to set parameters for
    :param recursive: if True also look in paths connected by conditions or `Path.for_each()`
    :param kwargs: Named parameters to be set for the module, see  `register_module()`
    """
    if not kwargs:
        raise ValueError("no parameters given")

    found = False
    for module in path.modules():
        if module.name() == name:
            # use register_module as this automatically takes care of logLevel
            # and debugLevel parameters
            register_module(module, **kwargs)
            found = True

        if recursive:
            if module.has_condition():
                for condition_path in module.get_all_condition_paths():
                    set_module_parameters(condition_path, name, recursive, **kwargs)
            if module.type() == "SubEvent":
                for subpath in [p.values for p in module.available_params() if p.name == "path"]:
                    set_module_parameters(subpath, name, recursive, **kwargs)

    if not found:
        raise KeyError("No module with given name found anywhere in the path")


def create_path():
    """
    Creates a new path and returns it. You can also instantiate `basf2.Path` directly.
    """
    return Path()


def process(path, max_event=0):
    """
    Start processing events using the modules in the given `basf2.Path` object.

    Can be called multiple times in one steering file (some restrictions apply:
    modules need to perform proper cleanup & reinitialisation, if Geometry is
    involved this might be difficult to achieve.)

    :param path: The path with which the processing starts
    :param max_event:  The maximal number of events which will be processed,
                0 for no limit
    """

    # if we are running in an ipython session set the steering file to the
    # current history
    try:
        ipython = get_ipython()
        history = "\n".join(e[2] for e in ipython.history_manager.get_range())
        from ROOT import Belle2
        Belle2.Environment.Instance().setSteering(history)
    except NameError:
        pass

    # If a pickle path is set via  --dump-path or --execute-path we do something special
    pickle_path = fw.get_pickle_path()
    if pickle_path != '':
        # If the given path is None and the picklePath is valid we load a path from the pickle file
        if os.path.isfile(pickle_path) and path is None:
            path = get_path_from_file(pickle_path)
            import pickle
            loaded = pickle.load(open(pickle_path, 'br'))
            if 'state' in loaded:
                B2INFO("Pickled path contains a state object. Activating pickled state.")
                for name, args, kwargs in loaded['state']:
                    getattr(sys.modules[__name__], name)(*args, **kwargs)

        # Otherwise we dump the given path into the pickle file and exit
        elif path is not None:
            write_path_to_file(path, pickle_path)
            return
        else:
            B2FATAL("Couldn't open path-file '" + pickle_path + "' and no steering file provided.")

    B2INFO("Starting event processing, random seed is set to '" + get_random_seed() + "'")

    if max_event != 0:
        fw.process(path, max_event)
    else:
        fw.process(path)


def set_nprocesses(nproc):
    """
    Sets number of worker processes for parallel processing.

    Can be overridden using the ``-p`` argument to basf2.

    :param nproc: number of worker processes. 0 to disable parallel processing.
    """

    fw.set_nprocesses(nproc)


def set_streamobjs(obj_list):
    """
    When using parallel processing, limits the objects/arrays streamed between
    processes to those in the provided list. Can be used to improve performance
    in some workloads.
    """
    fw.set_streamobjs(obj_list)


def print_all_modules(moduleList, package=''):
    """
    Loop over the list of available modules,
    register them and print their information
    """

    fail = False

    modules = []
    for (moduleName, sharedLib) in sorted(moduleList.items()):
        try:
            current_module = register_module(moduleName)
            if package == '' or current_module.package() == package:
                modules.append((current_module.package(), moduleName, current_module.description()))
        except ModuleNotCreatedError:
            B2ERROR('The module {} could not be loaded.'.format(moduleName))
            fail = True
        except Exception as e:
            B2ERROR('An exception occured when trying to load the module {}: {}'.format(moduleName, e))
            fail = True

    table = []
    current_package = ''
    for (packageName, moduleName, description) in sorted(modules):
        if current_package != packageName:
            current_package = packageName
            table.append((current_package,))
        table.append((moduleName, description))
    if package != '' and len(table) == 0:
        B2FATAL('Print module information: No module or package named "' +
                package + '" found!')

    pretty_print_description_list(table)

    print('To show detailed information on a module, including its parameters,')
    print("type \'basf2 -m ModuleName\'. Use \'basf2 -m package\' to only list")
    print('modules belonging to a given package.')

    if fail:
        B2FATAL("One or more modules could not be loaded. Please check the "
                "following ERROR messages and contact the responsible authors.")


def print_params(module, print_values=True, shared_lib_path=None):
    """
    This function prints parameter information

    :param module: Print the parameter information of this module
    :param print_values: Set it to True to print the current values of the parameters
    :param shared_lib_path: The path of the shared library from which the module was
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

    :param defaults: Set it to True to print also the parameters with default values
    :param description: Set to True to print the descriptions of modules and
        parameters
    :param indentation: an internal parameter to indent the whole output (needed for outputting subpaths)
    :param title: show the title string or not (defaults to True)
    """

    if title:
        B2INFO('Modules and parameter settings in the path:')
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
            out = indentation_string + '      %s=%s' % (param.name, param.values)
            if description:
                out += '  #%s' % param.description
            print(out)

        for condition in module.get_all_conditions():
            out = "\n" + indentation_string + '      ' + str(condition) + ":"
            print(out)
            print_path(condition.get_path(), defaults=defaults, description=description, indentation=indentation + 6,
                       title=False)


def set_log_level(level):
    """
    Sets the global log level which specifies up to which level the
    logging messages will be shown

    :param basf2.LogLevel level: minimum severity of messages to be logged
    """

    logging.log_level = level


def set_debug_level(level):
    """
    Sets the global debug level which specifies up to which level the
    debug messages should be shown

    :param int level: The debug level. The default value is 100
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

    :param filename: The path and filename of the text file
    :param append: Should the logging system append the messages to the end of the
         file (True) or create a new file for each event processing session (False).
         Default is False.
    """

    logging.add_file(filename, append)


def reset_log():
    """
    Resets the logging by removing all logging destinations
    """

    logging.reset()


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
    module = register_module(module, logLevel=logLevel, debugLevel=debugLevel, **kwargs)
    self._add_module_object(module)
    return module


def _add_independent_path(self, skim_path, ds_ID='', merge_back_event=[]):
    """
    Add given path at the end of this path and ensure all modules there
    do not influence the main DataStore. You can thus use modules in
    skim_path to clean up e.g. the list of particles, save a skimmed uDST file,
    and continue working with the unmodified DataStore contents outside of
    skim_path.

    :param ds_ID: can be specified to give a defined ID to the temporary DataStore,
        otherwise, a random name will be generated.
    :param merge_back_event: is a list of object/array names (of event durability)
        that will be merged back into the main path.
    """
    self._add_independent_path(skim_path, ds_ID, merge_back_event)


Path.add_module = _add_module
Path.add_independent_path = _add_independent_path


class _TObjectConstWrapper:
    """Empty class to check whether an instance is already const wrapped"""
    pass


def _make_tobject_nonconst(obj):
    """Make a once read only TObject writeable again"""
    if isinstance(obj, _TObjectConstWrapper):
        object.__setattr__(obj, "__class__", obj.__real_class__)
        del obj.__real_class__
    return obj


def _make_tobject_const(obj):
    """
    Make a TObject const: Disallow setting any attributes and calling any
    methods which are not declared as const. This affects any reference to this
    object in python and stays permanent. Once called this particular instance
    will be readonly everywhere and will remain like this.

    This is done by modifying the __class__ attribute of the object and replace
    it with a new subclass which hides all non-const, non-static member
    functions and disable attribute setting. The reason we do it like this is
    that this is the only way how comparison and copy constructor calls work
    transparently. E.g with a normal instance of type T 'a' and a const wrapped
    T 'b' we can do things like a == b, b == a, a < b, b > a, c = T(b).
    """
    # nothing to do if already const wrapped or None
    if obj is None or isinstance(obj, _TObjectConstWrapper):
        return obj

    try:
        #: list of all non-const, public methods
        non_const = [m.GetName() for m in obj.Class().GetListOfAllPublicMethods()
                     if not (m.Property() & (_ROOT_kIsConstMethod | _ROOT_kIsStatic))]
    except AttributeError:
        raise ValueError("Object does not have a valid dictionary: %r" % obj)

    def __make_const_proxy(obj, name):
        """return a proxy function which just raises an attribute error on access"""

        def proxy(self, *args):
            """raise attribute error when called"""
            raise AttributeError("%s is readonly and method '%s' is not const" % (obj, name))
        return proxy

    def __setattr(self, name, value):
        """disallow setting of any attributes"""
        raise AttributeError("%s is readonly, can't set attribute" % obj)

    # ok we create a derived class which removes access to non-const, non-static member
    # functions by adding properties for them which will always cause
    # AttributeErrors
    scope = {m: property(__make_const_proxy(obj, m)) for m in non_const}
    # Also add a __setattr__ to make sure nobody changes anything
    scope["__setattr__"] = __setattr
    # create a dynamic type for this which inherits from the original type and
    # _TObjectConstWrapper so we can distuingish it later. Add all our proxies
    # as scope
    normal_type = type(obj)
    const_type = type("const %s" % normal_type.__name__, (normal_type, _TObjectConstWrapper), scope)
    # remember the old class
    obj.__real_class__ = normal_type
    # and sneakily replace the class of this object :D
    obj.__class__ = const_type
    # done
    return obj


def _PyDBArray__iter__(self):
    """Provide iteration over Belle2::PyDBArray. Needs to be done here since we
    want to make all returned classes read only"""
    for i in range(len(self)):
        yield self[i]


# now replace the PyDBObj getter with one that returns non-modifiable objects.
# This is how root does it in ROOT.py so let's keep that
dbobj_scope = cppyy._backend.CreateScopeProxy("Belle2::PyDBObj")
dbobj_scope.obj = lambda self: _make_tobject_const(self._obj())
# and allow to use it most of the time without calling obj() like the ->
# indirection in C++
dbobj_scope.__getattr__ = lambda self, name: getattr(self.obj(), name)
# also make item access in DBArray readonly
dbarray_scope = cppyy._backend.CreateScopeProxy("Belle2::PyDBArray")
dbarray_scope.__getitem__ = lambda self, i: _make_tobject_const(self._get(i))
# and supply an iterator
dbarray_scope.__iter__ = _PyDBArray__iter__


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
        module.if_value(str(ConditionOperator.values[cond['operator']]) + str(cond['value']),
                        deserialize_path(cond['path']), AfterConditionPath.values[cond['option']])


def serialize_module(module):
    if module.type() == '' or module.type() == 'PyModule':
        raise RuntimeError("Module '%s' doesn't have a type or is a Python module! Note that --dump-path cannot work"
                           "properly with basf2 modules written in Python." % (module.name()))
    return {
        'name': module.name(),
        'type': module.type(),
        'flag': module.has_properties(ModulePropFlags.PARALLELPROCESSINGCERTIFIED),
        'parameters': [{'name': parameter.name, 'values': serialize_value(module, parameter)}
                       for parameter in module.available_params()
                       if parameter.setInSteering or module.type() == 'SubEvent'],
        'condition': serialize_conditions(module) if module.has_condition() else None}


def deserialize_module(module_state):
    module = fw.register_module(module_state['type'])
    module.set_name(module_state['name'])
    if 'condition' in module_state and module_state['condition'] is not None:
        deserialize_conditions(module, module_state)
    if 'flag' in module_state and module_state['flag']:
        # for some modules, this flag might be changed from the default
        module.set_property_flags(ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
    for parameter_state in module_state['parameters']:
        module.param(parameter_state['name'],
                     deserialize_value(module, parameter_state))
    return module


def serialize_path(path):
    return {'modules': [serialize_module(module) for module in path.modules()]}


def deserialize_path(path_state):
    path = Path()
    for module_state in path_state['modules']:
        module = deserialize_module(module_state)
        path.add_module(module)
    return path


def get_path_from_file(path_filename):
    import pickle
    with open(path_filename, 'br') as f:
        return deserialize_path(pickle.load(f))


def write_path_to_file(path, filename):
    import pickle
    with open(filename, 'bw') as f:
        pickle.dump(serialize_path(path), f)


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
