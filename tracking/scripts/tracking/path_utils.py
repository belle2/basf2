##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from pybasf2 import B2WARNING, B2ERROR

from basf2 import register_module
from ckf.path_functions import add_pxd_ckf, add_cosmics_pxd_ckf, add_ckf_based_merger, add_svd_ckf, add_cosmics_svd_ckf, \
    add_vtx_ckf, add_ckf_based_vtx_track_merger, add_cosmics_vtx_ckf
from pxd import add_pxd_reconstruction
from svd import add_svd_reconstruction
from vtx import add_vtx_reconstruction
from tracking.adjustments import adjust_module


def use_local_sectormap(path, pathToLocalSM):
    """
    Helper function that sets up the SectorMapBootstrapModule in that way that a local sectormap will be
    loaded instead the one from the DB. Has to be applied on the path after the SectorMapBootstrap was
    put into the path (usually in add_reconstructin)

    :param path: The path the SectorMapBootstrapModule is in.
    :param pathToLocalSM: the local storage position of the sectormap (including the name)

    """
    B2WARNING("Warning will load local SectorMap from:  " + pathToLocalSM)
    adjust_module(path, 'SectorMapBootstrap', **{"ReadSecMapFromDB": False,
                                                 "ReadSectorMap": True, "SectorMapsInputFile": pathToLocalSM})


def add_geometry_modules(path, components=None):
    """
    Helper function to add the geometry related modules needed for tracking
    to the path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    """
    # check for detector geometry, necessary for track extrapolation in genfit
    if 'Geometry' not in path:
        path.add_module('Geometry', useDB=True)
        if components is not None:
            B2WARNING("Custom detector components specified: Will still build full geometry")

    # Material effects for all track extrapolations
    if 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation',
                        energyLossBrems=False, noiseBrems=False)


def add_hit_preparation_modules(path, components=None, useVTX=False, useVTXClusterShapes=True):
    """
    Helper fucntion to prepare the hit information to be used by tracking.
    """

    # Preparation of the SVD clusters
    if is_svd_used(components) and not useVTX:
        add_svd_reconstruction(path)

    # Preparation of the PXD clusters
    if is_pxd_used(components) and not useVTX:
        add_pxd_reconstruction(path)

    # Preparation of the VTX clusters
    if is_vtx_used(components) and useVTX:
        add_vtx_reconstruction(path, useVTXClusterShapes=useVTXClusterShapes)


def add_track_fit_and_track_creator(path, components=None, pruneTracks=False, trackFitHypotheses=None,
                                    reco_tracks="RecoTracks", add_mva_quality_indicator=False):
    """
    Helper function to add the modules performing the
    track fit, the V0 fit and the Belle2 track creation to the path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param pruneTracks: Delete all hits expect the first and the last from the found tracks.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    :param add_mva_quality_indicator: If true, add the MVA track quality estimation
        to the path that sets the quality indicator property of the found tracks.
    """

    add_prefilter_track_fit_and_track_creator(path,
                                              trackFitHypotheses=trackFitHypotheses,
                                              reco_tracks=reco_tracks,
                                              add_mva_quality_indicator=add_mva_quality_indicator)

    add_postfilter_track_fit(path, components=components, pruneTracks=pruneTracks, reco_tracks=reco_tracks)


def add_prefilter_track_fit_and_track_creator(path, trackFitHypotheses=None,
                                              reco_tracks="RecoTracks", add_mva_quality_indicator=False):
    """
    Helper function to add only the modules required to calculate HLT filter decision:
    performing the track fit and the Belle2 track creation to the path.

    :param path: The path to add the tracking reconstruction modules to
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    :param add_mva_quality_indicator: If true, add the MVA track quality estimation
        to the path that sets the quality indicator property of the found tracks.
    """

    # Correct time seed
    path.add_module("IPTrackTimeEstimator",
                    recoTracksStoreArrayName=reco_tracks, useFittedInformation=False)
    # track fitting
    path.add_module("DAFRecoFitter", recoTracksStoreArrayName=reco_tracks).set_name(
        "Combined_DAFRecoFitter")
    # Add MVA classifier that uses information not included in the calculation of the fit p-value
    # to add a track quality indicator for classification of fake vs. MC-matched tracks
    if add_mva_quality_indicator:
        path.add_module("TrackQualityEstimatorMVA", collectEventFeatures=True)
    # create Belle2 Tracks from the genfit Tracks
    # The following particle hypothesis will be fitted: Pion, Kaon and Proton
    # Muon fit is working but gives very similar as the Pion due to the closeness of masses
    # -> therefore not in the default fit list
    # Electron fit has as systematic bias and therefore not done here. Therefore, pion fits
    # will be used for electrons which gives a better result as GenFit's current electron
    # implementation.
    path.add_module('TrackCreator', recoTrackColName=reco_tracks,
                    pdgCodes=[211, 321, 2212] if not trackFitHypotheses else trackFitHypotheses)


def add_postfilter_track_fit(path, components=None, pruneTracks=False, reco_tracks="RecoTracks"):
    """
    Helper function to add the modules not requred to calcualte HLT filter decision: performing
    the V0 fit to the path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param pruneTracks: Delete all hits expect the first and the last from the found tracks.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    """

    # V0 finding
    path.add_module('V0Finder', RecoTracks=reco_tracks, v0FitterMode=1)

    # prune genfit tracks
    if pruneTracks:
        add_prune_tracks(path, components=components, reco_tracks=reco_tracks)


def add_cr_track_fit_and_track_creator(path, components=None,
                                       data_taking_period='early_phase3', top_in_counter=False,
                                       prune_tracks=False, event_timing_extraction=True,
                                       reco_tracks="RecoTracks", tracks=""):
    """
    Helper function to add the modules performing the cdc cr track fit
    and track creation to the path.

    :param path: The path to which to add the tracking reconstruction modules
    :param data_taking_period: The cosmics generation will be added using the
           parameters, that where used in this period of data taking. The periods can be found in cdc/cr/__init__.py.

    :param components: the list of geometry components in use or None for all components.
    :param reco_tracks: The name of the reco tracks to use
    :param tracks: the name of the output Belle tracks
    :param prune_tracks: Delete all hits expect the first and the last from the found tracks.
    :param event_timing_extraction: extract the event time
    :param top_in_counter: time of propagation from the hit point to the PMT in the trigger counter is subtracted
           (assuming PMT is put at -z of the counter).
    """

    if data_taking_period not in ["phase2", "phase3", "early_phase3"]:
        import cdc.cr as cosmics_setup

        cosmics_setup.set_cdc_cr_parameters(data_taking_period)

        # Time seed
        path.add_module("PlaneTriggerTrackTimeEstimator",
                        recoTracksStoreArrayName=reco_tracks,
                        pdgCodeToUseForEstimation=13,
                        triggerPlanePosition=cosmics_setup.triggerPos,
                        triggerPlaneDirection=cosmics_setup.normTriggerPlaneDirection,
                        useFittedInformation=False)

        # Initial track fitting
        path.add_module("DAFRecoFitter",
                        recoTracksStoreArrayName=reco_tracks,
                        probCut=0.00001,
                        pdgCodesToUseForFitting=13,
                        )

        # Correct time seed
        path.add_module("PlaneTriggerTrackTimeEstimator",
                        recoTracksStoreArrayName=reco_tracks,
                        pdgCodeToUseForEstimation=13,
                        triggerPlanePosition=cosmics_setup.triggerPos,
                        triggerPlaneDirection=cosmics_setup.normTriggerPlaneDirection,
                        useFittedInformation=True,
                        useReadoutPosition=top_in_counter,
                        readoutPosition=cosmics_setup.readOutPos,
                        readoutPositionPropagationSpeed=cosmics_setup.lightPropSpeed
                        )
    else:
        path.add_module("IPTrackTimeEstimator",
                        recoTracksStoreArrayName=reco_tracks, useFittedInformation=False)

    # Track fitting
    path.add_module("DAFRecoFitter",
                    recoTracksStoreArrayName=reco_tracks,
                    pdgCodesToUseForFitting=13,
                    )

    if event_timing_extraction:
        # Extract the time
        path.add_module("FullGridChi2TrackTimeExtractor",
                        RecoTracksStoreArrayName=reco_tracks,
                        GridMaximalT0Value=40,
                        GridMinimalT0Value=-40,
                        GridGridSteps=6
                        )

        # Track fitting
        path.add_module("DAFRecoFitter",
                        # probCut=0.00001,
                        recoTracksStoreArrayName=reco_tracks,
                        pdgCodesToUseForFitting=13,
                        )

    # Create Belle2 Tracks from the genfit Tracks
    path.add_module('TrackCreator',
                    pdgCodes=[13],
                    recoTrackColName=reco_tracks,
                    trackColName=tracks,
                    useClosestHitToIP=True,
                    useBFieldAtHit=True
                    )

    # Prune genfit tracks
    if prune_tracks:
        add_prune_tracks(path=path, components=components,
                         reco_tracks=reco_tracks)


