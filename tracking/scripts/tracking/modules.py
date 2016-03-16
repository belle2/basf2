#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""This file contains python modules that are helpful in construction BASF2 paths for tracking runs.
"""

import basf2
import tracking

import ROOT

import logging

import tracking.metamodules as metamodules
import tracking.root_utils as root_utils
from tracking.run.event_generation import StandardEventGenerationRun
from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule

from ROOT import Belle2


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
        with root_utils.root_open(self.root_file) as tfile:
            root_utils.root_browse(tfile)
            # FIXME: Is there a way to listen to the close event of the TBrowser?
            input('Press enter to close.')

        super(BrowseTFileOnTerminateModule, self).terminate()


class StandardTrackingReconstructionModule(metamodules.PathModule):

    """Populates a path with the modules introduced by reconstuction.add_reconstruction() and wraps it as a module."""

    def __init__(self, *args, **kwds):
        """Initialises initialises and wraps a path populated with a call to reconstuction.add_reconstruction(path, *args, *kwds)

       The signature is expressed generically such that it matches the current arguments of the add_reconstruction call.
       Consult the documentation of reconstruction.add_reconstruction for meaning and names of the possible arguments.
       """

        path = basf2.create_path()
        tracking.add_tracking_reconstruction(path, *args, **kwds)
        super(StandardTrackingReconstructionModule, self).__init__(path)


class CDCMCFinder(metamodules.WrapperModule):
    """Convenience module to add the MC track finder to the path."""

    def __init__(
            self,
            use_cdc=True,
            use_svd=False,
            use_pxd=False,
            only_primaries=False,
            only_axials=False,
            track_candidates_store_array_name="MCTrackCands"):
        mc_track_finder_module = StandardEventGenerationRun.get_basf2_module(
            'TrackFinderMCTruth',
            UseCDCHits=use_cdc,
            UseSVDHits=use_svd,
            UsePXDHits=use_pxd,
            UseOnlyAxialCDCHits=only_axials,
            GFTrackCandidatesColName=track_candidates_store_array_name)

        if only_primaries:
            mc_track_finder_module.param("WhichParticles", ["primary"])
        else:
            mc_track_finder_module.param("WhichParticles", [])

        metamodules.WrapperModule.__init__(self, mc_track_finder_module)
