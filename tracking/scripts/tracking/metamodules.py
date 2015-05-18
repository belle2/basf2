#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""This file contains python modules that generally decorate other modules and paths
to serve a slightly changed purpose or circumstance.

Note:
  A great deal of the these modules are quite general purpose helper constructs,
  which might be helpful in other parts of the BASF2, and might therefore be better
  placed in the framework folder.
"""

import basf2
import ROOT
from ROOT import Belle2

import cProfile
import pstats

import logging

from ROOT import Belle2


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
        super(WrapperModule, self).__init__()

        #: The wrapped module
        self.module = module

        # Forward the name of this module to the C++ world
        self.set_name(self.name)

    @property
    def wrapper_name(self):
        return self.__class__.__name__

    @property
    def param(self):
        """Forwards the parameters"""
        return self.module.param

    @property
    def name(self):
        """Compose a name that indicates the wrapped module."""
        return "{wrapper_name}({module_name})".format(module_name=self.module.name(),
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

    default_output_file_name = "profile.txt"

    def __init__(self, module, output_file_name=None):
        super(PyProfilingModule, self).__init__(module)

        if output_file_name is None:
            self.output_file_name = self.default_output_file_name
        else:
            self.output_file_name = output_file_name

    def initialize(self):
        """Initialize method of the module"""
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


class IfStoreArrayPresentModule(IfModule):

    """Conditional execution of the wrapped module based if a StoreArray is present.

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

    def condition(self):
        """Returns the flag if the StoreArray is present"""
        if self.storearray_is_present is None:
            storearray = Belle2.PyStoreArray(self.storearray_name, self.storearray_durability)
            if storearray:
                self.storearray_is_present = True
            else:
                self.storearray_is_present = False

        return self.storearray_is_present


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

    """Wrapper for a BASF2 path into a module such that
    it can be passed around and added to a BASF2 path as a BASF2 module.

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

        self._path = path

        if modules:
            for module in modules:
                path.add_module(module)

        self.if_true(path, basf2.AfterConditionPath.CONTINUE)

    @classmethod
    def from_modules(cls, *modules):
        return cls(modules=modules)

    def event(self):
        """Event method of the module

        Sets the return value of this module to true and triggers the execution of the wrapped path.
        """

        self.return_value(True)


class IfPathWithStoreArrayName(PathModule):

    """ Special Path which conditionally executes a given module only of the given store array name is not already present in the
    store array

    Attributes
    ----------
    modules or path: list of basf2.Module or path
        The modules/path executed if a StoreArray with the given name is not already present
    store_array_name:
        The given name for the store array

    """

    def __init__(self, store_array_name, path=None, modules=None):
        """ Initialization method sending the parameters to the base class IfModule """

        self.is_not_store_array = None
        self.store_array_name = store_array_name

        super(IfPathWithStoreArrayName, self).__init__(path=path, modules=modules)

    def search_for_store_array_name_condition(self):
        if self.is_not_store_array is None:
            store_array_list = Belle2.PyStoreArray.list()
            self.is_not_store_array = self.store_array_name not in store_array_list

        print self.is_not_store_array
        return self.is_not_store_array

    def event(self):
        """ Only execute the event if the condition is true """
        self.return_value(self.search_for_store_array_name_condition())