def add_mc_matcher(path, components=None, mc_reco_tracks="MCRecoTracks",
                   reco_tracks="RecoTracks", use_second_cdc_hits=False,
                   split_after_delta_t=-1.0):
    """
    Match the tracks to the MC truth. The matching works based on
    the output of the TrackFinderMCTruthRecoTracks.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param mc_reco_tracks: Name of the StoreArray where the mc reco tracks will be stored
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    :param split_after_delta_t: If positive, split MCRecoTrack into multiple MCRecoTracks if the time
                                distance between two adjecent SimHits is more than the given value
    """
    path.add_module('TrackFinderMCTruthRecoTracks',
                    RecoTracksStoreArrayName=mc_reco_tracks,
                    WhichParticles=[],
                    UseSecondCDCHits=use_second_cdc_hits,
                    UsePXDHits=is_pxd_used(components),
                    UseSVDHits=is_svd_used(components),
                    UseVTXHits=is_vtx_used(components),
                    UseCDCHits=is_cdc_used(components),
                    SplitAfterDeltaT=split_after_delta_t)

    path.add_module('MCRecoTracksMatcher',
                    mcRecoTracksStoreArrayName=mc_reco_tracks,
                    prRecoTracksStoreArrayName=reco_tracks,
                    UsePXDHits=is_pxd_used(components),
                    UseSVDHits=is_svd_used(components),
                    UseVTXHits=is_vtx_used(components),
                    UseCDCHits=is_cdc_used(components))


def add_prune_tracks(path, components=None, reco_tracks="RecoTracks"):
    """
    Adds removal of the intermediate states at each measurement from the fitted tracks.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    """

    # do not add any pruning, if no tracking detectors are in the components
    if components and not ('SVD' in components or 'CDC' in components or 'VTX' in components):
        return

    path.add_module('PruneRecoTracks', storeArrayName=reco_tracks)
    path.add_module("PruneGenfitTracks")


def add_pxd_track_finding(path, components, input_reco_tracks, output_reco_tracks, use_mc_truth=False,
                          add_both_directions=False, temporary_reco_tracks="PXDRecoTracks", **kwargs):
    """Add the pxd track finding to the path"""
    if not is_pxd_used(components):
        return

    if use_mc_truth:
        # MC CKF needs MC matching information
        path.add_module("MCRecoTracksMatcher", UsePXDHits=False,
                        UseSVDHits=is_svd_used(components), UseCDCHits=is_cdc_used(components),
                        mcRecoTracksStoreArrayName="MCRecoTracks",
                        prRecoTracksStoreArrayName=input_reco_tracks)

    add_pxd_ckf(path, svd_cdc_reco_tracks=input_reco_tracks, pxd_reco_tracks=temporary_reco_tracks,
                direction="backward", use_mc_truth=use_mc_truth, **kwargs)

    if add_both_directions:
        add_pxd_ckf(path, svd_cdc_reco_tracks=input_reco_tracks, pxd_reco_tracks=temporary_reco_tracks,
                    direction="forward", use_mc_truth=use_mc_truth, **kwargs)

    path.add_module("RelatedTracksCombiner", CDCRecoTracksStoreArrayName=input_reco_tracks,
                    VXDRecoTracksStoreArrayName=temporary_reco_tracks, recoTracksStoreArrayName=output_reco_tracks)


def add_pxd_cr_track_finding(path, components, input_reco_tracks, output_reco_tracks, use_mc_truth=False,
                             add_both_directions=False, temporary_reco_tracks="PXDRecoTracks", **kwargs):
    """Add the pxd track finding to the path"""
    if not is_pxd_used(components):
        return

    if use_mc_truth:
        # MC CKF needs MC matching information
        path.add_module("MCRecoTracksMatcher", UsePXDHits=False,
                        UseSVDHits=is_svd_used(components), UseCDCHits=is_cdc_used(components),
                        mcRecoTracksStoreArrayName="MCRecoTracks",
                        prRecoTracksStoreArrayName=input_reco_tracks)

    add_cosmics_pxd_ckf(path, svd_cdc_reco_tracks=input_reco_tracks, pxd_reco_tracks=temporary_reco_tracks,
                        direction="backward", use_mc_truth=use_mc_truth, **kwargs)

    if add_both_directions:
        add_cosmics_pxd_ckf(path, svd_cdc_reco_tracks=input_reco_tracks, pxd_reco_tracks=temporary_reco_tracks,
                            direction="forward", use_mc_truth=use_mc_truth, **kwargs)

    path.add_module("RelatedTracksCombiner", CDCRecoTracksStoreArrayName=input_reco_tracks,
                    VXDRecoTracksStoreArrayName=temporary_reco_tracks, recoTracksStoreArrayName=output_reco_tracks)


