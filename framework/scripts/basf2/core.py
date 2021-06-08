#!/usr/bin/env python3

"""
The core module of the Belle II Analysis Software Framework.
"""

import sys as _sys
import signal as _signal

# now let's make sure we actually run in python 3
if _sys.version_info[0] < 3:
    print("basf2 requires python3. Please run the steering files using basf2 "
          "(or python3), not python")
    _sys.exit(1)

# import to override print function
from basf2 import _override_print  # noqa

# import the C++ library with the exported functions
import pybasf2  # noqa
# and also import all of them in current scope for ease of use
from pybasf2 import *  # noqa

# make sure conditions objects are read only
from basf2 import _constwrapper  # noqa


#: name of the framework
basf2label = 'BASF2 (Belle Analysis Software Framework 2)'
#: and copyright notice
basf2copyright = 'Copyright(C) 2010-2021 Belle II Collaboration'

# -----------------------------------------------
#               Prepare basf2
# -----------------------------------------------

# Reset the signal handler to allow the framework execution
# to be stopped with Ctrl-c (Python installs own handler)
# This will again be replaced once process() is called.
_signal.signal(_signal.SIGINT, _signal.SIG_DFL)


def register_module(name_or_module, shared_lib_path=None, logLevel=None, debugLevel=None, **kwargs):
    """
    Register the module 'name' and return it (e.g. for adding to a path). This
    function is intended to instantiate existing modules. To find out which
    modules exist you can run :program:`basf2 -m` and to get details about the
    parameters for each module you can use :program:`basf2 -m {modulename}`

    Parameters can be passed directly to the module as keyword parameters or can
    be set later using `Module.param`

    >>> module = basf2.register_module('EventInfoSetter', evtNumList=100, logLevel=LogLevel.ERROR)
    >>> module.param("evtNumList", 100)

    Parameters:
      name_or_module: The name of the module type, may also be an existing
           `Module` instance for which parameters should be set
      shared_lib_path (str): An optional path to a shared library from which the
           module should be loaded
      logLevel (LogLevel): indicates the minimum severity of log messages
           to be shown from this module. See `Module.set_log_level`
      debugLevel (int): Number indicating the detail of debug messages, the
           default level is 100. See `Module.set_debug_level`
      kwargs: Additional parameters to be passed to the module.

    Note:
        You can also use `Path.add_module()` directly,
        which accepts the same name, logging and module parameter arguments. There
        is no need to register the module by hand if you will add it to the path in
        any case.
    """

    if isinstance(name_or_module, pybasf2.Module):
        module = name_or_module
    else:
        module_name = name_or_module
        if shared_lib_path is not None:
            module = pybasf2._register_module(module_name, shared_lib_path)
        else:
            module = pybasf2._register_module(module_name)

    if kwargs:
        module.param(kwargs)
    if logLevel is not None:
        module.set_log_level(logLevel)
    if debugLevel is not None:
        module.set_debug_level(debugLevel)

    return module


def set_module_parameters(path, name=None, type=None, recursive=False, **kwargs):
    """Set the given set of parameters for all `modules <Module>` in a path which
    have the given ``name`` (see `Module.set_name`)

    Usage is similar to `register_module()` but this function will not create
    new modules but just adjust parameters for modules already in a `Path`

    >>> set_module_parameters(path, "Geometry", components=["PXD"], logLevel=LogLevel.WARNING)

    Parameters:
      path (basf2.Path): The path to search for the modules
      name (str): Then name of the module to set parameters for
      type (str): The type of the module to set parameters for.
      recursive (bool): if True also look in paths connected by conditions or `Path.for_each()`
      kwargs: Named parameters to be set for the module, see  `register_module()`
    """

    if name is None and type is None:
        raise ValueError("At least one of name or type has to be given")

    if not kwargs:
        raise ValueError("no module parameters given")

    found = False
    for module in path.modules():
        if (name is None or module.name() == name) and (type is None or module.type() == type):
            # use register_module as this automatically takes care of logLevel
            # and debugLevel parameters
            register_module(module, **kwargs)
            found = True

        if recursive:
            if module.has_condition():
                for condition_path in module.get_all_condition_paths():
                    set_module_parameters(condition_path, name, type, recursive, **kwargs)
            if module.type() == "SubEvent":
                for subpath in [p.values for p in module.available_params() if p.name == "path"]:
                    set_module_parameters(subpath, name, type, recursive, **kwargs)

    if not found:
        raise KeyError("No module with given name found anywhere in the path")


