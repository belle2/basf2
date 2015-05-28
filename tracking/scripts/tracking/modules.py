#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""This file contains python modules that are helpful in construction BASF2 paths for tracking runs.
"""

import basf2
import reconstruction

import ROOT

import logging

import tracking.metamodules as metamodules
from tracking.run.event_generation import StandardEventGenerationRun
from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule


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

    def __init__(self, output_track_cands_store_array_name="TrackCands",
                 tmva_cut=0.1):

        modules = [CDCBackgroundHitFinder(tmva_cut=tmva_cut),
                   CDCLegendreTrackFinder(),
                   CDCLocalTrackFinder(),
                   CDCNotAssignedHitsCombiner(output_track_cands_store_array_name=output_track_cands_store_array_name)
                   ]

        super(CDCFullFinder, self).__init__(modules=modules)


class CDCBackgroundHitFinder(metamodules.WrapperModule):

    """ Adds the background hit finder to the path. The vectors with __Temp get registered but not created.
    WARNING: Pleasy remember that even a tmva_cut of 0 does remove hits!
    """

    def __init__(self, tmva_cut=0.1):
        self.tmva_cut = tmva_cut

        if self.tmva_cut < 0 or self.tmva_cut > 1:
            print "Given tmva_cut %.1f is not in the valid range [0, 1]. Not adding the module" % self.tmva_cut
            return

        background_hit_finder_module = StandardEventGenerationRun.get_basf2_module(
            "SegmentFinderCDCFacetAutomatonDev",
            ClusterFilter="tmva",
            ClusterFilterParameters={
                "cut": str(
                    self.tmva_cut)},
            SegmentsStoreObjName="__TempCDCRecoSegment2DVector",
            CreateGFTrackCands=False,
            WriteGFTrackCands=False,
            FacetFilter="none",
            FacetRelationFilter="none",
            TracksStoreObjName="__TempCDCTracksVector")

        super(CDCBackgroundHitFinder, self).__init__(background_hit_finder_module)


class CDCLegendreTrackFinder(metamodules.PathModule):

    """Add the legendre finder to the path.
    If assign_stereo_hits is True, also add the Histogramming module (default)
    If delete_hit_information is True, all already marked hits get unmarked (not default)
    If output_track_cands_store_array_name is None, do not write out the track cands to a genfit store array (default)
    With output_track_cands_store_vector_name you can control the output vector of the CDCTracks. Be aware that every content
    in this vector will be deleted before executing this module!
    """

    def __init__(self, delete_hit_information=False,
                 output_track_cands_store_array_name=None,
                 output_track_cands_store_vector_name="CDCTrackVector",
                 assign_stereo_hits=True):

        module_list = []

        legendre_tracking_module = StandardEventGenerationRun.get_basf2_module(
            'CDCLegendreTracking',
            WriteGFTrackCands=False,
            TracksStoreObjName=output_track_cands_store_vector_name)
        if delete_hit_information:
            legendre_tracking_module.param('SkipHitsPreparation', False)
        else:
            legendre_tracking_module.param('SkipHitsPreparation', True)

        last_tracking_module = legendre_tracking_module

        cdc_stereo_combiner = StandardEventGenerationRun.get_basf2_module('CDCLegendreHistogramming',
                                                                          SkipHitsPreparation=True,
                                                                          TracksStoreObjNameIsInput=True,
                                                                          WriteGFTrackCands=False,
                                                                          TracksStoreObjName=output_track_cands_store_vector_name)

        if assign_stereo_hits:
            module_list.append(last_tracking_module)
            last_tracking_module = cdc_stereo_combiner

        if output_track_cands_store_array_name is not None:
            last_tracking_module.param({'WriteGFTrackCands': True,
                                        'GFTrackCandsStoreArrayName': output_track_cands_store_array_name})

        module_list.append(last_tracking_module)

        super(CDCLegendreTrackFinder, self).__init__(modules=module_list)


class CDCLocalTrackFinder(metamodules.WrapperModule):

    """ Add the local finder to the path.
    If delete_hit_information is True, all already marked hits get unmarked (not default)
    If output_track_cands_store_array_name is None, do not write out the track cands to a genfit store array (default)
    With output_segments_store_vector_name you can control the output vector of the CDCRecoSegments2D. Be aware that every content
    in this vector will be deleted before executing this module!

    The vector with _Temp gets registered but not created
    """

    def __init__(self, delete_hit_information=False,
                 output_track_cands_store_array_name=None,
                 output_segments_store_vector_name="CDCRecoSegment2DVector"):

        local_track_finder_module = StandardEventGenerationRun.get_basf2_module(
            'SegmentFinderCDCFacetAutomaton',
            SegmentsStoreObjName=output_segments_store_vector_name,
            WriteGFTrackCands=False,
            FitSegments=True,
            TracksStoreObjName="__TempCDCTracksVector")

        if delete_hit_information:
            local_track_finder_module.param('SkipHitsPreparation', False)
        else:
            local_track_finder_module.param('SkipHitsPreparation', True)

        if output_track_cands_store_array_name is not None:
            local_track_finder_module.param({'WriteGFTrackCands': True,
                                             'GFTrackCandsStoreArrayName': output_track_cands_store_array_name,
                                             'CreateGFTrackCands': True})

        super(CDCLocalTrackFinder, self).__init__(local_track_finder_module)


class CDCNotAssignedHitsCombiner(metamodules.WrapperModule):

    """Add the old combiner module to the path
    If output_track_cands_store_array_name is None, do not write out the track cands to a genfit store array (default)
    With track_cands_store_vector_name you can control the input vector of the CDCTracks. Be aware that the content
    of this vector will be replaced by the output of this module
    With segments_store_vector_name you can control the input vector of the CDCRecoSegments2D. Be aware that the content
    of this vector will be replaced by the output of this module
    """

    def __init__(self, output_track_cands_store_array_name=None,
                 track_cands_store_vector_name="CDCTrackVector",
                 segments_store_vector_name="CDCRecoSegment2DVector",
                 use_second_stage=False):

        not_assigned_hits_combiner_module = StandardEventGenerationRun.get_basf2_module(
            "NotAssignedHitsCombiner",
            SkipHitsPreparation=True,
            TracksStoreObjNameIsInput=True,
            WriteGFTrackCands=False,
            SegmentsStoreObjName=segments_store_vector_name,
            TracksStoreObjName=track_cands_store_vector_name,
            UseSecondStage=use_second_stage)

        if output_track_cands_store_array_name is not None:
            not_assigned_hits_combiner_module.param({'WriteGFTrackCands': True,
                                                     'GFTrackCandsStoreArrayName': output_track_cands_store_array_name})

        super(CDCNotAssignedHitsCombiner, self).__init__(not_assigned_hits_combiner_module)


class CDCSegmentTrackCombiner(metamodules.WrapperModule):

    """ Add the new combiner module to the path
    If output_track_cands_store_array_name is None, do not write out the track cands to a genfit store array (default)
    With track_cands_store_vector_name you can control the input vector of the CDCTracks. Be aware that the content
    of this vector will be replaced by the output of this module
    With segments_store_vector_name you can control the input vector of the CDCRecoSegments2D. Be aware that the content
    of this vector will be replaced by the output of this module
    With the other parameters you can control the filters of the SegmentTrackCombiner
    """

    def __init__(self, output_track_cands_store_array_name=None,
                 track_cands_store_vector_name="CDCTrackVector",
                 segments_store_vector_name="CDCRecoSegment2DVector",
                 segment_track_chooser_filter="simple",
                 segment_track_chooser_cut=0,
                 segment_train_filter="simple",
                 segment_track_filter="simple"):

        combiner_module = StandardEventGenerationRun.get_basf2_module("SegmentTrackCombinerDev",
                                                                      SegmentTrackChooser=segment_track_chooser_filter,
                                                                      SegmentTrainFilter=segment_train_filter,
                                                                      SegmentTrackFilter=segment_track_filter,
                                                                      WriteGFTrackCands=False,
                                                                      SkipHitsPreparation=True,
                                                                      TracksStoreObjNameIsInput=True,
                                                                      SegmentsStoreObjName=segments_store_vector_name,
                                                                      TracksStoreObjName=track_cands_store_vector_name)

        if segment_track_chooser_filter == "tmva":
            combiner_module.param('SegmentTrackChooserParameters', {"cut": str(segment_track_chooser_cut)})

        if output_track_cands_store_array_name is not None:
            combiner_module.param({'WriteGFTrackCands': True,
                                   'GFTrackCandsStoreArrayName': output_track_cands_store_array_name})

        super(CDCSegmentTrackCombiner, self).__init__(combiner_module)


class CDCValidation(metamodules.PathModule):

    """Add a validation and a mc track matcher to the path
    With track_candidates_store_array_name you can choose the name of the genfit::TrackCands this validation should be created for
    With output_file_name you can choose the file name for the results

    TODO: Do only create the mc_track_matching relations if not already present
    TODO: Create a parameter to only do the validation if the file is not already present (something like force overwrite)
    """

    def __init__(self, track_candidates_store_array_name, output_file_name):
        from tracking.validation.module import SeparatedTrackingValidationModule

        mc_track_matcher_module = StandardEventGenerationRun.get_basf2_module(
            'MCTrackMatcher',
            UseCDCHits=True,
            UseSVDHits=False,
            UsePXDHits=False,
            RelateClonesToMCParticles=True,
            MCGFTrackCandsColName="MCTrackCands",
            PRGFTrackCandsColName=track_candidates_store_array_name)

        validation_module = SeparatedTrackingValidationModule(
            name="",
            contact="",
            output_file_name=output_file_name,
            trackCandidatesColumnName=track_candidates_store_array_name,
            expert_level=2)

        super(CDCValidation, self).__init__(modules=[mc_track_matcher_module, validation_module])


class CDCFitter(metamodules.PathModule):

    """ Add the genfit module to te path """

    def __init__(self, setup_geometry=True, fit_geometry="Geant4",
                 input_track_cands_store_array_name="TrackCands",
                 output_tracks_store_array_name="GF2Tracks"):

        setup_genfit_extrapolation_module = StandardEventGenerationRun.get_basf2_module('SetupGenfitExtrapolation',
                                                                                        whichGeometry=fit_geometry)
        gen_fitter_module = StandardEventGenerationRun.get_basf2_module('GenFitter',
                                                                        PDGCodes=[211],
                                                                        FilterId="DAF",
                                                                        StoreFailedTracks=False,
                                                                        GFTrackCandidatesColName=input_track_cands_store_array_name,
                                                                        GFTracksColName=output_tracks_store_array_name,
                                                                        BuildBelle2Tracks=False)

        track_builder = StandardEventGenerationRun.get_basf2_module('TrackBuilder',
                                                                    GFTracksColName=output_tracks_store_array_name,
                                                                    GFTrackCandidatesColName=input_track_cands_store_array_name)

        module_list = []
        if setup_geometry:
            module_list.append(setup_genfit_extrapolation_module)

        module_list.append(gen_fitter_module)
        module_list.append(track_builder)

        super(CDCFitter, self).__init__(modules=module_list)


class CDCEventDisplay(metamodules.WrapperModule):

    """ Add the b2display module of the cdc display to the path """

    def __init__(self, full_display=True):

        if full_display:
            display_module = StandardEventGenerationRun.get_basf2_module("Display", showTrackCandidates=True, showMCInfo=False)
        else:
            display_module = CDCSVGDisplayModule()
            display_module.draw_gftrackcand_trajectories = True
            display_module.draw_gftrackcands = True

        super(CDCEventDisplay, self).__init__(display_module)