def add_svd_track_finding(
        path,
        components,
        input_reco_tracks,
        output_reco_tracks,
        svd_ckf_mode="VXDTF2_after",
        use_mc_truth=False,
        add_both_directions=True,
        temporary_reco_tracks="SVDRecoTracks",
        temporary_svd_cdc_reco_tracks="SVDPlusCDCStandaloneRecoTracks",
        use_svd_to_cdc_ckf=True,
        prune_temporary_tracks=True,
        add_mva_quality_indicator=False,
        **kwargs,
):
    """
    Add SVD track finding to the path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: The list of geometry components in use or None for all components.
    :param input_reco_tracks: Name of the StoreArray with the input reco tracks (usually from CDC) that are used in the
           CKF track finding and are merged with the newly found SVD tracks into the ``output_reco_tracks``.
    :param output_reco_tracks: Name of the StoreArray where the reco tracks outputted by the SVD track finding should be
           stored.
    :param svd_ckf_mode: String designating the mode of the CDC-to-SVD CKF, that is how it is combined with the VXDTF2
            standalone track finding. One of "VXDTF2_after", "VXDTF2_before", "VXDTF2_before_with_second_ckf",
            "only_ckf", "VXDTF2_alone", "cosmics".
    :param use_mc_truth: Add mc matching and use the MC information in the CKF (but not in the VXDTF2)
    :param add_both_directions: Whether to add the CKF with both forward and backward extrapolation directions instead
           of just one.
    :param temporary_reco_tracks: Intermediate store array where the SVD tracks from the VXDTF2 standalone track finding
           are stored, before they are merged with CDC tracks and extended via the CKF tracking.
    :param temporary_svd_cdc_reco_tracks: Intermediate store array where the combination of ``temporary_reco_tracks``
           (from SVD) and ``input_reco_tracks`` (from CDC standalone) is stored, before the CKF is applied.
           It is only used if ``use_svd_to_cdc_ckf`` is true. Otherwise, the combination is stored directly in
           ``output_reco_tracks``.
    :param use_svd_to_cdc_ckf: Whether to enable the CKF extrapolation from the SVD into the CDC.
           That CKF application is not affected by ``svd_ckf_mode``.
    :param prune_temporary_tracks: Delete all hits expect the first and last from intermediate track objects.
    :param add_mva_quality_indicator: Add the VVXDQualityEstimatorMVA module to set the quality indicator
           property for tracks from VXDTF2 standalone tracking
           (ATTENTION: Standard triplet QI of VXDTF2 is replaced in this case
           -> setting this option to 'True' will have some influence on the final track collection)
    """

    if not is_svd_used(components):
        return

    if not input_reco_tracks:
        # We do not have an input track store array. So lets just add vxdtf track finding
        add_vxd_track_finding_vxdtf2(path, components=["SVD"], reco_tracks=output_reco_tracks,
                                     add_mva_quality_indicator=add_mva_quality_indicator)
        return

    if use_mc_truth:
        # MC CKF needs MC matching information
        path.add_module("MCRecoTracksMatcher", UsePXDHits=False, UseSVDHits=False,
                        UseCDCHits=is_cdc_used(components),
                        mcRecoTracksStoreArrayName="MCRecoTracks",
                        prRecoTracksStoreArrayName=input_reco_tracks)

    if svd_ckf_mode == "VXDTF2_before":
        add_vxd_track_finding_vxdtf2(path, components=["SVD"], reco_tracks=temporary_reco_tracks,
                                     add_mva_quality_indicator=add_mva_quality_indicator)
        add_ckf_based_merger(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                             use_mc_truth=use_mc_truth, direction="backward", **kwargs)
        if add_both_directions:
            add_ckf_based_merger(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                                 use_mc_truth=use_mc_truth, direction="forward", **kwargs)

    elif svd_ckf_mode == "VXDTF2_before_with_second_ckf":
        add_vxd_track_finding_vxdtf2(path, components=["SVD"], reco_tracks=temporary_reco_tracks,
                                     add_mva_quality_indicator=add_mva_quality_indicator)
        add_ckf_based_merger(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                             use_mc_truth=use_mc_truth, direction="backward", **kwargs)
        if add_both_directions:
            add_ckf_based_merger(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                                 use_mc_truth=use_mc_truth, direction="forward", **kwargs)
        add_svd_ckf(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                    use_mc_truth=use_mc_truth, direction="backward", **kwargs)
        if add_both_directions:
            add_svd_ckf(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                        use_mc_truth=use_mc_truth, direction="forward", filter_cut=0.01, **kwargs)

    elif svd_ckf_mode == "only_ckf":
        add_svd_ckf(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                    use_mc_truth=use_mc_truth, direction="backward", **kwargs)
        if add_both_directions:
            add_svd_ckf(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                        use_mc_truth=use_mc_truth, direction="forward", filter_cut=0.01, **kwargs)

    elif svd_ckf_mode == "VXDTF2_after":
        add_svd_ckf(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                    use_mc_truth=use_mc_truth, direction="backward", **kwargs)
        if add_both_directions:
            add_svd_ckf(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                        use_mc_truth=use_mc_truth, direction="forward", filter_cut=0.01, **kwargs)

        add_vxd_track_finding_vxdtf2(path, components=["SVD"], reco_tracks=temporary_reco_tracks,
                                     add_mva_quality_indicator=add_mva_quality_indicator)
        add_ckf_based_merger(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                             use_mc_truth=use_mc_truth, direction="backward", **kwargs)
        if add_both_directions:
            add_ckf_based_merger(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                                 use_mc_truth=use_mc_truth, direction="forward", **kwargs)

    elif svd_ckf_mode == "VXDTF2_alone":
        add_vxd_track_finding_vxdtf2(path, components=["SVD"], reco_tracks=temporary_reco_tracks,
                                     add_mva_quality_indicator=add_mva_quality_indicator)
        path.add_module('VXDCDCTrackMerger',
                        CDCRecoTrackColName=input_reco_tracks,
                        VXDRecoTrackColName=temporary_reco_tracks)

    elif svd_ckf_mode == "cosmics":
        add_cosmics_svd_ckf(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                            use_mc_truth=use_mc_truth, direction="backward", **kwargs)
        if add_both_directions:
            add_cosmics_svd_ckf(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                                use_mc_truth=use_mc_truth, direction="forward", **kwargs)

    else:
        raise ValueError(f"Do not understand the svd_ckf_mode {svd_ckf_mode}")

    if use_svd_to_cdc_ckf:
        combined_svd_cdc_standalone_tracks = temporary_svd_cdc_reco_tracks
    else:
        combined_svd_cdc_standalone_tracks = output_reco_tracks

        # Write out the combinations of tracks
    path.add_module("RelatedTracksCombiner", VXDRecoTracksStoreArrayName=temporary_reco_tracks,
                    CDCRecoTracksStoreArrayName=input_reco_tracks,
                    recoTracksStoreArrayName=combined_svd_cdc_standalone_tracks)

    if use_svd_to_cdc_ckf:
        path.add_module("ToCDCCKF",
                        inputWireHits="CDCWireHitVector",
                        inputRecoTrackStoreArrayName=combined_svd_cdc_standalone_tracks,
                        relatedRecoTrackStoreArrayName="CKFCDCRecoTracks",
                        relationCheckForDirection="backward",
                        ignoreTracksWithCDChits=True,
                        outputRecoTrackStoreArrayName="CKFCDCRecoTracks",
                        outputRelationRecoTrackStoreArrayName=combined_svd_cdc_standalone_tracks,
                        writeOutDirection="backward",
                        stateBasicFilterParameters={"maximalHitDistance": 0.15},
                        pathFilter="arc_length",
                        maximalLayerJump=4)

        path.add_module("CDCCKFTracksCombiner",
                        CDCRecoTracksStoreArrayName="CKFCDCRecoTracks",
                        VXDRecoTracksStoreArrayName=combined_svd_cdc_standalone_tracks,
                        recoTracksStoreArrayName=output_reco_tracks)

        if prune_temporary_tracks:
            for temp_reco_track in [combined_svd_cdc_standalone_tracks, "CKFCDCRecoTracks"]:
                path.add_module('PruneRecoTracks', storeArrayName=temp_reco_track)


def add_cdc_track_finding(path, output_reco_tracks="RecoTracks", with_ca=False,
                          use_second_hits=False, add_mva_quality_indicator=True,
                          reattach_hits=False):
    """
    Convenience function for adding all cdc track finder modules
    to the path.

    The result is a StoreArray with name @param reco_tracks full of RecoTracks (not TrackCands any more!).
    Use the GenfitTrackCandidatesCreator Module to convert back.

    :param path: basf2 path
    :param output_reco_tracks: Name of the output RecoTracks. Defaults to RecoTracks.
    :param use_second_hits: If true, the second hit information will be used in the CDC track finding.
    :param add_mva_quality_indicator: Add the TFCDC_TrackQualityEstimator module to set the CDC quality
           indicator property of the CDC ``output_reco_tracks``
    :param cdc_quality_estimator_weightfile: Weightfile identifier for the TFCDC_TrackQualityEstimator
    :param reattach_hits: if true, use the ReattachCDCWireHitsToRecoTracks module at the end of the CDC track finding
                          to readd hits with bad ADC or TOT rejected by the TFCDC_WireHitPreparer module.
    """
    # add EventLevelTrackinginfo for logging errors
    if 'RegisterEventLevelTrackingInfo' not in path:
        path.add_module('RegisterEventLevelTrackingInfo')

    # Init the geometry for cdc tracking and the hits and cut low ADC hits
    path.add_module("TFCDC_WireHitPreparer",
                    wirePosition="aligned",
                    useSecondHits=use_second_hits,
                    flightTimeEstimation="outwards",
                    filter="cuts_from_DB")

    # Constructs clusters
    path.add_module("TFCDC_ClusterPreparer",
                    ClusterFilter="all",
                    ClusterFilterParameters={})

    # Find segments within the clusters
    path.add_module("TFCDC_SegmentFinderFacetAutomaton")

    # Find axial tracks
    path.add_module("TFCDC_AxialTrackFinderLegendre")

    # Improve the quality of the axial tracks
    path.add_module("TFCDC_TrackQualityAsserter",
                    corrections=["B2B"])

    # Find the stereo hits to those axial tracks
    path.add_module('TFCDC_StereoHitFinder')

    # Combine segments with axial tracks
    path.add_module('TFCDC_SegmentTrackCombiner',
                    segmentTrackFilter="mva",
                    segmentTrackFilterParameters={"cut": 0.74},
                    trackFilter="mva",
                    trackFilterParameters={"cut": 0.1})

    output_tracks = "CDCTrackVector"

    if with_ca:
        output_tracks = "CombinedCDCTrackVector"
        path.add_module("TFCDC_TrackFinderSegmentPairAutomaton",
                        tracks="CDCTrackVector2")

        # Overwrites the origin CDCTrackVector
        path.add_module("TFCDC_TrackCombiner",
                        inputTracks="CDCTrackVector",
                        secondaryInputTracks="CDCTrackVector2",
                        tracks=output_tracks)

    # Improve the quality of all tracks and output
    path.add_module("TFCDC_TrackQualityAsserter",
                    inputTracks=output_tracks,
                    corrections=[
                        "LayerBreak",
                        "OneSuperlayer",
                        "Small",
                    ])

    if with_ca:
        # Add curlers in the axial inner most superlayer
        path.add_module("TFCDC_TrackCreatorSingleSegments",
                        inputTracks=output_tracks,
                        MinimalHitsBySuperLayerId={0: 15})

    if add_mva_quality_indicator:
        # Add CDC-specific mva method to set the quality indicator for the CDC tracks
        path.add_module(
            "TFCDC_TrackQualityEstimator",
            inputTracks=output_tracks,
            filter='mva',
            filterParameters={"cut": 0.7},
            deleteTracks=True,
            resetTakenFlag=True
        )

    # Export CDCTracks to RecoTracks representation
    path.add_module("TFCDC_TrackExporter",
                    inputTracks=output_tracks,
                    RecoTracksStoreArrayName="CDCRecoTracksBeforeReattaching" if reattach_hits else output_reco_tracks)

    if reattach_hits:
        # The ReattachCDCWireHitsToRecoTracks module (below) requires the SetupGenfitExtrapolation module
        if 'SetupGenfitExtrapolation' not in path:
            # Prepare Genfit extrapolation
            path.add_module('SetupGenfitExtrapolation')

        # Loop over low-ADC/TOT CDCWireHits and RecoTracks and reattach the hits to the tracks if they are close enough
        path.add_module("ReattachCDCWireHitsToRecoTracks",
                        inputRecoTracksStoreArrayName="CDCRecoTracksBeforeReattaching",
                        outputRecoTracksStoreArrayName=output_reco_tracks)

    # Correct time seed (only necessary for the CDC tracks)
    path.add_module("IPTrackTimeEstimator",
                    useFittedInformation=False,
                    recoTracksStoreArrayName=output_reco_tracks)

    # run fast t0 estimation from CDC hits only
    path.add_module("CDCHitBasedT0Extraction")

    # prepare mdst event level info
    path.add_module("CDCTrackingEventLevelMdstInfoFiller")


