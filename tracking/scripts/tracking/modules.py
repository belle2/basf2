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

import cProfile
import pstats


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
    root_file: str or TFile
        Path to the file or the TFile that should be shown in the browser.
    """

    def __init__(self, root_file):
        super(BrowseFileOnTerminateModule, self).__init__()
        self.root_file = root_file

    def terminate(self):
        """Termination method of the module

        Opens the ROOT file an opens a Browser to show it.
        """

        if isinstance(self.root_file, ROOT.TFile):
            tfile = self.root_file
        else:
            tfile = ROOT.TFile(self.root_file)

        tBrowser = ROOT.TBrowser()
        tBrowser.BrowseObject(tfile)
        tBrowser.Show()

        # FIXME: Is there a way to listen to the close event of the TBrowser?
        raw_input('Press enter to close.')

        # If we opened the file ourselves close it again.
        if not isinstance(self.root_file, ROOT.TFile):
            tfile.Close()


class PyProfilingModule(basf2.Module):

    """Wrapper module that evaluates the computational performance of python modules.

    Uses cProfile

    Attributes
    ----------
    module : basf2.Module
        The wrapped module that should be profiled. Should be a module written in Python,
        since the profile interacts with the interpretor for the measurements,
        but cannot look into c++ implementations.
    output_file_name : str, optional
        Path to the file where the profiling information shall be stored. Defaults to profile.txt.
    profiler : cProf
    """

    default_output_file_name = "profile.txt"

    def __init__(self, module, output_file_name=None):
        super(PyProfilingModule, self).__init__()
        self.module = module

        if output_file_name is None:
            self.output_file_name = self.default_output_file_name
        else:
            self.output_file_name = output_file_name

    @property
    def param(self):
        """Forwards the parameters"""
        return self.module.param

    @property
    def name(self):
        """Forwards the name"""
        return self.module.name

    def initialize(self):
        """Initialize method of the module"""
        self.profiler = cProfile.Profile()
        self.module.initialize()

    def beginRun(self):
        """Begin run method of the module"""
        self.module.beginRun()

    def event(self):
        """Event method of the module"""
        profiler = self.profiler
        profiler.enable()
        self.module.event()
        profiler.disable()

    def endRun(self):
        """End run method of the module"""
        self.module.endRun()

    def terminate(self):
        """Terminate method of the module"""
        self.module.terminate()
        sortby = 'cumulative'
        with open(self.output_file_name, 'w') as profile_output_file:
            profile_stats = pstats.Stats(self.profiler, stream=profile_output_file).sort_stats(sortby)
            profile_stats.print_stats()


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


class CDCFullFinder(PathModule):

    """Full finder sequence for the CDC with a step of Legendre tracking first and cellular automaton tracking second."""

    def __init__(self):
        path = basf2.create_path()

        legendre_track_finder_module = basf2.register_module("CDCLegendreTracking")
        legendre_track_finder_module.param("GFTrackCandidatesColName", "LegendreTrackCands")

        stereo_histogramming_finder = basf2.register_module("CDCLegendreHistogramming")
        stereo_histogramming_finder.param("GFTrackCandidatesColName", "LegendreTrackCands")

        not_assigned_hits_searcher_module = basf2.register_module("NotAssignedHitsSearcher")
        not_assigned_hits_searcher_module.param({"TracksFromFinder": "LegendreTrackCands",
                                                 "NotAssignedCDCHits": "NotAssignedCDCHits"})

        local_track_finder_module = basf2.register_module("SegmentFinderCDCFacetAutomaton")
        local_track_finder_module.param({"UseOnlyCDCHitsRelatedFrom": "NotAssignedCDCHits",
                                         "GFTrackCandsStoreArrayName": "LocalTrackCands"})

        not_assigned_hits_combiner_module = basf2.register_module("NotAssignedHitsCombiner")

        not_assigned_hits_combiner_module.param({"TracksFromLegendreFinder": "LegendreTrackCands",
                                                 "NotAssignedTracksFromLocalFinder": "LocalTrackCands",
                                                 "ResultTrackCands": "TrackCands"})

        path.add_module(legendre_track_finder_module)
        path.add_module(not_assigned_hits_searcher_module)
        path.add_module(local_track_finder_module)
        path.add_module(not_assigned_hits_combiner_module)

        super(CDCFullFinder, self).__init__(path)
