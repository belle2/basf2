#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""This file contains python modules that generally decorate other modules and paths
to serve a slightly changed purpose or circumstance.

Note:
  A great deal of the these modules are quite general purpose helper constructs,
  which might be helpful in other parts of the basf2, and might therefore be better
  placed in the framework folder.
"""

import basf2
from ROOT import Belle2

import cProfile
import pstats

import logging


def get_logger():
    return logging.getLogger(__name__)


class WrapperModule(basf2.Module):

    """Wrapping module base class that wraps a single module to slightly change its behaviour.

    The base implementation does nothing, but forward the relevant methods to the wrapped module.

    Note:
      Implements the decorator/wrapper pattern.

    Attributes:
      module (basf2.Module): The wrapped module instance.
    """

    def __init__(self, module):
        """Create a new wrapper module around the given module. This base implementation does not change anything,
           so there is no reason to use this base implementation alone."""
        super(WrapperModule, self).__init__()
        name = self.compose_wrapped_module_name(module)

        #: The wrapped module
        self.module = module

        # Forward the logging parameters
        self.set_debug_level(self.module.logging.debug_level)
        self.set_abort_level(self.module.logging.abort_level)

        if self.module.logging.log_level != basf2.LogLevel.default:
            self.set_log_level(self.module.logging.log_level)
            self.set_log_info(self.module.logging.log_level,
                              self.module.logging.get_info(self.module.logging.log_level))

        # Forward the name of this module to the C++ world
        self.set_name(name)

    @property
    def wrapper_name(self):
        """Name of the wrapper class."""
        return self.__class__.__name__

    @property
    def param(self):
        """Forwards the parameters"""
        return self.module.param

    @property
    def available_params(self):
        """Forwards the avaiilable parameters"""
        return self.module.available_params

    def compose_wrapped_module_name(self, module):
        """Compose a name that indicates the wrapped module."""
        return "{wrapper_name}({module_name})".format(module_name=module.name(),
                                                      wrapper_name=self.wrapper_name)

    def get_name(self):
        """Forwards the name()."""
        return self.name()

    def initialize(self):
        """Initialize method of the module"""
        self.module.initialize()

    def beginRun(self):
        """Begin run method of the module"""
        self.module.beginRun()

    def event(self):
        """Event method of the module"""
        self.module.event()

    def endRun(self):
        """End run method of the module"""
        self.module.endRun()

    def terminate(self):
        """Terminate method of the module"""
        self.module.terminate()


class PyProfilingModule(WrapperModule):

    """Wrapper module that evaluates the computational performance of python modules.

    Uses cProfile.

    Attributes:
      module (basf2.Module): The wrapped module that should be profiled.
        Should be a module written in Python, since the profile interacts
        with the interpretor for the measurements, but cannot look into c++ implementations.

      output_file_name (str, optional): Path to the file where the profiling information
        shall be stored. Defaults to profile.txt.

      profiler (cProfile.Profile): Profiler instance to manage and extract the profiling statistics.
    """

    #: The default name for output if none is given.
    default_output_file_name = "profile.txt"

    def __init__(self, module, output_file_name=None):
        """Create a new PyProfilingModule wrapped around the given module
           which outputs its results to the output_file_name of given (if not, to profile.txt)."""
        super(PyProfilingModule, self).__init__(module)

        #: The output file name the results will be written into.
        self.output_file_name = self.default_output_file_name

        if output_file_name is not None:
            self.output_file_name = output_file_name

    def initialize(self):
        """Initialize method of the module"""
        #: The used profiler instance.
        self.profiler = cProfile.Profile()
        super(PyProfilingModule, self).initialize()

    def event(self):
        """Event method of the module"""
        profiler = self.profiler
        profiler.enable()
        super(PyProfilingModule, self).event()
        profiler.disable()

    def terminate(self):
        """Terminate method of the module"""
        super(PyProfilingModule, self).terminate()
        sortby = 'cumulative'
        with open(self.output_file_name, 'w') as profile_output_file:
            profile_stats = pstats.Stats(self.profiler, stream=profile_output_file)
            profile_stats.sort_stats(sortby)
            profile_stats.print_stats()


class IfModule(WrapperModule):

    """Wrapper module to conditionally execute module and continue with the normal path afterwards.

    There are two ways to specify the condition.
    One way is to override the condtion(self) method in a subclass.
    The second way is to give a function as the second argument to the module constructor,
    which is called each event.

    Attributes:
      module (basf2.Module): The module executed, if the condition is met.

      condition (function() -> bool, optional): Function executed at each event to determine,
        if the given module shall be executed.
        If None call the condition method instead, which can be overridden by subclasses.

    """

    def __init__(self, module, condition=None):
        """Initialisation method taking the module instance to be wrapped.

        Args:
          module (basf2.Module): The module executed, if the condition is met.
          condition  (function() -> bool, optional): Function executed at each event to determine,
            if the given module shall be executed.
            If None call the condition method instead, which can be overridden by subclasses.
        """

        super(IfModule, self).__init__(module)
        if condition is not None:
            #: Condition function called at each event to determine
            # if wrapped module should be executed
            self.condition = condition

    def condition(self):
        """Condition method called if not given a condition function during construction.

        Can be overridden by a concrete subclass to specify
        under which condition the wrapped module should be executed.
        It can also be shadowed by a condition function given to the constructor of this module.

        Returns:
          bool: The indication if the wrapped module should be executed.
            Always True in the base implementation
        """
        return True

    def event(self):
        """Event method of the module

        Evaluates the condition and sets the return value of this module
        to trigger the execution of the wrapped module.
        """

        if self.condition():
            super(IfModule, self).event()


def is_storearray_present(storearray_name,
                          storearray_durability=0):
    """Checks if a StoreArray with name and durability is present in the DataStore.

    Only works during the event processing phase, but not on initialisation,
    due to limitation of the python interface to the framework
    """
    storearray_list = Belle2.PyStoreArray.list(storearray_durability)
    return storearray_name in storearray_list


class IfStoreArrayPresentModule(IfModule):

    """Conditional execution of the wrapped module if a StoreArray is present.

    Attributes:
      storearray_name (str): The name of the StoreArray which presence has to be checked.
      storearray_durability (int): The durability of the StoreArray
      storearray_is_present (bool): The flag whether the StoreArray is present.
        Set during initialisation.
    """

    def __init__(self,
                 module,
                 storearray_name,
                 storearray_durability=0):
        """
        Args:
          module (basf2.Module): The module executed, if the condition is met.
          storearray_name (str): The name of the StoreArray which presence has to be checked.
          storearray_durability (int, optional): The durability of the StoreArray. Default 0.
        """

        super(IfStoreArrayPresentModule, self).__init__(module)

        #: Name of the StoreArray to be checked (str).
        self.storearray_name = storearray_name

        #: Durability of the StoreArray to be checked (int).
        self.storearray_durability = storearray_durability

        #: Flag that the StoreArray is present (bool).
        self.storearray_is_present = None

    def initialize(self):
        """Initialize the contianed module (only of the condition is true)."""
        if self.condition():
            super().initialize()

    def condition(self):
        """Returns true if the StoreArray is present.

        Checks presence of the StoreArray once and remembers the result for all following events.
        """
        if self.storearray_is_present is None:
            self.storearray_is_present = is_storearray_present(self.storearray_name,
                                                               self.storearray_durability)
        return self.storearray_is_present


class IfStoreArrayNotPresentModule(IfStoreArrayPresentModule):

    """Conditional execution of the wrapped module based if a StoreArray is not present."""

    def condition(self):
        """Returns false if the StoreArray is present.

        Checks presence of the StoreArray once and remembers the result for all following events.
        """
        return not IfStoreArrayPresentModule.condition(self)


class IfMCParticlesPresentModule(IfStoreArrayPresentModule):

    """Conditional execution of the wrapped module based on the presence of Monte Carlo information.
    """

    def __init__(self, module):
        """
        Args:
          module (basf2.Module): The module executed, if the condition is met.
        """
        super(IfMCParticlesPresentModule, self).__init__(module, "MCParticles")


class PathModule(basf2.Module):

    """Wrapper for a basf2 path into a module such that
    it can be passed around and added to a basf2 path as a basf2 module.

    The wrapper is implement in such a way that it unconditionally executes
    the contained path by means of a positive return value.
    The calling path is continued after the execution of the wrapped path.

    Attributes:
      _path (basf2.Path): The wrapped execution path.
    """

    def __init__(self, path=None, modules=None):
        """Initialises the module with a path to be wrapped.

        May also give a list of modules that should be appended to the path.
        If the path is omitted a new basf2.Path is constructed.

        Args:
          path (basf2.Path): The execution path to be wrapped.
            If no path is given create a new one.

          modules (iterable of basf2.Module): Module to be added to the path.
        """

        super(PathModule, self).__init__()

        if path is None:
            path = basf2.create_path()

        #: The contained path.
        self._path = path

        if modules:
            for module in modules:
                path.add_module(module)

        self.if_true(path, basf2.AfterConditionPath.CONTINUE)

        # Pass a telling name to the C++ world
        if modules is None:
            itemCount = 0
        else:
            itemCount = len(modules)
        self.set_name("{path_module} ({items} modules):".format(path_module=self.__class__.__name__,
                                                                items=itemCount))

    def event(self):
        """Event method of the module

        Sets the return value of this module to true and triggers the execution of the wrapped path.
        """

        self.return_value(True)