def add_eclcdc_track_finding(path, components, output_reco_tracks="RecoTracks", prune_temporary_tracks=True, useVTX=False):
    """
    Convenience function for adding all track finder modules to the path that are based on ecl seeds.

    The result is a StoreArray with name @param reco_tracks full of RecoTracks.
    Use the GenfitTrackCandidatesCreator Module to convert back.

    :param path: basf2 path
    :param components: the list of geometry components in use or None for all components.
    :param output_reco_tracks: Name of the output RecoTracks. Defaults to RecoTracks.
    :param pruneTracks: Delete all hits expect the first and the last from the found tracks.
    """
    if not is_cdc_used(components) or not is_ecl_used(components):
        return

    ecl_cdc_reco_tracks = "ECLCDCRecoTracks"

    if not is_svd_used(components):
        ecl_cdc_reco_tracks = output_reco_tracks

    # collections that will be pruned
    temporary_reco_track_list = []

    path.add_module("ToCDCFromEclCKF",
                    inputWireHits="CDCWireHitVector",
                    minimalEnRequirementCluster=0.3,
                    eclSeedRecoTrackStoreArrayName='EclSeedRecoTracks',
                    hitFindingDirection="backward",
                    outputRecoTrackStoreArrayName="CDCRecoTracksFromEcl",
                    outputRelationRecoTrackStoreArrayName="EclSeedRecoTracks",
                    writeOutDirection="forward",
                    stateBasicFilterParameters={"maximalHitDistance": 7.5, "maximalHitDistanceEclSeed": 75.0},
                    stateExtrapolationFilterParameters={"direction": "backward"},
                    pathFilter="arc_length_fromEcl",
                    inputECLshowersStoreArrayName="ECLShowers",
                    trackFindingDirection="backward",
                    setTakenFlag=False,
                    seedComponent="ECL"
                    )

    path.add_module("ToCDCCKF",
                    inputWireHits="CDCWireHitVector",
                    inputRecoTrackStoreArrayName="CDCRecoTracksFromEcl",
                    relatedRecoTrackStoreArrayName=ecl_cdc_reco_tracks,
                    relationCheckForDirection="backward",
                    outputRecoTrackStoreArrayName=ecl_cdc_reco_tracks,
                    outputRelationRecoTrackStoreArrayName="CDCRecoTracksFromEcl",
                    writeOutDirection="backward",
                    stateBasicFilterParameters={"maximalHitDistance": 0.75},
                    stateExtrapolationFilterParameters={"direction": "forward"},
                    pathFilter="arc_length",
                    seedComponent="ECL"
                    )
    # "EclSeedRecoTracks" don't have to be added to the list as these do not contain any hits
    temporary_reco_track_list.append('CDCRecoTracksFromEcl')

    # Do the following modules have to be added as these are executed already after
    # the CDC standalone?
    # If so: they also have to be included in the new SVD->CDC CKF (see add_svd_track_finding(..) above)

    # Correct time seed (only necessary for the CDC tracks)
    # path.add_module("IPTrackTimeEstimator",
    #                useFittedInformation=False,
    #                recoTracksStoreArrayName=ecl_cdc_reco_tracks)

    # run fast t0 estimation from CDC hits only
    # path.add_module("CDCHitBasedT0Extraction")

    # prepare mdst event level info
    # path.add_module("CDCTrackingEventLevelMdstInfoFiller")

    if is_svd_used(components) and not useVTX:
        add_svd_track_finding(path, components=components, input_reco_tracks=ecl_cdc_reco_tracks,
                              output_reco_tracks=output_reco_tracks, use_mc_truth=False,
                              svd_ckf_mode="only_ckf", add_both_directions=False,
                              temporary_reco_tracks="ECLSVDRecoTracks", use_svd_to_cdc_ckf=False,
                              prune_temporary_tracks=prune_temporary_tracks)
        temporary_reco_track_list.append(ecl_cdc_reco_tracks)
        temporary_reco_track_list.append('ECLSVDRecoTracks')

    if is_vtx_used(components) and useVTX:
        add_vtx_track_finding(path, components=components, input_reco_tracks=ecl_cdc_reco_tracks,
                              output_reco_tracks=output_reco_tracks, use_mc_truth=False,
                              vtx_ckf_mode="only_ckf", add_both_directions=False,
                              temporary_reco_tracks="ECLVTXRecoTracks", use_vtx_to_cdc_ckf=False,
                              prune_temporary_tracks=prune_temporary_tracks)
        temporary_reco_track_list.append(ecl_cdc_reco_tracks)
        temporary_reco_track_list.append('ECLVTXRecoTracks')

    if prune_temporary_tracks:
        for temporary_reco_track_name in temporary_reco_track_list:
            if temporary_reco_track_name != output_reco_tracks:
                path.add_module('PruneRecoTracks', storeArrayName=temporary_reco_track_name)


def add_cdc_cr_track_finding(path, output_reco_tracks="RecoTracks", trigger_point=(0, 0, 0), merge_tracks=True,
                             use_second_cdc_hits=False):
    """
    Convenience function for adding all cdc track finder modules currently dedicated for the CDC-TOP testbeam
    to the path.

    The result is a StoreArray with name @param reco_tracks full of RecoTracks (not TrackCands any more!).

    Arguments
    ---------
    path: basf2.Path
       The path to be filled
    output_reco_tracks: str
       Name of the output RecoTracks. Defaults to RecoTracks.
    merge_tracks: bool
       The upper and lower half of the tracks should be merged together in one track
    use_second_hits: bool
       If true, the second hit information will be used in the CDC track finding.
    """

    # Init the geometry for cdc tracking and the hits
    path.add_module("TFCDC_WireHitPreparer",
                    useSecondHits=use_second_cdc_hits,
                    flightTimeEstimation="downwards",
                    triggerPoint=trigger_point)

    # Constructs clusters and reduce background hits
    path.add_module("TFCDC_ClusterPreparer",
                    ClusterFilter="mva_bkg",
                    ClusterFilterParameters={"cut": 0.2})

    # Find segments within the clusters
    path.add_module("TFCDC_SegmentFinderFacetAutomaton",
                    SegmentOrientation="downwards")

    # Find axial tracks
    path.add_module("TFCDC_AxialTrackFinderLegendre")

    # Improve the quality of the axial tracks
    path.add_module("TFCDC_TrackQualityAsserter",
                    corrections=["B2B"])

    # Find the stereo hits to those axial tracks
    path.add_module('TFCDC_StereoHitFinder')

    # Combine segments with axial tracks
    path.add_module('TFCDC_SegmentTrackCombiner',
                    segmentTrackFilter="mva",
                    segmentTrackFilterParameters={"cut": 0.74},
                    trackFilter="mva",
                    trackFilterParameters={"cut": 0.1})

    # Improve the quality of all tracks and output
    path.add_module("TFCDC_TrackQualityAsserter",
                    corrections=["LayerBreak", "OneSuperlayer", "Small"],
                    )

    # Flip track orientation to always point downwards
    path.add_module("TFCDC_TrackOrienter",
                    inputTracks="CDCTrackVector",
                    tracks="OrientedCDCTrackVector",
                    TrackOrientation="downwards",
                    )

    output_tracks = "OrientedCDCTrackVector"

    if merge_tracks:
        # Merge tracks together if needed
        path.add_module("TFCDC_TrackLinker",
                        inputTracks="OrientedCDCTrackVector",
                        tracks="MergedCDCTrackVector",
                        filter="phi",
                        )
        output_tracks = "MergedCDCTrackVector"

        # However, we also want to export the non merged tracks
        # Correct time seed - assumes velocity near light speed
        path.add_module("TFCDC_TrackFlightTimeAdjuster",
                        inputTracks="OrientedCDCTrackVector",
                        )

        # Export CDCTracks to RecoTracks representation
        path.add_module("TFCDC_TrackExporter",
                        inputTracks="OrientedCDCTrackVector",
                        RecoTracksStoreArrayName="NonMergedRecoTracks")

    # Correct time seed - assumes velocity near light speed
    path.add_module("TFCDC_TrackFlightTimeAdjuster",
                    inputTracks=output_tracks,
                    )

    # Export CDCTracks to RecoTracks representation
    path.add_module("TFCDC_TrackExporter",
                    inputTracks=output_tracks,
                    RecoTracksStoreArrayName=output_reco_tracks)

    # run fast t0 estimation from CDC hits only
    path.add_module("CDCHitBasedT0Extraction")


