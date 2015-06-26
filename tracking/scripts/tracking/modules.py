#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""This file contains python modules that are helpful in construction BASF2 paths for tracking runs.
"""

import basf2
import reconstruction

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
            raw_input('Press enter to close.')

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


class Printer(metamodules.WrapperModule):

    """
    Add the PrintCollections modules to the path.
    This metamodule is just for convenience.
    """

    def __init__(self):
        metamodules.WrapperModule.__init__(self, StandardEventGenerationRun.get_basf2_module("PrintCollections"))


class CDCFullFinder(metamodules.PathModule):

    """
    Full finder sequence for the CDC with a step of Legendre tracking first and cellular automaton tracking second.


    Attributes
    ----------
    output_track_cands_store_array_name: The name for output of the genfit::TrackCands
    tmva_cut: the cut for the BackgroundHitFinder
    combiner_tmva_cut: the cut for the first step of the SegmentTrackCombiner
    """

    def __init__(self, output_track_cands_store_array_name="TrackCands",
                 tmva_cut=0.1,
                 first_filter="tmva", first_tmva_cut=0.67,
                 background_filter="all", background_filter_tmva_cut=0.0,
                 second_filter="none", second_tmva_cut=0.2):

        modules = [
            CDCBackgroundHitFinder(
                tmva_cut=tmva_cut), CDCLocalTrackFinder(), CDCBackgroundHitFinder(
                tmva_cut=tmva_cut), CDCLegendreTrackFinder(
                debug_output=False),
            CDCSegmentTrackCombiner(output_track_cands_store_array_name=output_track_cands_store_array_name,
                                    segment_track_chooser_first_step_cut=first_tmva_cut,
                                    segment_track_chooser_first_step_filter=first_filter,
                                    background_segment_filter=background_filter,
                                    background_segment_cut=background_filter_tmva_cut,
                                    segment_track_chooser_second_step_cut=second_tmva_cut,
                                    segment_track_chooser_second_step_filter=second_filter,
                                    segment_track_filter="none",
                                    segment_train_filter="none")]

        metamodules.PathModule.__init__(self, modules=modules)


class CDCNaiveFinder(metamodules.PathModule):

    """
    Full finder with naive combiner in the end. Can use MC-Information

    Attributes
    ---------_
    output_track_cands_store_array_name: The name for output of the genfit::TrackCands
    tmva_cut: the cut for the BackgroundHitFinder
    stereo_tmva_cut: the cut for the StereoSegmentTrackMatcher
    """

    def __init__(self, output_track_cands_store_array_name="TrackCands", use_mc_information=False, **kwargs):

        full_finder = CDCFullFinder(output_track_cands_store_array_name=None, **kwargs)
        modules = full_finder._path.modules()
        modules.append(
            StandardEventGenerationRun.get_basf2_module(
                "NaiveCombiner",
                SkipHitsPreparation=True,
                TracksStoreObjNameIsInput=True,
                WriteGFTrackCands=True,
                GFTrackCandsStoreArrayName=output_track_cands_store_array_name,
                UseMCInformation=use_mc_information))

        metamodules.PathModule.__init__(self, modules=modules)


class CDCBackgroundHitFinder(metamodules.WrapperModule):

    """ Adds the background hit finder to the path. The vectors with __Temp get registered but not created.
    WARNING: Pleasy remember that even a tmva_cut of 0 does remove hits!

    Attributes
    ----------
    tmva_cut: the cut for the tmva. 0.1 is the default und should be reasonable enough.
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


    Attributes
    ----------
    If assign_stereo_hits is True, also add the Histogramming module (default)
    If delete_hit_information is True, all already marked hits get unmarked (not default)
    If output_track_cands_store_array_name is None, do not write out the track cands to a genfit store array (default)
    With output_track_cands_store_vector_name you can control the output vector of the CDCTracks. Be aware that every content
    in this vector will be deleted before executing this module!
    """

    def __init__(self, delete_hit_information=False,
                 output_track_cands_store_array_name=None,
                 output_track_cands_store_vector_name="CDCTrackVector",
                 assign_stereo_hits=True, debug_output=False,
                 stereo_level=6, stereo_hits=5):

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

        cdc_stereo_combiner = StandardEventGenerationRun.get_basf2_module(
            'StereoHitFinderCDCLegendreHistogramming',
            SkipHitsPreparation=True,
            TracksStoreObjNameIsInput=True,
            WriteGFTrackCands=False,
            TracksStoreObjName=output_track_cands_store_vector_name,
            DebugOutput=debug_output,
            QuadTreeLevel=stereo_level,
            MinimumHitsInQuadtree=stereo_hits)

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


    Attributes
    ----------
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

    Attributes
    ----------
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

    Attributes
    ----------
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
                 segment_track_chooser_first_step_filter="tmva",
                 background_segment_filter="all",
                 segment_track_chooser_second_step_filter="none",
                 segment_track_chooser_first_step_cut=0.75,
                 background_segment_cut=0.0,
                 segment_track_chooser_second_step_cut=0.25,
                 segment_train_filter="simple",
                 segment_track_filter="simple"):

        combiner_module = StandardEventGenerationRun.get_basf2_module(
            "SegmentTrackCombinerDev",
            SegmentTrackChooserFirstStepFilter=segment_track_chooser_first_step_filter,
            BackgroundSegmentsFilter=background_segment_filter,
            SegmentTrackChooserSecondStepFilter=segment_track_chooser_second_step_filter,
            SegmentTrainFilter=segment_train_filter,
            SegmentTrackFilter=segment_track_filter,
            WriteGFTrackCands=False,
            SkipHitsPreparation=True,
            TracksStoreObjNameIsInput=True,
            SegmentsStoreObjName=segments_store_vector_name,
            TracksStoreObjName=track_cands_store_vector_name)

        if segment_track_chooser_first_step_filter == "tmva":
            combiner_module.param(
                'SegmentTrackChooserFirstStepFilterParameters', {
                    "cut": str(segment_track_chooser_first_step_cut)})

        if background_segment_filter == "tmva":
            combiner_module.param(
                'BackgroundSegmentsFilterParameters', {
                    "cut": str(background_segment_cut)})

        if segment_track_chooser_second_step_filter == "tmva":
            combiner_module.param(
                'SegmentTrackChooserSecondStepFilterParameters', {
                    "cut": str(segment_track_chooser_second_step_cut)})

        if output_track_cands_store_array_name is not None:
            combiner_module.param({'WriteGFTrackCands': True,
                                   'GFTrackCandsStoreArrayName': output_track_cands_store_array_name})

        super(CDCSegmentTrackCombiner, self).__init__(combiner_module)


class CDCValidation(metamodules.PathModule):

    """Add a validation and a mc track matcher to the path

    Attributes
    ----------
    With track_candidates_store_array_name you can choose the name of the genfit::TrackCands this validation should be created for
    With output_file_name you can choose the file name for the results

    TODO: Do only create the mc_track_matching relations if not already present
    """

    def __init__(
            self,
            output_file_name,
            track_candidates_store_array_name="TrackCands",
            use_pxd=False,
            use_cdc=True,
            use_svd=False):
        from tracking.validation.module import SeparatedTrackingValidationModule

        mc_track_finder_module_if_module = CDCMCFinder(use_cdc=use_cdc, use_pxd=use_pxd, use_svd=use_svd)

        mc_track_matcher_module = CDCMCMatcher(track_cands_store_array_name=track_candidates_store_array_name,
                                               use_cdc=use_cdc, use_pxd=use_pxd, use_svd=use_svd)

        validation_module = SeparatedTrackingValidationModule(
            name="",
            contact="",
            output_file_name=output_file_name,
            trackCandidatesColumnName=track_candidates_store_array_name,
            expert_level=2)

        super(CDCValidation, self).__init__(modules=[mc_track_finder_module_if_module, mc_track_matcher_module, validation_module])


class CDCMCFinder(metamodules.IfStoreArrayNotPresentModule):

    def __init__(self, use_cdc=True, use_svd=False, use_pxd=False):
        mc_track_finder_module = StandardEventGenerationRun.get_basf2_module('TrackFinderMCTruth',
                                                                             UseCDCHits=use_cdc,
                                                                             UseSVDHits=use_svd,
                                                                             UsePXDHits=use_pxd,
                                                                             WhichParticles=[],
                                                                             GFTrackCandidatesColName="MCTrackCands")

        metamodules.IfStoreArrayNotPresentModule.__init__(self, mc_track_finder_module, storearray_name="MCTrackCands")


class CDCFitter(metamodules.PathModule):

    """ Add the genfit module to te path

    Attributes
    ----------
    setup_geometry: Load the SetupGenfitExtrapolation module
    input_track_cands_store_array_name: store array name for input genfit::TrackCands
    output_tracks_store_array_name: store array name for output G2Tracks

    """

    def __init__(self, setup_geometry=True, fit_geometry="Geant4",
                 input_track_cands_store_array_name="TrackCands",
                 output_tracks_store_array_name="GF2Tracks"):

        setup_genfit_extrapolation_module = StandardEventGenerationRun.get_basf2_module('SetupGenfitExtrapolation',
                                                                                        whichGeometry=fit_geometry)
        gen_fitter_module = StandardEventGenerationRun.get_basf2_module('GenFitter',
                                                                        PDGCodes=[211],
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

    """ Add the b2display or the cdc display module of the cdc display to the path """

    def __init__(self, full_display=True):

        if full_display:
            display_module = StandardEventGenerationRun.get_basf2_module("Display", showTrackCandidates=True,
                                                                         showTrackLevelObjects=True, showMCInfo=False,
                                                                         hideObjects=["Unassigned RecoHits"])
        else:
            display_module = CDCSVGDisplayModule()
            display_module.draw_hits = True
            display_module.draw_track_trajectories = True
            display_module.draw_tracks = True
            display_module.draw_takenflag = False
            display_module.draw_wrong_rl_infos_in_tracks = False
            display_module.draw_wrong_rl_infos_in_segments = False

        super(CDCEventDisplay, self).__init__(display_module)


class CDCMCFiller(basf2.Module):

    """ Fill the later needed mc information """

    def event(self):
        Belle2.TrackFindingCDC.CDCMCManager.getInstance().fill()


class CDCMCMatcher(metamodules.WrapperModule):

    """ Add the mc mathcer module for convenience """

    def __init__(self, mc_track_cands_store_array_name="MCTrackCands",
                 track_cands_store_array_name="TrackCands",
                 use_cdc=True, use_svd=False, use_pxd=False):

        mc_track_matcher_module = StandardEventGenerationRun.get_basf2_module('MCTrackMatcher',
                                                                              UseCDCHits=use_cdc,
                                                                              UseSVDHits=use_svd,
                                                                              UsePXDHits=use_pxd,
                                                                              RelateClonesToMCParticles=True,
                                                                              MCGFTrackCandsColName=mc_track_cands_store_array_name,
                                                                              PRGFTrackCandsColName=track_cands_store_array_name)

        metamodules.WrapperModule.__init__(self, module=mc_track_matcher_module)


class CDCHitUniqueAssumer(basf2.Module):

    """
    A small helper module to look for double assigned hits.
    Prints a summary after execution
    """

    def initialize(self):
        self.number_of_doubled_hits = 0
        self.number_of_total_hits = 0
        self.number_of_hits_with_wrong_flags = 0

    def event(self):
        track_store_vector = Belle2.PyStoreObj('CDCTrackVector')

        if track_store_vector:
            # Wrapper around std::vector like
            wrapped_vector = track_store_vector.obj()
            tracks = wrapped_vector.get()

            for track in tracks:
                # Unset all taken flags
                for recoHit in track.items():
                    if not recoHit.getWireHit().getAutomatonCell().hasTakenFlag():
                        self.number_of_hits_with_wrong_flags += 1

            for track in tracks:
                # Now check that we only have every wire hit once
                for recoHit in track.items():
                    self.number_of_total_hits += 1
                    if recoHit.getWireHit().getAutomatonCell().hasAssignedFlag():
                        self.number_of_doubled_hits += 1
                    recoHit.getWireHit().getAutomatonCell().setAssignedFlag()

                for innerTrack in tracks:
                    for recoHit in innerTrack.items():
                        recoHit.getWireHit().getAutomatonCell().unsetAssignedFlag()

    def terminate(self):
        print "Number of doubled hits:", self.number_of_doubled_hits
        print "Number of hits with wrong taken flag:", self.number_of_hits_with_wrong_flags
        print "Number of total hits:", self.number_of_total_hits


class HitCleaner(basf2.Module):
    #: A small hit cleaner module to set the track information according to mc information.
    #: This is surely not for later usage but for testing the genfitter module

    def __init__(self):
        super(HitCleaner, self).__init__()

        self.number_of_tracks = 0
        self.number_of_deleted_hits = 0
        self.number_of_hits = 0

    def initialize(self):
        self.cdc_hit_look_up = Belle2.TrackFindingCDC.CDCMCHitLookUp()
        self.mc_matcher_lookup = Belle2.TrackMatchLookUp("MCTrackCands", "TrackCands")

    def event(self):
        tracks = Belle2.PyStoreArray("TrackCands")
        cdc_hits = Belle2.PyStoreArray("CDCHits")
        mc_particles = Belle2.PyStoreArray("MCParticles")

        cdc_hit_lookup = self.cdc_hit_look_up
        cdc_hit_lookup.fill()

        mc_matcher_lookup = self.mc_matcher_lookup

        self.number_of_tracks += tracks.getEntries()

        for track in tracks:
            # Store all Hit IDs and reset the track
            hitIDs = track.getHitIDs(Belle2.Const.CDC)
            hits = [cdc_hits[i] for i in hitIDs]
            good_hits = []

            relation_track_particle = [0] * mc_particles.getEntries()

            # Now only add those hits which do belong to the track (with MC Info)
            for i in hitIDs:
                current_mc_track = cdc_hit_lookup.getMCTrackId(cdc_hits[i])
                if 0 <= current_mc_track:
                    relation_track_particle[current_mc_track] += 1

            should_belong_to_track = np.argmax(relation_track_particle)

            deleted_hits = sum(relation_track_particle) - relation_track_particle[should_belong_to_track]

            plane_IDs_of_good_hits = []
            for i, hitID in enumerate(hitIDs):
                current_mc_track = cdc_hit_lookup.getMCTrackId(cdc_hits[hitID])
                if current_mc_track == should_belong_to_track:
                    good_hits.append(hitID)

            self.number_of_deleted_hits += deleted_hits
            self.number_of_hits += len(hitIDs)

            # Set the position and momentum
            mc_track = mc_matcher_lookup.getMatchedMCTrackCand(track)

            if mc_track:
                mc_trajectory = Belle2.TrackFindingCDC.CDCTrajectory3D(Belle2.TrackFindingCDC.Vector3D(mc_track.getPosSeed()),
                                                                       Belle2.TrackFindingCDC.Vector3D(mc_track.getMomSeed()),
                                                                       mc_track.getChargeSeed())
                startingPosition = Belle2.TrackFindingCDC.Vector3D(track.getPosSeed().X(), track.getPosSeed().Y(), 0)
                sStartingPosition = mc_trajectory.calcPerpS(startingPosition)
                zStartingPosition = mc_trajectory.getTrajectorySZ().mapSToZ(sStartingPosition)
                mc_trajectory.setLocalOrigin(Belle2.TrackFindingCDC.Vector3D(startingPosition.xy(), zStartingPosition))

                pos = ROOT.TVector3(mc_trajectory.getSupport().x(), mc_trajectory.getSupport().y(), mc_trajectory.getSupport().z())
                mom = ROOT.TVector3(
                    mc_trajectory.getMom3DAtSupport().x(),
                    mc_trajectory.getMom3DAtSupport().y(),
                    mc_trajectory.getMom3DAtSupport().z())

                # track.setPosMomSeedAndPdgCode(pos, mom , int(mc_track.getChargeSeed() * 211))
                track.setPdgCode(int(track.getChargeSeed() * 211))

            else:
                # track.reset()
                track.setPdgCode(int(track.getChargeSeed() * 211))

    def terminate(self):
        print("Number of tracks in total: %d" % self.number_of_tracks)
        print("Number of hits in total: %d" % self.number_of_hits)
        print("Number of deleted hits: %d" % self.number_of_deleted_hits)

        print("Number of deleted hits per track: %f" % (1.0 * self.number_of_deleted_hits / self.number_of_tracks))
        print("Ratio of deleted hits: %f" % (100.0 * self.number_of_deleted_hits / self.number_of_hits))
