#!/usr/bin/env python3
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
        reconstruction.add_tracking_reconstruction(path, *args, **kwds)
        super(StandardTrackingReconstructionModule, self).__init__(path)


class VXDFinder(metamodules.PathModule):

    def __init__(self, sectorSetup=None, tuneCutoffs=0.22, track_candidates_store_array_name="TrackCands",
                 setup_geometry=True, fit_geometry="Geant4",):

        setup_genfit_extrapolation_module = StandardEventGenerationRun.get_basf2_module('SetupGenfitExtrapolation',
                                                                                        whichGeometry=fit_geometry)

        if sectorSetup is None:
            sectorSetup = ['secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-moreThan500MeV_PXDSVD',
                           'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-125to500MeV_PXDSVD',
                           'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-30to125MeV_PXDSVD']

        vxd_module = StandardEventGenerationRun.get_basf2_module("VXDTF",
                                                                 sectorSetup=sectorSetup,
                                                                 tuneCutoffs=tuneCutoffs,
                                                                 GFTrackCandidatesColName=track_candidates_store_array_name)

        modules = []
        if setup_geometry:
            modules.append(setup_genfit_extrapolation_module)

        modules.append(vxd_module)

        metamodules.PathModule.__init__(self, modules=modules)


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
                 tmva_cut=0.2,
                 first_filter="tmva", first_tmva_cut=0.75,
                 background_filter="tmva", background_filter_tmva_cut=0.7,
                 new_segments_filter="none", new_segments_filter_tmva_cut=0,
                 second_filter="none", second_tmva_cut=0,
                 track_filter="tmva", track_filter_cut=0.1,
                 use_pair_finder=False):

        modules = [
            CDCLocalTrackFinder(tmva_cut=tmva_cut), CDCLegendreTrackFinder()
        ]

        filter_parameters_for_combiner = dict(segment_track_filter_first_step_cut=first_tmva_cut,
                                              segment_track_filter_first_step_filter=first_filter,

                                              background_segment_filter=background_filter,
                                              background_segment_cut=background_filter_tmva_cut,

                                              new_segment_filter=new_segments_filter,
                                              new_segment_cut=new_segments_filter_tmva_cut,

                                              segment_track_filter_second_step_cut=second_tmva_cut,
                                              segment_track_filter_second_step_filter=second_filter,

                                              segment_train_filter="none",

                                              segment_information_list_track_filter="none",

                                              track_filter=track_filter,
                                              track_filter_cut=track_filter_cut)

        if use_pair_finder:
            modules.append(CDCSegmentTrackCombiner(output_track_cands_store_array_name=None, **filter_parameters_for_combiner))
            modules.append(CDCSegmentPairAutomatonFinder(output_track_cands_store_array_name=output_track_cands_store_array_name))
        else:
            modules.append(
                CDCSegmentTrackCombiner(
                    output_track_cands_store_array_name=output_track_cands_store_array_name,
                    **filter_parameters_for_combiner))

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

        naive_combiner = StandardEventGenerationRun.get_basf2_module(
            "NaiveCombiner",
            TracksStoreObjNameIsInput=True,
            WriteGFTrackCands=False,
            UseMCInformation=use_mc_information)

        if output_track_cands_store_array_name is not None:
            naive_combiner.param({'WriteGFTrackCands': True,
                                  'GFTrackCandsStoreArrayName': output_track_cands_store_array_name})

        modules.append(naive_combiner)

        metamodules.PathModule.__init__(self, modules=modules)