def add_vtx_track_finding(
        path,
        components,
        input_reco_tracks,
        output_reco_tracks,
        vtx_ckf_mode="VXDTF2_after",
        use_mc_truth=False,
        add_both_directions=True,
        temporary_reco_tracks="VTXRecoTracks",
        temporary_vtx_cdc_reco_tracks="VTXPlusCDCStandaloneRecoTracks",
        use_vtx_to_cdc_ckf=True,
        use_ckf_based_cdc_vtx_merger=False,
        prune_temporary_tracks=True,
        add_mva_quality_indicator=False,
        **kwargs,
):
    """
    Add VTX track finding to the path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: The list of geometry components in use or None for all components.
    :param input_reco_tracks: Name of the StoreArray with the input reco tracks (usually from CDC) that are used in the
           CKF track finding and are merged with the newly found VTX tracks into the ``output_reco_tracks``.
    :param output_reco_tracks: Name of the StoreArray where the reco tracks outputted by the VTX track finding should be
           stored.
    :param vtx_ckf_mode: String designating the mode of the CDC-to-VTX CKF, that is how it is combined with the VXDTF2
            standalone track finding. One of "VXDTF2_after", "VXDTF2_before", "VXDTF2_before_with_second_ckf",
            "only_ckf", "VXDTF2_alone", "cosmics".
    :param use_mc_truth: Add mc matching and use the MC information in the CKF (but not in the VXDTF2)
    :param add_both_directions: Whether to add the CKF with both forward and backward extrapolation directions instead
           of just one.
    :param temporary_reco_tracks: Intermediate store array where the VTX tracks from the VXDTF2 standalone track finding
           are stored, before they are merged with CDC tracks and extended via the CKF tracking.
    :param temporary_vtx_cdc_reco_tracks: Intermediate store array where the combination of ``temporary_reco_tracks``
           (from VTX) and ``input_reco_tracks`` (from CDC standalone) is stored, before the CKF is applied.
           It is only used if ``use_vtx_to_cdc_ckf`` is true. Otherwise, the combination is stored directly in
           ``output_reco_tracks``.
    :param use_vtx_to_cdc_ckf: Whether to enable the CKF extrapolation from the VTX into the CDC.
           That CKF application is not affected by ``vtx_ckf_mode``.
    :param use_ckf_based_cdc_vtx_merger: Use the CKF based merger for VTX+CDC tracks or the simple VXDCDCTrackMerger.
    :param prune_temporary_tracks: Delete all hits expect the first and last from intermediate track objects.
    :param add_mva_quality_indicator: Add the VVXDQualityEstimatorMVA module to set the quality indicator
           property for tracks from VXDTF2 standalone tracking
    """

    if not is_vtx_used(components):
        return

    if not input_reco_tracks:
        # We do not have an input track store array. So lets just add vxdtf track finding
        add_vtx_track_finding_vxdtf2(path, components=["VTX"], reco_tracks=output_reco_tracks,
                                     add_mva_quality_indicator=add_mva_quality_indicator)
        return

    if use_mc_truth:
        # MC CKF needs MC matching information
        path.add_module("MCRecoTracksMatcher", UsePXDHits=False, UseSVDHits=False, UseVTXHits=False,
                        UseCDCHits=is_cdc_used(components),
                        mcRecoTracksStoreArrayName="MCRecoTracks",
                        prRecoTracksStoreArrayName=input_reco_tracks)

    if vtx_ckf_mode == "VXDTF2_before":
        add_vtx_track_finding_vxdtf2(path, components=["VTX"], reco_tracks=temporary_reco_tracks,
                                     add_mva_quality_indicator=add_mva_quality_indicator)

        if use_ckf_based_cdc_vtx_merger:
            add_ckf_based_vtx_track_merger(path, cdc_reco_tracks=input_reco_tracks, vtx_reco_tracks=temporary_reco_tracks,
                                           use_mc_truth=use_mc_truth, direction="backward", **kwargs)
            if add_both_directions:
                add_ckf_based_vtx_track_merger(path, cdc_reco_tracks=input_reco_tracks, vtx_reco_tracks=temporary_reco_tracks,
                                               use_mc_truth=use_mc_truth, direction="forward", **kwargs)
        else:
            path.add_module('VXDCDCTrackMerger',
                            CDCRecoTrackColName=input_reco_tracks,
                            VXDRecoTrackColName=temporary_reco_tracks)

    elif vtx_ckf_mode == "VXDTF2_before_with_second_ckf":
        add_vtx_track_finding_vxdtf2(path, components=["VTX"], reco_tracks=temporary_reco_tracks,
                                     add_mva_quality_indicator=add_mva_quality_indicator)

        if use_ckf_based_cdc_vtx_merger:
            add_ckf_based_vtx_track_merger(path, cdc_reco_tracks=input_reco_tracks, vtx_reco_tracks=temporary_reco_tracks,
                                           use_mc_truth=use_mc_truth, direction="backward", **kwargs)
            if add_both_directions:
                add_ckf_based_vtx_track_merger(path, cdc_reco_tracks=input_reco_tracks, vtx_reco_tracks=temporary_reco_tracks,
                                               use_mc_truth=use_mc_truth, direction="forward", **kwargs)
        else:
            path.add_module('VXDCDCTrackMerger',
                            CDCRecoTrackColName=input_reco_tracks,
                            VXDRecoTrackColName=temporary_reco_tracks)

        add_vtx_ckf(path, cdc_reco_tracks=input_reco_tracks, cdc_vtx_reco_tracks=temporary_reco_tracks,
                    use_mc_truth=use_mc_truth, direction="backward",
                    first_filter_cut=0.1, second_filter_cut=0.1, third_filter_cut=0.1, result_filter_cut=0.01,
                    **kwargs)
        if add_both_directions:
            add_vtx_ckf(path, cdc_reco_tracks=input_reco_tracks, cdc_vtx_reco_tracks=temporary_reco_tracks,
                        use_mc_truth=use_mc_truth, direction="forward",
                        first_filter_cut=0.1, second_filter_cut=0.1, third_filter_cut=0.1, result_filter_cut=0.01,
                        **kwargs)

    elif vtx_ckf_mode == "only_ckf":
        add_vtx_ckf(path, cdc_reco_tracks=input_reco_tracks, cdc_vtx_reco_tracks=temporary_reco_tracks,
                    use_mc_truth=use_mc_truth, direction="backward",
                    first_filter_cut=0.1, second_filter_cut=0.1, third_filter_cut=0.1, result_filter_cut=0.01,
                    **kwargs)
        if add_both_directions:
            add_vtx_ckf(path, cdc_reco_tracks=input_reco_tracks, cdc_vtx_reco_tracks=temporary_reco_tracks,
                        use_mc_truth=use_mc_truth, direction="forward",
                        first_filter_cut=0.1, second_filter_cut=0.1, third_filter_cut=0.1, result_filter_cut=0.01,
                        **kwargs)

    elif vtx_ckf_mode == "VXDTF2_after":
        add_vtx_ckf(path, cdc_reco_tracks=input_reco_tracks, cdc_vtx_reco_tracks=temporary_reco_tracks,
                    use_mc_truth=use_mc_truth, direction="backward",
                    first_filter_cut=0.1, second_filter_cut=0.1, third_filter_cut=0.1, result_filter_cut=0.01,
                    **kwargs)
        if add_both_directions:
            add_vtx_ckf(path, cdc_reco_tracks=input_reco_tracks, cdc_vtx_reco_tracks=temporary_reco_tracks,
                        use_mc_truth=use_mc_truth, direction="forward",
                        first_filter_cut=0.1, second_filter_cut=0.1, third_filter_cut=0.1, result_filter_cut=0.01,
                        **kwargs)

        add_vtx_track_finding_vxdtf2(path, components=["VTX"], reco_tracks=temporary_reco_tracks,
                                     add_mva_quality_indicator=add_mva_quality_indicator)

        if use_ckf_based_cdc_vtx_merger:
            add_ckf_based_vtx_track_merger(path, cdc_reco_tracks=input_reco_tracks, vtx_reco_tracks=temporary_reco_tracks,
                                           use_mc_truth=use_mc_truth, direction="backward", **kwargs)
            if add_both_directions:
                add_ckf_based_vtx_track_merger(path, cdc_reco_tracks=input_reco_tracks, vtx_reco_tracks=temporary_reco_tracks,
                                               use_mc_truth=use_mc_truth, direction="forward", **kwargs)
        else:
            path.add_module('VXDCDCTrackMerger',
                            CDCRecoTrackColName=input_reco_tracks,
                            VXDRecoTrackColName=temporary_reco_tracks)

    elif vtx_ckf_mode == "VXDTF2_alone":
        add_vtx_track_finding_vxdtf2(path, components=["VTX"], reco_tracks=temporary_reco_tracks,
                                     add_mva_quality_indicator=add_mva_quality_indicator)
        path.add_module('VXDCDCTrackMerger',
                        CDCRecoTrackColName=input_reco_tracks,
                        VXDRecoTrackColName=temporary_reco_tracks)

    elif vtx_ckf_mode == "cosmics":
        add_cosmics_vtx_ckf(path, cdc_reco_tracks=input_reco_tracks, cdc_vtx_reco_tracks=temporary_reco_tracks,
                            use_mc_truth=use_mc_truth, direction="backward", **kwargs)
        if add_both_directions:
            add_cosmics_vtx_ckf(path, cdc_reco_tracks=input_reco_tracks, cdc_vtx_reco_tracks=temporary_reco_tracks,
                                use_mc_truth=use_mc_truth, direction="forward", **kwargs)

    else:
        raise ValueError(f"Do not understand the vtx_ckf_mode {vtx_ckf_mode}")

    if use_vtx_to_cdc_ckf:
        combined_vtx_cdc_standalone_tracks = temporary_vtx_cdc_reco_tracks
    else:
        combined_vtx_cdc_standalone_tracks = output_reco_tracks

        # Write out the combinations of tracks
    path.add_module("RelatedTracksCombiner", VXDRecoTracksStoreArrayName=temporary_reco_tracks,
                    CDCRecoTracksStoreArrayName=input_reco_tracks,
                    recoTracksStoreArrayName=combined_vtx_cdc_standalone_tracks)

    if use_vtx_to_cdc_ckf:
        path.add_module("ToCDCCKF",
                        inputWireHits="CDCWireHitVector",
                        inputRecoTrackStoreArrayName=combined_vtx_cdc_standalone_tracks,
                        relatedRecoTrackStoreArrayName="CKFCDCRecoTracks",
                        relationCheckForDirection="backward",
                        ignoreTracksWithCDChits=True,
                        outputRecoTrackStoreArrayName="CKFCDCRecoTracks",
                        outputRelationRecoTrackStoreArrayName=combined_vtx_cdc_standalone_tracks,
                        writeOutDirection="backward",
                        stateBasicFilterParameters={"maximalHitDistance": 0.2},
                        pathFilter="arc_length",
                        seedComponent="VTX",
                        maximalLayerJump=4)

        path.add_module("CDCCKFTracksCombiner",
                        CDCRecoTracksStoreArrayName="CKFCDCRecoTracks",
                        VXDRecoTracksStoreArrayName=combined_vtx_cdc_standalone_tracks,
                        recoTracksStoreArrayName=output_reco_tracks)

        if prune_temporary_tracks:
            for temp_reco_track in [combined_vtx_cdc_standalone_tracks, "CKFCDCRecoTracks"]:
                path.add_module('PruneRecoTracks', storeArrayName=temp_reco_track)