def remove_module(old_path, name=None):
    """Remove a module of the given ``name`` from the path (see `Module.set_name`)

    Usage is very simple, in this example we remove Geometry the path:

    >>> main = remove_module(main, "Geometry")

    Parameters:
      old_path (basf2.Path): The path to search for the module
      name (str): Then name of the module you want to remove
    """

    if name is None:
        raise ValueError("You should provide the module name")

    new_path = create_path()

    for module in old_path.modules():
        if name != module.name():
            new_path.add_module(module)

    return new_path


def create_path():
    """
    Creates a new path and returns it. You can also instantiate `basf2.Path` directly.
    """
    return pybasf2.Path()


def process(path, max_event=0):
    """
    Start processing events using the modules in the given `basf2.Path` object.

    Can be called multiple times in one steering file (some restrictions apply:
    modules need to perform proper cleanup & reinitialisation, if Geometry is
    involved this might be difficult to achieve.)

    When used in a Jupyter notebook this function will automatically print a
    nice progress bar and display the log messages in an advanced way once the
    processing is complete.

    Note:
     This also means that in a Jupyter Notebook, modifications to class members
     or global variables will not be visible after processing is complete as
     the processing is performed in a subprocess.

     To restore the old behavior you can use ``basf2.core.process()`` which
     will behave exactly identical in Jupyter notebooks as it does in normal
     python scripts ::

           from basf2 import core
           core.process(path)


    Parameters:
      path: The path with which the processing starts
      max_event:  The maximal number of events which will be processed,
                0 for no limit

    .. versionchanged:: release-03-00-00
       automatic Jupyter integration
    """

    # if we are running in an ipython session set the steering file to the
    # current history
    try:
        ipython = get_ipython()  # noqa
        history = "\n".join(e[2] for e in ipython.history_manager.get_range())
        from ROOT import Belle2
        Belle2.Environment.Instance().setSteering(history)
    except NameError:
        pass

    # If a pickle path is set via  --dump-path or --execute-path we do something special
    if pybasf2.get_pickle_path() != "":
        from basf2.pickle_path import check_pickle_path
        path = check_pickle_path(path)

    # apparently nothing to do
    if path is None:
        return

    pybasf2.B2INFO("Starting event processing, random seed is set to '" + pybasf2.get_random_seed() + "'")

    if max_event != 0:
        pybasf2._process(path, max_event)
    else:
        pybasf2._process(path)


def set_log_level(level):
    """
    Sets the global log level which specifies up to which level the
    logging messages will be shown

    Parameters:
      level (basf2.LogLevel): minimum severity of messages to be logged
    """

    logging.log_level = level


def set_debug_level(level):
    """
    Sets the global debug level which specifies up to which level the
    debug messages should be shown

    Parameters:
      level (int): The debug level. The default value is 100
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

    Parameters:
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


def _add_independent_path(self, skim_path, ds_ID='', merge_back_event=None):
    """
    Add given path at the end of this path and ensure all modules there
    do not influence the main DataStore. You can thus use modules in
    skim_path to clean up e.g. the list of particles, save a skimmed uDST file,
    and continue working with the unmodified DataStore contents outside of
    skim_path.

    Parameters:
      ds_ID: can be specified to give a defined ID to the temporary DataStore,
        otherwise, a random name will be generated.
      merge_back_event: is a list of object/array names (of event durability)
        that will be merged back into the main path.
    """
    if merge_back_event is None:
        merge_back_event = []
    self._add_independent_path(skim_path, ds_ID, merge_back_event)


pybasf2.Path.add_module = _add_module
pybasf2.Path.add_independent_path = _add_independent_path


def get_default_global_tags():
    """
    Return the list of default globaltags in one string separated with comma

    .. deprecated:: release-04-00-00
       Please use `basf2.conditions.default_globaltags <ConditionsConfiguration.default_globaltags>` instead
    """
    B2WARNING("basf2.get_default_global_tags() is deprecated, please use basf2.conditions.default_globaltags")
    return ",".join(conditions.default_globaltags)


def set_central_database_networkparams(**argk):
    """
    Set some expert database connection details

    .. deprecated:: release-04-00-00
       Please use `basf2.conditions.expert_settings <ConditionsConfiguration.expert_settings>` instead
    """
    B2WARNING("basf2.set_central_database_networkparams() is deprecated, please use basf2.conditions.expert_settings()")
    return conditions.expert_settings(**argk)


def set_central_serverlist(serverlist):
    """
    Set the list of database servers

    .. deprecated:: release-04-00-00
       Please use `basf2.conditions.metadata_providers <ConditionsConfiguration.metadata_providers>` instead
    """
    B2WARNING("basf2.set_central_serverlist() is deprecated, please use basf2.conditions.metadata_providers instead")
    conditions.metadata_providers = serverlist + [e for e in conditions.metadata_providers if not e.startswith("http")]