class CDCBackgroundHitFinder(metamodules.WrapperModule):

    """ Adds the background hit finder to the path. The vectors with __Temp get registered but not created.
    WARNING: Pleasy remember that even a tmva_cut of 0 does remove hits!

    Attributes
    ----------
    tmva_cut: the cut for the tmva. 0.1 is the default und should be reasonable enough.
    """

    def __init__(self, tmva_cut):
        self.tmva_cut = tmva_cut

        if self.tmva_cut < 0 or self.tmva_cut > 1:
            print("Given tmva_cut %.1f is not in the valid range [0, 1]. Not adding the module" % self.tmva_cut)
            return

        background_hit_finder_module = StandardEventGenerationRun.get_basf2_module(
            "SegmentFinderCDCFacetAutomaton",
            ClusterFilter="tmva",
            ClusterFilterParameters={
                "cut": str(
                    self.tmva_cut)},
            SegmentsStoreObjName="__TempCDCRecoSegment2DVector",
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

    def __init__(self, output_track_cands_store_array_name=None,
                 output_track_cands_store_vector_name="CDCTrackVector",
                 assign_stereo_hits=True,
                 TracksStoreObjNameIsInput=False,
                 stereo_level=6, stereo_hits=5):

        module_list = []

        legendre_tracking_module = StandardEventGenerationRun.get_basf2_module(
            'TrackFinderCDCLegendreTracking',
            WriteGFTrackCands=False,
            TracksStoreObjNameIsInput=TracksStoreObjNameIsInput,
            TracksStoreObjName=output_track_cands_store_vector_name)

        module_list.append(legendre_tracking_module)

        quality_module = StandardEventGenerationRun.get_basf2_module("TrackQualityAsserterCDC", WriteGFTrackCands=False,
                                                                     TracksStoreObjNameIsInput=True,
                                                                     corrections=["B2B"])

        last_tracking_module = quality_module

        cdc_stereo_combiner = StandardEventGenerationRun.get_basf2_module(
            'StereoHitFinderCDCLegendreHistogramming',
            TracksStoreObjNameIsInput=True,
            WriteGFTrackCands=False,
            TracksStoreObjName=output_track_cands_store_vector_name,
            quadTreeLevel=stereo_level,
            minimumNumberOfHits=stereo_hits)

        if assign_stereo_hits:
            module_list.append(last_tracking_module)
            last_tracking_module = cdc_stereo_combiner

        if output_track_cands_store_array_name is not None:
            last_tracking_module.param({'WriteGFTrackCands': True,
                                        'GFTrackCandsStoreArrayName': output_track_cands_store_array_name})

        module_list.append(last_tracking_module)

        super(CDCLegendreTrackFinder, self).__init__(modules=module_list)


class CDCSegmentPairAutomatonFinder(metamodules.WrapperModule):

    def __init__(self, output_track_cands_store_array_name=None,
                 input_track_cands_store_vector_name="CDCTrackVector",
                 input_segments_store_vector_name="CDCRecoSegment2DVector"):

        segment_pair_finder_module = StandardEventGenerationRun.get_basf2_module(
            "TrackFinderCDCSegmentPairAutomatonDev",
            SegmentsStoreObjName=input_segments_store_vector_name,
            WriteGFTrackCands=False,
            TracksStoreObjNameIsInput=True,
            TracksStoreObjName=input_track_cands_store_vector_name
        )

        if output_track_cands_store_array_name is not None:
            segment_pair_finder_module.param({'WriteGFTrackCands': True,
                                              'GFTrackCandsStoreArrayName': output_track_cands_store_array_name})

        super(CDCSegmentPairAutomatonFinder, self).__init__(segment_pair_finder_module)


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

    def __init__(self, output_track_cands_store_array_name=None,
                 tmva_cut=0.2,
                 output_segments_store_vector_name="CDCRecoSegment2DVector"):

        local_track_finder_module = StandardEventGenerationRun.get_basf2_module(
            "SegmentFinderCDCFacetAutomaton",
            SegmentOrientation="outwards",
            ClusterFilter="tmva",
            ClusterFilterParameters={
                "cut": str(tmva_cut)},
            SegmentsStoreObjName=output_segments_store_vector_name,
            WriteGFTrackCands=False,
            TracksStoreObjName="__TempCDCTracksVector")

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

    def __init__(self,
                 segment_track_filter_first_step_filter,
                 segment_track_filter_first_step_cut,

                 background_segment_filter,
                 background_segment_cut,

                 new_segment_filter,
                 new_segment_cut,

                 segment_track_filter_second_step_filter,
                 segment_track_filter_second_step_cut,

                 segment_train_filter,

                 segment_information_list_track_filter,

                 track_filter,
                 track_filter_cut,

                 output_track_cands_store_array_name=None,
                 track_cands_store_vector_name="CDCTrackVector",
                 segments_store_vector_name="CDCRecoSegment2DVector"):

        combiner_module = StandardEventGenerationRun.get_basf2_module(
            "SegmentTrackCombinerDev",
            SegmentTrackFilterFirstStepFilter=segment_track_filter_first_step_filter,
            BackgroundSegmentsFilter=background_segment_filter,
            NewSegmentsFilter=new_segment_filter,
            SegmentTrackFilterSecondStepFilter=segment_track_filter_second_step_filter,
            SegmentTrainFilter=segment_train_filter,
            SegmentInformationListTrackFilter=segment_information_list_track_filter,
            TrackFilter=track_filter,
            WriteGFTrackCands=False,
            TracksStoreObjNameIsInput=True,
            SegmentsStoreObjName=segments_store_vector_name,
            TracksStoreObjName=track_cands_store_vector_name)

        if segment_track_filter_first_step_filter == "tmva":
            combiner_module.param(
                'SegmentTrackFilterFirstStepFilterParameters', {
                    "cut": str(segment_track_filter_first_step_cut)})

        if background_segment_filter == "tmva":
            combiner_module.param(
                'BackgroundSegmentsFilterParameters', {
                    "cut": str(background_segment_cut)})

        if new_segment_filter == "tmva":
            combiner_module.param(
                'NewSegmentsFilterParameters', {
                    "cut": str(new_segment_cut)})

        if segment_track_filter_second_step_filter == "tmva":
            combiner_module.param(
                'SegmentTrackFilterSecondStepFilterParameters', {
                    "cut": str(segment_track_filter_second_step_cut)})

        if track_filter == "tmva":
            combiner_module.param(
                'TrackFilterParameters', {
                    "cut": str(track_filter_cut)})

        if output_track_cands_store_array_name is not None:
            combiner_module.param({'WriteGFTrackCands': True,
                                   'GFTrackCandsStoreArrayName': output_track_cands_store_array_name})

        super(CDCSegmentTrackCombiner, self).__init__(combiner_module)


class CDCTrackQualityAsserter(metamodules.WrapperModule):

    """ Add the TrackQualityAsserterCDC module to the path

    Attributes
    ----------
    If output_track_cands_store_array_name is None, do not write out the track cands to a genfit store array (default)
    With track_cands_store_vector_name you can control the input vector of the CDCTracks. Be aware that the content
    of this vector will be replaced by the output of this module
    With segments_store_vector_name you can control the input vector of the CDCRecoSegments2D. Be aware that the content
    of this vector will be replaced by the output of this module
    With the other parameters you can control the filters of the SegmentTrackCombiner
    """

    def __init__(self,
                 output_track_cands_store_array_name=None,
                 minimal_perp_s_cut=1,
                 track_cands_store_vector_name="CDCTrackVector"):

        module = StandardEventGenerationRun.get_basf2_module(
            "TrackQualityAsserterCDC",
            WriteGFTrackCands=False,
            TracksStoreObjNameIsInput=True,
            TracksStoreObjName=track_cands_store_vector_name)

        if output_track_cands_store_array_name is not None:
            module.param({'WriteGFTrackCands': True,
                          'GFTrackCandsStoreArrayName': output_track_cands_store_array_name})

        metamodules.WrapperModule.__init__(self, module)


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
            use_svd=False,
            queue=None):

        if queue is not None:
            queue.put(self.__class__.__name__ + "_output_file_name", output_file_name)

        from tracking.validation.module import SeparatedTrackingValidationModule

        mc_track_matcher_module = CDCMCMatcher(track_cands_store_array_name=track_candidates_store_array_name,
                                               use_cdc=use_cdc, use_pxd=use_pxd, use_svd=use_svd)

        validation_module = SeparatedTrackingValidationModule(
            name="",
            contact="",
            output_file_name=output_file_name,
            trackCandidatesColumnName=track_candidates_store_array_name,
            expert_level=2)

        super(CDCValidation, self).__init__(modules=[mc_track_matcher_module, validation_module])