def add_vxd_track_finding_vxdtf2(
    path,
    svd_clusters="",
    reco_tracks="RecoTracks",
    components=None,
    suffix="",
    useTwoStepSelection=True,
    PXDminSVDSPs=3,
    sectormap_file=None,
    custom_setup_name=None,
    min_SPTC_quality=0.,
    filter_overlapping=True,
    add_mva_quality_indicator=False,
):
    """
    Convenience function for adding all vxd track finder Version 2 modules
    to the path.

    The result is a StoreArray with name @param reco_tracks full of RecoTracks (not TrackCands any more!).
    Use the GenfitTrackCandidatesCreator Module to convert back.

    :param path: basf2 path
    :param svd_clusters: SVDCluster collection name
    :param reco_tracks: Name of the output RecoTracks, Defaults to RecoTracks.
    :param components: List of the detector components to be used in the reconstruction. Defaults to None which means
                       all components.
    :param suffix: all names of intermediate Storearrays will have the suffix appended. Useful in cases someone needs to
                   put several instances of track finding in one path.
    :param useTwoStepSelection: if True Families will be defined during path creation and will be used to create only
                                the best candidate per family.
    :param PXDminSVDSPs: When using PXD require at least this number of SVD SPs for the SPTCs
    :param sectormap_file: if set to a finite value, a file will be used instead of the sectormap in the database.
    :param custom_setup_name: Set a custom setup name for the tree in the sector map.
    :param min_SPTC_quality: minimal qualityIndicator value to keeps SPTCs after the QualityEstimation.
                                 0 means no cut. Default: 0
    :param filter_overlapping: Whether to use SVDOverlapResolver, Default: True
    :param add_mva_quality_indicator: Whether to use the MVA Quality Estimator module for VXDTF2 tracks to set the
           quality_indicator property of the found ``reco_tracks``. Default: False.
    """
    ##########################
    # some setting for VXDTF2
    ##########################

    # setting different for pxd and svd:
    if is_pxd_used(components):
        setup_name = "SVDPXDDefault"
        db_sec_map_file = "VXDSectorMap_v000.root"
        use_pxd = True
    else:
        setup_name = "SVDOnlyDefault"
        db_sec_map_file = "SVDSectorMap_v000.root"
        use_pxd = False

    #################
    # VXDTF2 Step 0
    # Preparation
    #################

    # setup the event level tracking info to log errors and stuff
    nameTrackingInfoModule = "RegisterEventLevelTrackingInfo" + suffix
    nameEventTrackingInfo = "EventLevelTrackingInfo" + suffix
    if nameTrackingInfoModule not in path:
        # Use modified name of module and created StoreObj as module might be added twice (PXDDataReduction)
        registerEventlevelTrackingInfo = register_module('RegisterEventLevelTrackingInfo')
        registerEventlevelTrackingInfo.set_name(nameTrackingInfoModule)
        registerEventlevelTrackingInfo.param('EventLevelTrackingInfoName', nameEventTrackingInfo)
        path.add_module(registerEventlevelTrackingInfo)

    nameSPs = 'SpacePoints' + suffix

    pxdSPCreatorName = 'PXDSpacePointCreator' + suffix
    if pxdSPCreatorName not in [e.name() for e in path.modules()]:
        if use_pxd:
            spCreatorPXD = register_module('PXDSpacePointCreator')
            spCreatorPXD.set_name(pxdSPCreatorName)
            spCreatorPXD.param('NameOfInstance', 'PXDSpacePoints')
            spCreatorPXD.param('SpacePoints', "PXD" + nameSPs)
            path.add_module(spCreatorPXD)

    # SecMap Bootstrap
    secMapBootStrap = register_module('SectorMapBootstrap')
    secMapBootStrap.param('ReadSectorMap', sectormap_file is not None)  # read from file
    secMapBootStrap.param('ReadSecMapFromDB', sectormap_file is None)  # this will override ReadSectorMap
    secMapBootStrap.param('SectorMapsInputFile', sectormap_file or db_sec_map_file)
    secMapBootStrap.param('SetupToRead', custom_setup_name or setup_name)
    secMapBootStrap.param('WriteSectorMap', False)
    path.add_module(secMapBootStrap)

    ##################
    # VXDTF2 Step 1
    # SegmentNet
    ##################

    spacePointArrayNames = ["SVD" + nameSPs]
    if use_pxd:
        spacePointArrayNames += ["PXD" + nameSPs]

    nameSegNet = 'SegmentNetwork' + suffix

    segNetProducer = register_module('SegmentNetworkProducer')
    segNetProducer.param('NetworkOutputName', nameSegNet)
    segNetProducer.param('SpacePointsArrayNames', spacePointArrayNames)
    segNetProducer.param('sectorMapName', custom_setup_name or setup_name)
    segNetProducer.param('EventLevelTrackingInfoName', nameEventTrackingInfo)
    path.add_module(segNetProducer)

    #################
    # VXDTF2 Step 2
    # TrackFinder
    #################

    # append a suffix to the storearray name
    nameSPTCs = 'SPTrackCands' + suffix

    trackFinder = register_module('TrackFinderVXDCellOMat')
    trackFinder.param('NetworkName', nameSegNet)
    trackFinder.param('SpacePointTrackCandArrayName', nameSPTCs)
    trackFinder.param('printNetworks', False)
    trackFinder.param('setFamilies', useTwoStepSelection)
    trackFinder.param('selectBestPerFamily', useTwoStepSelection)
    trackFinder.param('xBestPerFamily', 30)
    trackFinder.param('EventLevelTrackingInfoName', nameEventTrackingInfo)
    path.add_module(trackFinder)

    if useTwoStepSelection:
        subSetModule = register_module('AddVXDTrackCandidateSubSets')
        subSetModule.param('NameSpacePointTrackCands', nameSPTCs)
        path.add_module(subSetModule)

    #################
    # VXDTF2 Step 3
    # Analyzer
    #################

    # When using PXD require at least PXDminSVDSPs SVD SPs for the SPTCs
    if use_pxd:
        pxdSVDCut = register_module('PXDSVDCut')
        pxdSVDCut.param('minSVDSPs', PXDminSVDSPs)
        pxdSVDCut.param('SpacePointTrackCandsStoreArrayName', nameSPTCs)
        path.add_module(pxdSVDCut)

    if add_mva_quality_indicator:
        path.add_module(
            "VXDQualityEstimatorMVA",
            SpacePointTrackCandsStoreArrayName=nameSPTCs,
        )
    else:
        path.add_module(
            'QualityEstimatorVXD',
            EstimationMethod='tripletFit',
            SpacePointTrackCandsStoreArrayName=nameSPTCs,
        )

    if min_SPTC_quality > 0.:
        qualityIndicatorCutter = register_module('VXDTrackCandidatesQualityIndicatorCutter')
        qualityIndicatorCutter.param('minRequiredQuality', min_SPTC_quality)
        qualityIndicatorCutter.param('NameSpacePointTrackCands', nameSPTCs)
        path.add_module(qualityIndicatorCutter)

    # will discard track candidates (with low quality estimators) if the number of TC is above threshold
    maxCandidateSelection = register_module('BestVXDTrackCandidatesSelector')
    maxCandidateSelection.param('NameSpacePointTrackCands', nameSPTCs)
    maxCandidateSelection.param('NewNameSpacePointTrackCands', nameSPTCs)
    maxCandidateSelection.param('SubsetCreation', False)
    path.add_module(maxCandidateSelection)

    # Properties
    vIPRemover = register_module('SPTCvirtualIPRemover')
    vIPRemover.param('tcArrayName', nameSPTCs)
    # want to remove virtualIP for any track length
    vIPRemover.param('maxTCLengthForVIPKeeping', 0)
    path.add_module(vIPRemover)

    #################
    # VXDTF2 Step 4
    # OverlapFilter
    #################

    if filter_overlapping:
        overlapResolver = register_module('SVDOverlapResolver')
        overlapResolver.param('NameSpacePointTrackCands', nameSPTCs)
        overlapResolver.param('ResolveMethod', 'greedy')  # other option is  'hopfield'
        overlapResolver.param('NameSVDClusters', svd_clusters)
        path.add_module(overlapResolver)

    #################
    # VXDTF2 Step 5
    # Converter
    #################

    momSeedRetriever = register_module('SPTCmomentumSeedRetriever')
    momSeedRetriever.param('tcArrayName', nameSPTCs)
    path.add_module(momSeedRetriever)

    converter = register_module('SPTC2RTConverter')
    converter.param('recoTracksStoreArrayName', reco_tracks)
    converter.param('spacePointsTCsStoreArrayName', nameSPTCs)
    converter.param('svdClustersName', svd_clusters)
    converter.param('svdHitsStoreArrayName', svd_clusters)
    path.add_module(converter)


