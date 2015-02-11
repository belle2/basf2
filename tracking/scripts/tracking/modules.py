#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""This file contains python modules that are helpful in construction BASF2 paths for tracking runs.

Note
----
A great deal of the these modules are quite general purpose helper constructs, 
which might be helpful in other parts of the BASF2, and might therefore be better
placed in the framework folder. Most noteably these are the

  * IfModule
  * PathModule

"""

import basf2
import reconstruction

import ROOT

import logging


def get_logger():
    return logging.getLogger(__name__)


class BrowseFileOnTerminateModule(basf2.Module):

    """A simple module that shows a ROOT file on termination of the execution path.

    On termination of the BASF2 path the module opens a ROOT file specified by name and
    shows it in a TBrowser.

    This can be used to show results from a BASF2 run as soon as they are finished.

    Put this module at an early position in your path or at least higher than the module 
    writing the ROOT file to be shown, since the terminate methods are called in reverse order.

    Attributes
    ----------
    root_file_path : str
        Path to the file that should be shown in the browser.
    """

    def __init__(self, root_file_path):
        super(BrowseFileOnTerminateModule, self).__init__()
        self.root_file_path = root_file_path

    def terminate(self):
        """Termination method of the module

        Opens the ROOT file an opens a Browser to show it.
        """

        tFile = ROOT.TFile(self.root_file_path)
        tBrowser = ROOT.TBrowser()
        tBrowser.BrowseObject(tFile)
        tBrowser.Show()

        # FIXME: Is there a way to listen to the close event of the TBrowser?
        raw_input('Press enter to close.')
        tFile.Close()


class IfModule(basf2.Module):

    """Wrapper module to conditionally execute module and continue with the normal path afterwards.

    There are two ways to specify the condition. 
    One way is to override the condtion(self) method in a subclass. 
    The second way is to give a function as the second argument to the module constructor, which is called
    each event.

    Attributes
    ----------
    module : basf2.Module
         The module executed, if the condition is met.
    condition : function() -> bool, optional
         Function executed at each event to determine, if the given module shall be executed.
         If None call the condition method instead, which can be overridden by subclasses
    """

    def __init__(self, module, condition=None):
        """Initialisation method taking the module instance to 

        Parameters
        ----------
        module : basf2.Module
            The module executed, if the condition is met.
        condition : function() -> bool, optional
            Function executed at each event to determine, if the given module shall be executed.
            If None call the condition method instead, which can be overridden by subclasses.
        """

        super(IfModule, self).__init__()

        if condition is not None:
            # Condition function called at each event to determine if wrapped module should be executed
            self.condition = condition

        conditional_path = basf2.create_path()
        conditional_path.add_module(module)
        self.if_true(conditional_path, basf2.AfterConditionPath.CONTINUE)

    def condition(self):
        """Condition method called if not given a condition function during construction.

        Can be overridden by a concrete subclass to specify under which condition the wrapped module should be executed.
        It can also be shadowed by a condition function given to the constructor of this module.

        Returns
        -------
        bool
            The indication if the wrapped module should be executed. Always True in the base implementation
        """

        return True

    def event(self):
        """Event method of the module

        Evaluates the condition and sets the return value of this module to trigger the execution of the wrapped module.
        """

        condition_is_met = self.condition()
        self.return_value(bool(condition_is_met))


class PathModule(basf2.Module):

    """Wrapper for a BASF2 path into a module such that 
    it can be passed around and added to a BASF2 path as BASF2 module.

    The wrapper is implement in such a way that it unconditionally executes
    the contained path by means of a positive return value.
    The calling path is continued after the execution of the wrapped path.

    Attributes
    ----------
    _path : basf2.Path
        The wrapped execution path.
    """

    def __init__(self, path):
        """Initialises the module with a path to be wrapped.

        Parameters
        ----------
        path : basf2.Path
            The execution path to be wrapped
        """

        super(PathModule, self).__init__()

        self._path = path
        self.if_true(path, basf2.AfterConditionPath.CONTINUE)

    @classmethod
    def from_modules(cls, *modules):
        path = basf2.create_path()
        for module in modules:
            path.add_module(module)
        return cls(path)

    def event(self):
        """Event method of the module

        Sets the return value of this module to true and triggers the execution of the wrapped path.
        """

        self.return_value(True)


class StandardTrackingReconstructionModule(PathModule):

    """Populates a path with the modules introduced by reconstuction.add_reconstruction() and wraps it as a module."""

    def __init__(self, *args, **kwds):
        """Initialises initialises and wraps a path populated with a call to reconstuction.add_reconstruction(path, *args, *kwds)

       The signature is expressed generically such that it matches the current arguments of the add_reconstruction call.
       Consult the documentation of reconstruction.add_reconstruction for meaning and names of the possible arguments.
       """

        path = basf2.create_path()
        reconstruction.add_tracking_reconstruction(path, *args, **kwds)
        super(StandardTrackingReconstructionModule, self).__init__(path)