class CDCMCFinder(metamodules.WrapperModule):

    def __init__(
            self,
            use_cdc=True,
            use_svd=False,
            use_pxd=False,
            only_primaries=False,
            track_candidates_store_array_name="MCTrackCands"):
        mc_track_finder_module = StandardEventGenerationRun.get_basf2_module(
            'TrackFinderMCTruth',
            UseCDCHits=use_cdc,
            UseSVDHits=use_svd,
            UsePXDHits=use_pxd,
            GFTrackCandidatesColName=track_candidates_store_array_name)

        if only_primaries:
            mc_track_finder_module.param("WhichParticles", ["primary"])
        else:
            mc_track_finder_module.param("WhichParticles", [])

        metamodules.WrapperModule.__init__(self, mc_track_finder_module)


class CDCRecoFitter(metamodules.PathModule):

    def __init__(self, setup_geometry=True, fit_geometry="Geant4",
                 input_track_cands_store_array_name="TrackCands",
                 output_tracks_store_array_name="GF2Tracks",
                 pdg_code=211, vxdParams=None, use_filter="kalman"):

        setup_genfit_extrapolation_module = StandardEventGenerationRun.get_basf2_module('SetupGenfitExtrapolation',
                                                                                        whichGeometry=fit_geometry)

        reco_track_creator_module = StandardEventGenerationRun.get_basf2_module(
            "RecoTrackCreator",
            recoTracksStoreArrayName=output_tracks_store_array_name,
            trackCandidatesStoreArrayName=input_track_cands_store_array_name)

        usedCDCMeasurementCreators = {"RecoHitCreator": {}}
        usedSVDMeasurementCreators = {"RecoHitCreator": {}}
        usedPXDMeasurementCreators = {"RecoHitCreator": {}}
        usedAdditionalMeasurementCreators = {}

        if vxdParams is not None:
            usedSVDMeasurementCreators.update({"MomentumEstimationCreator": vxdParams})
            usedPXDMeasurementCreators.update({"MomentumEstimationCreator": vxdParams})

        measurement_creator_module = StandardEventGenerationRun.get_basf2_module(
            "MeasurementCreator",
            usedCDCMeasurementCreators=usedCDCMeasurementCreators,
            usedSVDMeasurementCreators=usedSVDMeasurementCreators,
            usedPXDMeasurementCreators=usedPXDMeasurementCreators,
            recoTracksStoreArrayName=output_tracks_store_array_name,
            usedAdditionalMeasurementCreators=usedAdditionalMeasurementCreators)

        if use_filter == "daf":
            reco_fitter_module = StandardEventGenerationRun.get_basf2_module(
                "DAFRecoFitter",
                resortHits=True,
                recoTracksStoreArrayName=output_tracks_store_array_name,
                numberOfFailedHits=5,
                minimumIterations=3,
                maximumIterations=10,
                pdgCodeToUseForFitting=pdg_code)
        elif use_filter == "kalman":
            reco_fitter_module = StandardEventGenerationRun.get_basf2_module(
                "KalmanRecoFitter",
                recoTracksStoreArrayName=output_tracks_store_array_name,
                pdgCodeToUseForFitting=pdg_code)
        elif use_filter == "gbl":
            reco_fitter_module = StandardEventGenerationRun.get_basf2_module(
                "GBLRecoFitter",
                recoTracksStoreArrayName=output_tracks_store_array_name,
                pdgCodeToUseForFitting=pdg_code)

        reco_fitter_module.set_debug_level(basf2.LogLevel.DEBUG)

        track_builder = StandardEventGenerationRun.get_basf2_module(
            'TrackBuilderFromRecoTracks',
            recoTracksStoreArrayName=output_tracks_store_array_name,
            trackCandidatesStoreArrayName=input_track_cands_store_array_name)

        module_list = []
        if setup_geometry:
            module_list.append(setup_genfit_extrapolation_module)

        module_list.append(reco_track_creator_module)
        module_list.append(measurement_creator_module)
        module_list.append(reco_fitter_module)
        module_list.append(track_builder)

        super(CDCRecoFitter, self).__init__(modules=module_list)


class CDCFitter(metamodules.PathModule):

    """ Add the genfit module to te path

    Attributes
    ----------
    setup_geometry: Load the SetupGenfitExtrapolation module
    input_track_cands_store_array_name: store array name for input genfit::TrackCands
    output_tracks_store_array_name: store array name for output G2Tracks

    """

    def __init__(self, setup_geometry=True, fit_geometry="Geant4", build_tracks=True):

        setup_genfit_extrapolation_module = StandardEventGenerationRun.get_basf2_module('SetupGenfitExtrapolation',
                                                                                        whichGeometry=fit_geometry)
        gen_fitter_module = StandardEventGenerationRun.get_basf2_module('GenFitter',
                                                                        PDGCodes=[211],
                                                                        BuildBelle2Tracks=False)

        track_builder = StandardEventGenerationRun.get_basf2_module('TrackBuilder')

        module_list = []
        if setup_geometry:
            module_list.append(setup_genfit_extrapolation_module)

        module_list.append(gen_fitter_module)
        if build_tracks:
            module_list.append(track_builder)

        super(CDCFitter, self).__init__(modules=module_list)


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