def add_vtx_track_finding_vxdtf2(
    path,
    vtx_clusters="",
    reco_tracks="RecoTracks",
    components=None,
    suffix="",
    useTwoStepSelection=True,
    sectormap_file=None,
    custom_setup_name=None,
    min_SPTC_quality=0.0,
    filter_overlapping=True,
    add_mva_quality_indicator=False,
    EstimationMethod="tripletFit",
    materialBudgetFactor=1.2,
    maxPt=0.01,
    vxdQualityEstimatarParametersFromDB=True,
    vtx_bg_cut=0.0
):
    """
    Convenience function for adding all vxd track finder Version 2 modules
    to the path. The function only considers VTX hits for VTX standalone
    tracking.

    The result is a StoreArray with name @param reco_tracks full of RecoTracks (not TrackCands any more!).
    Use the GenfitTrackCandidatesCreator Module to convert back.

    :param path: basf2 path
    :param vtx_clusters: VTXCluster collection name
    :param reco_tracks: Name of the output RecoTracks, Defaults to RecoTracks.
    :param components: List of the detector components to be used in the reconstruction. Defaults to None which means
                       all components.
    :param suffix: all names of intermediate Storearrays will have the suffix appended. Useful in cases someone needs to
                   put several instances of track finding in one path.
    :param useTwoStepSelection: if True Families will be defined during path creation and will be used to create only
                                the best candidate per family.
    :param sectormap_file: if set to a finite value, a file will be used instead of the sectormap in the database.
    :param custom_setup_name: Set a custom setup name for the tree in the sector map.
    :param min_SPTC_quality: minimal qualityIndicator value to keeps SPTCs after the QualityEstimation.
                                 0 means no cut. Default: 0
    :param filter_overlapping: Whether to use VTXOverlapResolver, Default: True
    :param add_mva_quality_indicator: Whether to use the MVA Quality Estimator module for VXDTF2 tracks to set the
           quality_indicator property of the found ``reco_tracks``. Default: False.
    :param EstimationMethod: Estimation method for QualityEstimatorVXD. Default: circleFit
    :param materialBudgetFactor: MaterialBudgetFactor is a hyperparameter of TripletFit QE, Default: 50
    :param maxPt: MaxPt is a hyperparameter of TripletFit QE, Default: 0.5
    :param vxdQualityEstimatarParametersFromDB: If True, take TripletFit hyperparameters from DB, otherwise from function arguments
    :param vtx_bg_cut: If positive, VTX background remover gets applied. Valid values in range [0,1].
    """
    ##########################
    # some setting for VXDTF2
    ##########################

    setup_name = "VTXDefault"
    db_sec_map_file = "VTXSectorMaps.root"

    #################
    # VXDTF2 Step 0
    # Preparation
    #################

    # setup the event level tracking info to log errors and stuff
    nameTrackingInfoModule = "RegisterEventLevelTrackingInfo" + suffix
    nameEventTrackingInfo = "EventLevelTrackingInfo" + suffix
    if nameTrackingInfoModule not in path:
        # Use modified name of module and created StoreObj
        registerEventlevelTrackingInfo = register_module('RegisterEventLevelTrackingInfo')
        registerEventlevelTrackingInfo.set_name(nameTrackingInfoModule)
        registerEventlevelTrackingInfo.param('EventLevelTrackingInfoName', nameEventTrackingInfo)
        path.add_module(registerEventlevelTrackingInfo)

    nameSPs = 'SpacePoints' + suffix

    # SecMap Bootstrap
    secMapBootStrap = register_module('SectorMapBootstrap')
    secMapBootStrap.param('ReadSectorMap', sectormap_file is not None)  # read from file
    secMapBootStrap.param('ReadSecMapFromDB', sectormap_file is None)  # this will override ReadSectorMap
    secMapBootStrap.param('SectorMapsInputFile', sectormap_file or db_sec_map_file)
    secMapBootStrap.param('SetupToRead', custom_setup_name or setup_name)
    secMapBootStrap.param('WriteSectorMap', False)
    path.add_module(secMapBootStrap)

    ##################
    # VXDTF2 Step 1
    # SegmentNet
    ##################

    spacePointArrayNames = ["VTX" + nameSPs]

    nameSegNet = 'SegmentNetwork' + suffix

    segNetProducer = register_module('SegmentNetworkProducer')
    segNetProducer.param('NetworkOutputName', nameSegNet)
    segNetProducer.param('SpacePointsArrayNames', spacePointArrayNames)
    segNetProducer.param('sectorMapName', custom_setup_name or setup_name)
    segNetProducer.param('EventLevelTrackingInfoName', nameEventTrackingInfo)
    path.add_module(segNetProducer)

    #################
    # VXDTF2 Step 2
    # TrackFinder
    #################

    # append a suffix to the storearray name
    nameSPTCs = 'SPTrackCands' + suffix

    trackFinder = register_module('TrackFinderVXDCellOMat')
    trackFinder.param('NetworkName', nameSegNet)
    trackFinder.param('SpacePointTrackCandArrayName', nameSPTCs)
    trackFinder.param('printNetworks', False)
    trackFinder.param('setFamilies', useTwoStepSelection)
    trackFinder.param('selectBestPerFamily', useTwoStepSelection)
    trackFinder.param('xBestPerFamily', 30)
    trackFinder.param('EventLevelTrackingInfoName', nameEventTrackingInfo)
    path.add_module(trackFinder)

    if useTwoStepSelection:
        subSetModule = register_module('AddVXDTrackCandidateSubSets')
        subSetModule.param('NameSpacePointTrackCands', nameSPTCs)
        path.add_module(subSetModule)

    #################
    # VXDTF2 Step 3
    # Analyzer
    #################

    if add_mva_quality_indicator:
        path.add_module(
            "VXDQualityEstimatorMVA",
            SpacePointTrackCandsStoreArrayName=nameSPTCs,
        )
    else:
        path.add_module(
            'QualityEstimatorVXDFromDB',
            EstimationMethod=EstimationMethod,
            MaterialBudgetFactor=materialBudgetFactor,
            MaxPt=maxPt,
            SpacePointTrackCandsStoreArrayName=nameSPTCs,
            useDB=vxdQualityEstimatarParametersFromDB,
        )

    if min_SPTC_quality > 0.:
        qualityIndicatorCutter = register_module('VXDTrackCandidatesQualityIndicatorCutter')
        qualityIndicatorCutter.param('minRequiredQuality', min_SPTC_quality)
        qualityIndicatorCutter.param('NameSpacePointTrackCands', nameSPTCs)
        path.add_module(qualityIndicatorCutter)

    # will discard track candidates (with low quality estimators) if the number of TC is above threshold
    maxCandidateSelection = register_module('BestVXDTrackCandidatesSelector')
    maxCandidateSelection.param('NameSpacePointTrackCands', nameSPTCs)
    maxCandidateSelection.param('NewNameSpacePointTrackCands', nameSPTCs)
    maxCandidateSelection.param('SubsetCreation', False)
    path.add_module(maxCandidateSelection)

    # Properties
    vIPRemover = register_module('SPTCvirtualIPRemover')
    vIPRemover.param('tcArrayName', nameSPTCs)
    # want to remove virtualIP for any track length
    vIPRemover.param('maxTCLengthForVIPKeeping', 0)
    path.add_module(vIPRemover)

    #################
    # VXDTF2 Step 4
    # OverlapFilter
    #################

    if filter_overlapping:
        overlapResolver = register_module('VTXOverlapResolver')
        overlapResolver.param('NameSpacePointTrackCands', nameSPTCs)
        overlapResolver.param('ResolveMethod', 'greedy')  # other option is  'hopfield'
        overlapResolver.param('NameVTXClusters', vtx_clusters)
        path.add_module(overlapResolver)

    #################
    # VXDTF2 Step 5
    # Converter
    #################

    momSeedRetriever = register_module('SPTCmomentumSeedRetriever')
    momSeedRetriever.param('tcArrayName', nameSPTCs)
    path.add_module(momSeedRetriever)

    if vtx_bg_cut == 0:
        converter = register_module('SPTC2RTConverter')
        converter.param('recoTracksStoreArrayName', reco_tracks)
        converter.param('spacePointsTCsStoreArrayName', nameSPTCs)
        converter.param('vtxClustersName', vtx_clusters)
        converter.param('vtxHitsStoreArrayName', vtx_clusters)
        path.add_module(converter)

    else:
        B2WARNING("Experimental VTX Background Remover used!")
        from vtx_bgr.path_utils import add_vtx_bg_remover

        reco_tracks_raw = reco_tracks + 'Raw'

        converter = register_module('SPTC2RTConverter')
        converter.param('recoTracksStoreArrayName', reco_tracks_raw)
        converter.param('spacePointsTCsStoreArrayName', nameSPTCs)
        converter.param('vtxClustersName', vtx_clusters)
        converter.param('vtxHitsStoreArrayName', vtx_clusters)
        path.add_module(converter)

        add_vtx_bg_remover(
            path,
            vtx_bg_cut=vtx_bg_cut,
            inputStoreArrayName=reco_tracks_raw,
            outputStoreArrayName=reco_tracks,
        )


