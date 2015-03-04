#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""This file contains python modules that are helpful in construction BASF2 paths for tracking runs.
"""

import basf2
import reconstruction

import ROOT

import logging

import tracking.metamodules as metamodules


def get_logger():
    return logging.getLogger(__name__)


class BrowseTFileOnTerminateModule(basf2.Module):

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
        super(BrowseTFileOnTerminateModule, self).__init__()
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

        super(BrowseTFileOnTerminateModule, self).terminate()


class StandardTrackingReconstructionModule(metamodules.PathModule):

    """Populates a path with the modules introduced by reconstuction.add_reconstruction() and wraps it as a module."""

    def __init__(self, *args, **kwds):
        """Initialises initialises and wraps a path populated with a call to reconstuction.add_reconstruction(path, *args, *kwds)

       The signature is expressed generically such that it matches the current arguments of the add_reconstruction call.
       Consult the documentation of reconstruction.add_reconstruction for meaning and names of the possible arguments.
       """

        path = basf2.create_path()
        reconstruction.add_tracking_reconstruction(path, *args, **kwds)
        super(StandardTrackingReconstructionModule, self).__init__(path)


class CDCFullFinder(metamodules.PathModule):

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

        modules = [legendre_track_finder_module,
                   not_assigned_hits_searcher_module,
                   local_track_finder_module,
                   not_assigned_hits_combiner_module]

        super(CDCFullFinder, self).__init__(modules=modules)