def add_simple_vtx_tracking_reconstruction(path, components=['VTX', 'CDC'], pruneTracks=False, skipGeometryAdding=False,
                                           skipHitPreparerAdding=False, mcTrackFinding=False, trackFitHypotheses=None,
                                           reco_tracks="RecoTracks", prune_temporary_tracks=True, fit_tracks=True,
                                           use_second_cdc_hits=False, add_cdcTrack_QI=False,
                                           add_recoTrack_QI=False, useVTXClusterShapes=True):
    """
    This function adds a simplified version of tracking with the VTX to the path. In this case tracking in the
    VTX is performed with the VXDTF2, and afterwards the found tracks are extrapolated to the CDC using the ToCDCCKF.
    No standalone CDC tracking is performed, and also no CKF from CDC to the VTX is employed.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components. Default are VTX and CDC
        as the only two tracking detectors in the VTX setup.
    :param pruneTracks: Delete all hits except the first and the last in the found tracks.
    :param skipGeometryAdding: Advances flag: The tracking modules need the geometry module and will add it,
        if it is not already present in the path. In a setup with multiple (conditional) paths however, it can not
        determine, if the geometry is already loaded. This flag can be used o just turn off the geometry adding at
        all (but you will have to add it on your own then).
    :param skipHitPreparerAdding: Advanced flag: do not add the hit preparation (esp. VXD cluster creation
        modules. This is useful if they have been added before already.
    :param mcTrackFinding: Use the MC track finders instead of the realistic ones.
    :param trackFitHypotheses: Which pdg hypothesis to fit. Defaults to [211, 321, 2212].
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    :param prune_temporary_tracks: If false, store all information of the single CDC and VXD tracks before merging.
        If true, prune them.
    :param fit_tracks: If false, the final track find and the TrackCreator module will no be executed
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    :param add_cdcTrack_QI: If true, add the MVA track quality estimation
        to the path that sets the quality indicator property of the found CDC standalone tracks
    :param add_recoTrack_QI: If true, add the MVA track quality estimation
        to the path that sets the quality indicator property of all found reco tracks
        (Both other QIs needed as input.)
    :param useVTXClusterShapes: If true, use cluster shape corrections for hit position finding.
    """

    if (add_cdcTrack_QI or add_recoTrack_QI) and not fit_tracks:
        B2ERROR("MVA track qualiy indicator requires `fit_tracks` to be enabled. Turning all off.")
        add_cdcTrack_QI = False
        add_recoTrack_QI = False

    if add_recoTrack_QI and not add_cdcTrack_QI:
        B2ERROR("RecoTrack qualiy indicator requires CDC and VXD QI as input. Turning it all of.")
        add_cdcTrack_QI = False
        add_recoTrack_QI = False

    if not skipGeometryAdding:
        add_geometry_modules(path, components=components)

    if not skipHitPreparerAdding:
        add_hit_preparation_modules(path, components=components, useVTX=True,
                                    useVTXClusterShapes=useVTXClusterShapes)

    # Material effects for all track extrapolations
    if 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation', energyLossBrems=False, noiseBrems=False)

    else:
        add_vtx_track_finding_vxdtf2(path, reco_tracks="RecoTracksVTX", components=["VTX"])
        path.add_module("DAFRecoFitter", recoTracksStoreArrayName="RecoTracksVTX")

        path.add_module("TFCDC_WireHitPreparer",
                        wirePosition="aligned",
                        useSecondHits=False,
                        flightTimeEstimation="outwards")
        path.add_module("ToCDCCKF",
                        inputWireHits="CDCWireHitVector",
                        inputRecoTrackStoreArrayName="RecoTracksVTX",
                        relatedRecoTrackStoreArrayName="CKFCDCRecoTracks",
                        relationCheckForDirection="backward",
                        outputRecoTrackStoreArrayName="CKFCDCRecoTracks",
                        outputRelationRecoTrackStoreArrayName="RecoTracksVTX",
                        writeOutDirection="backward",
                        stateBasicFilterParameters={"maximalHitDistance": 0.2},
                        stateExtrapolationFilterParameters={"direction": "forward"},
                        pathFilter="arc_length",
                        seedComponent="VTX")

        path.add_module("RelatedTracksCombiner",
                        CDCRecoTracksStoreArrayName="CKFCDCRecoTracks",
                        VXDRecoTracksStoreArrayName="RecoTracksVTX",
                        recoTracksStoreArrayName="RecoTracks")

        path.add_module("DAFRecoFitter", recoTracksStoreArrayName="RecoTracks")

        path.add_module('TrackCreator', recoTrackColName='RecoTracks')

    add_mc_matcher(path, components=components, reco_tracks=reco_tracks,
                   use_second_cdc_hits=use_second_cdc_hits)

    if fit_tracks:
        add_track_fit_and_track_creator(path, components=components, pruneTracks=pruneTracks,
                                        trackFitHypotheses=trackFitHypotheses,
                                        reco_tracks=reco_tracks,
                                        add_mva_quality_indicator=add_recoTrack_QI)

    if prune_temporary_tracks or pruneTracks:
        path.add_module("PruneRecoHits")


def is_svd_used(components):
    """Return true, if the SVD is present in the components list"""
    return components is None or 'SVD' in components


def is_vtx_used(components):
    """Return true, if the VTX is present in the components list"""
    return components is None or 'VTX' in components


def is_pxd_used(components):
    """Return true, if the PXD is present in the components list"""
    return components is None or 'PXD' in components


def is_cdc_used(components):
    """Return true, if the CDC is present in the components list"""
    return components is None or 'CDC' in components


def is_ecl_used(components):
    """Return true, if the ECL is present in the components list"""
    return components is None or 'ECL' in components
