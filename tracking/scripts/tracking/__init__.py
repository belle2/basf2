#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from tracking.path_utils import *


def add_tracking_reconstruction(path, components=None, pruneTracks=False, skipGeometryAdding=False,
                                mcTrackFinding=False, trigger_mode="all", trackFitHypotheses=None,
                                reco_tracks="RecoTracks", prune_temporary_tracks=True, fit_tracks=True,
                                use_second_cdc_hits=False, skipHitPreparerAdding=False):
    """
    This function adds the standard reconstruction modules for tracking
    to a path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param pruneTracks: Delete all hits except the first and the last in the found tracks.
    :param skipGeometryAdding: Advances flag: The tracking modules need the geometry module and will add it,
        if it is not already present in the path. In a setup with multiple (conditional) paths however, it can not
        determine, if the geometry is already loaded. This flag can be used o just turn off the geometry adding at
        all (but you will have to add it on your own then).
    :param skipHitPreparerAdding: Advanced flag: do not add the hit preparation (esp. VXD cluster creation
        modules. This is useful if they have been added before already.
    :param mcTrackFinding: Use the MC track finders instead of the realistic ones.
    :param trigger_mode: For a description of the available trigger modes see add_reconstruction.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    :param prune_temporary_tracks: If false, store all information of the single CDC and VXD tracks before merging.
        If true, prune them.
    :param fit_tracks: If false, the final track find and the TrackCreator module will no be executed
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    :param trackFitHypotheses: Which pdg hypothesis to fit. Defaults to [211, 321, 2212].
    """

    if not is_svd_used(components) and not is_cdc_used(components):
        return

    if not skipGeometryAdding:
        # Add the geometry in all trigger modes if not already in the path
        add_geometry_modules(path, components=components)

    if not skipHitPreparerAdding and trigger_mode in ["all", "hlt"]:
        add_hit_preparation_modules(path, components=components)

    # Material effects for all track extrapolations
    if trigger_mode in ["all", "hlt"] and 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation',
                        energyLossBrems=False, noiseBrems=False)

    if mcTrackFinding:
        # Always add the MC finder in all trigger modes.
        add_mc_track_finding(path, components=components, reco_tracks=reco_tracks,
                             use_second_cdc_hits=use_second_cdc_hits)
    else:
        add_track_finding(path, components=components, trigger_mode=trigger_mode, reco_tracks=reco_tracks,
                          prune_temporary_tracks=prune_temporary_tracks,
                          use_second_cdc_hits=use_second_cdc_hits)

    # Only run the track time extraction on the full reconstruction chain for now. Later, we may
    # consider to do the CDC-hit based method already during the fast reconstruction stage
    if trigger_mode in ["hlt", "all"]:
        add_time_extraction(path, components=components)

    if trigger_mode in ["hlt", "all"]:
        add_mc_matcher(path, components=components, reco_tracks=reco_tracks,
                       use_second_cdc_hits=use_second_cdc_hits)

        if fit_tracks:
            add_track_fit_and_track_creator(path, components=components, pruneTracks=pruneTracks,
                                            trackFitHypotheses=trackFitHypotheses,
                                            reco_tracks=reco_tracks)


def add_time_extraction(path, components=None):
    """
    Add time extraction components via tracking
    """

    if is_cdc_used(components):
        path.add_module("FullGridChi2TrackTimeExtractor")


def add_cr_tracking_reconstruction(path, components=None, prune_tracks=False,
                                   skip_geometry_adding=False, event_time_extraction=True,
                                   data_taking_period="gcr2017", top_in_counter=False,
                                   merge_tracks=False, use_second_cdc_hits=False):
    """
    This function adds the reconstruction modules for cr tracking to a path.

    :param path: The path to which to add the tracking reconstruction modules
    :param data_taking_period: The cosmics generation will be added using the
           parameters, that where used in this period of data taking. The periods can be found in cdc/cr/__init__.py.

    :param components: the list of geometry components in use or None for all components.
    :param prune_tracks: Delete all hits except the first and the last in the found tracks.

    :param skip_geometry_adding: Advanced flag: The tracking modules need the geometry module and will add it,
        if it is not already present in the path. In a setup with multiple (conditional) paths however, it cannot
        determine if the geometry is already loaded. This flag can be used to just turn off the geometry adding
        (but you will have to add it on your own).
    :param event_time_extraction: extract the event time
    :param merge_tracks: The upper and lower half of the tracks should be merged together in one track
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.

    :param top_in_counter: time of propagation from the hit point to the PMT in the trigger counter is subtracted
           (assuming PMT is put at -z of the counter).
    """
    # make sure CDC is used
    if not is_cdc_used(components):
        return

    if not skip_geometry_adding:
        # Add the geometry in all trigger modes if not already in the path
        add_geometry_modules(path, components)

    add_hit_preparation_modules(path, components=components)

    # Material effects for all track extrapolations
    if 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation',
                        energyLossBrems=False, noiseBrems=False)

    # track finding
    add_cr_track_finding(path, reco_tracks="RecoTracks", components=components, data_taking_period=data_taking_period,
                         merge_tracks=merge_tracks, use_second_cdc_hits=use_second_cdc_hits)

    # track fitting
    # if tracks were merged, use the unmerged collection for time extraction
    add_cr_track_fit_and_track_creator(path, components=components, prune_tracks=prune_tracks,
                                       event_timing_extraction=event_time_extraction,
                                       data_taking_period=data_taking_period,
                                       top_in_counter=top_in_counter)

    if merge_tracks:
        # Do also fit the not merged tracks
        add_cr_track_fit_and_track_creator(path, components=components, prune_tracks=prune_tracks,
                                           event_timing_extraction=False,
                                           data_taking_period=data_taking_period, top_in_counter=top_in_counter,
                                           reco_tracks="NonMergedRecoTracks", tracks="NonMergedTracks")


def add_mc_tracking_reconstruction(path, components=None, pruneTracks=False, use_second_cdc_hits=False):
    """
    This function adds the standard reconstruction modules for MC tracking
    to a path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param pruneTracks: Delete all hits expect the first and the last from the found tracks.
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    """
    add_tracking_reconstruction(path,
                                components=components,
                                pruneTracks=pruneTracks,
                                mcTrackFinding=True,
                                use_second_cdc_hits=use_second_cdc_hits)


def add_track_finding(path, components=None, trigger_mode="all", reco_tracks="RecoTracks",
                      prune_temporary_tracks=True, use_second_cdc_hits=False,
                      use_mc_truth=False, svd_ckf_mode="VXDTF2_after", add_both_directions=True):
    """
    Add the CKF to the path with all the track finding related to and needed for it.
    :param path: The path to add the tracking reconstruction modules to
    :param reco_tracks: The store array name where to output all tracks
    :param trigger_mode: For a description of the available trigger modes see add_reconstruction.
    :param use_mc_truth: Use the truth information in the CKF modules
    :param svd_ckf_mode: how to apply the CKF (with VXDTF2 or without). Defaults to "VXDTF2_after".
    :param add_both_directions: Curlers may be found in the wrong orientation by the CDC track finder, so try to
           extrapolate also in the other direction.
    :param use_second_cdc_hits: whether to use the secondary CDC hit during CDC track finding or not
    :param components: the list of geometry components in use or None for all components.
    :param prune_temporary_tracks: If false, store all information of the single CDC and VXD tracks before merging.
        If true, prune them.
    """
    if not is_svd_used(components) and not is_cdc_used(components):
        return

    cdc_reco_tracks = "CDCRecoTracks"
    if not is_pxd_used(components) and not is_svd_used(components):
        cdc_reco_tracks = reco_tracks

    svd_cdc_reco_tracks = "SVDCDCRecoTracks"
    if not is_pxd_used(components):
        svd_cdc_reco_tracks = reco_tracks

    svd_reco_tracks = "SVDRecoTracks"
    pxd_reco_tracks = "PXDRecoTracks"

    full_reco_tracks = reco_tracks

    latest_reco_tracks = None

    if trigger_mode in ["fast_reco", "all"] and is_cdc_used(components):
        add_cdc_track_finding(path, use_second_hits=use_second_cdc_hits, output_reco_tracks=cdc_reco_tracks)
        latest_reco_tracks = cdc_reco_tracks

    if trigger_mode in ["hlt", "all"] and is_svd_used(components):
        # in case the lastest_reco_tracks is not set and we are in hlt mode, a previous call to
        # this method using trigger_mode = "fast_reco" was done and we have the CDC-only RecoTrack
        # already
        if trigger_mode == "hlt" and latest_reco_tracks is None:
            latest_reco_tracks = cdc_reco_tracks

        add_svd_track_finding(path, components=components, input_reco_tracks=latest_reco_tracks,
                              output_reco_tracks=svd_cdc_reco_tracks, use_mc_truth=use_mc_truth,
                              temporary_reco_tracks=svd_reco_tracks,
                              svd_ckf_mode=svd_ckf_mode, add_both_directions=add_both_directions)
        latest_reco_tracks = svd_cdc_reco_tracks

    if trigger_mode in ["all"] and is_pxd_used(components):
        add_pxd_track_finding(path, components=components, input_reco_tracks=latest_reco_tracks,
                              use_mc_truth=use_mc_truth, output_reco_tracks=full_reco_tracks,
                              temporary_reco_tracks=pxd_reco_tracks,
                              add_both_directions=add_both_directions)

    if trigger_mode in ["all"] and prune_temporary_tracks:
        for temporary_reco_track_name in [pxd_reco_tracks, svd_reco_tracks, cdc_reco_tracks, svd_cdc_reco_tracks]:
            if temporary_reco_track_name != reco_tracks:
                path.add_module('PruneRecoTracks', storeArrayName=temporary_reco_track_name)


def add_cr_track_finding(path, reco_tracks="RecoTracks", components=None, data_taking_period='gcr2017',
                         merge_tracks=True, use_second_cdc_hits=False):
    import cdc.cr as cosmics_setup

    if data_taking_period not in ["phase2", "early_phase3", "phase3"]:
        cosmics_setup.set_cdc_cr_parameters(data_taking_period)

        # track finding
        add_cdc_cr_track_finding(path, merge_tracks=merge_tracks, use_second_cdc_hits=use_second_cdc_hits,
                                 trigger_point=tuple(cosmics_setup.triggerPos))

    else:
        if not is_cdc_used(components):
            B2FATAL("CDC must be in components")

        reco_tracks_from_track_finding = reco_tracks
        if merge_tracks:
            reco_tracks_from_track_finding = "NonMergedRecoTracks"

        cdc_reco_tracks = "CDCRecoTracks"
        if not is_pxd_used(components) and not is_svd_used(components):
            cdc_reco_tracks = reco_tracks_from_track_finding

        svd_cdc_reco_tracks = "SVDCDCRecoTracks"
        if not is_pxd_used(components):
            svd_cdc_reco_tracks = reco_tracks_from_track_finding

        full_reco_tracks = reco_tracks_from_track_finding

        # CDC track finding with default settings
        add_cdc_cr_track_finding(path, merge_tracks=False, use_second_cdc_hits=use_second_cdc_hits,
                                 output_reco_tracks=cdc_reco_tracks)

        latest_reco_tracks = cdc_reco_tracks

        if is_svd_used(components):
            add_svd_track_finding(path, components=components, input_reco_tracks=latest_reco_tracks,
                                  output_reco_tracks=svd_cdc_reco_tracks,
                                  svd_ckf_mode="cosmics", add_both_directions=True)
            latest_reco_tracks = svd_cdc_reco_tracks

        if is_pxd_used(components):
            add_pxd_track_finding(path, components=components, input_reco_tracks=latest_reco_tracks,
                                  output_reco_tracks=full_reco_tracks, add_both_directions=True,
                                  filter_cut=0.01)

        if merge_tracks:
            # merge the tracks together
            path.add_module("CosmicsTrackMerger", inputRecoTracks=reco_tracks_from_track_finding,
                            outputRecoTracks=reco_tracks)


def add_mc_track_finding(path, components=None, reco_tracks="RecoTracks", use_second_cdc_hits=False):
    """
    Add the MC based TrackFinder to the path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    """
    if is_cdc_used(components) or is_pxd_used(components) or is_svd_used(components):
        # find MCTracks in CDC, SVD and PXD (or a subset of it)
        path.add_module('TrackFinderMCTruthRecoTracks',
                        RecoTracksStoreArrayName=reco_tracks,
                        UseSecondCDCHits=use_second_cdc_hits,
                        UsePXDHits=is_pxd_used(components),
                        UseSVDHits=is_svd_used(components),
                        UseCDCHits=is_cdc_used(components))


def add_ckf_based_track_finding(path,
                                reco_tracks="RecoTracks",
                                trigger_mode="all",
                                cdc_reco_tracks="CDCRecoTracks",
                                svd_reco_tracks="SVDRecoTracks",
                                pxd_reco_tracks="PXDRecoTracks",
                                use_mc_truth=False,
                                svd_ckf_mode="VXDTF2_before_with_second_ckf",
                                add_both_directions=True,
                                use_second_cdc_hits=False,
                                components=None):
    """
    Add the CKF to the path with all the track finding related to and needed for it.
    :param path: The path to add the tracking reconstruction modules to
    :param reco_tracks: The store array name where to output all tracks
    :param trigger_mode: For a description of the available trigger modes see add_reconstruction.
    :param cdc_reco_tracks: The store array name where to output/input the cdc tracks
    :param svd_reco_tracks: The store array name where to output the svd tracks
    :param pxd_reco_tracks: The store array name where to output the pxd tracks
    :param use_mc_truth: Use the truth information in the CKF modules
    :param svd_ckf_mode: how to apply the CKF (with VXDTF2 or without)
    :param add_both_directions: Curlers may be found in the wrong orientation by the CDC track finder, so try to
           extrapolate also in the other direction.
    :param use_second_cdc_hits: whether to use the secondary CDC hit during CDC track finding or not
    :param components: the list of geometry components in use or None for all components.
    """
    if not is_svd_used(components):
        raise ValueError("SVD must be present in the components!")

    if is_pxd_used(components):
        svd_cdc_reco_tracks = "SVDCDCRecoTracks"
    else:
        svd_cdc_reco_tracks = reco_tracks

    if trigger_mode in ["fast_reco", "all"]:
        if is_cdc_used(components):
            # First, start with a normal CDC track finding.
            # Otherwise we assume that the tracks are already in this store array
            add_cdc_track_finding(path, reco_tracks=cdc_reco_tracks, use_second_hits=use_second_cdc_hits)

            path.add_module("DAFRecoFitter", recoTracksStoreArrayName=cdc_reco_tracks)

    if use_mc_truth:
        # MC CKF needs MC matching information
        path.add_module("MCRecoTracksMatcher", UsePXDHits=False, UseSVDHits=False, UseCDCHits=True,
                        mcRecoTracksStoreArrayName="MCRecoTracks",
                        prRecoTracksStoreArrayName=cdc_reco_tracks)

    if trigger_mode in ["hlt", "all"]:
        if svd_ckf_mode == "VXDTF2_before":
            add_vxd_track_finding_vxdtf2(path, components=["SVD"], reco_tracks=svd_reco_tracks)
            add_ckf_based_merger(path, cdc_reco_tracks=cdc_reco_tracks, svd_reco_tracks=svd_reco_tracks,
                                 use_mc_truth=use_mc_truth, direction="backward")
            if add_both_directions:
                add_ckf_based_merger(path, cdc_reco_tracks=cdc_reco_tracks, svd_reco_tracks=svd_reco_tracks,
                                     use_mc_truth=use_mc_truth, direction="forward")

        elif svd_ckf_mode == "VXDTF2_before_with_second_ckf":
            add_vxd_track_finding_vxdtf2(path, components=["SVD"], reco_tracks=svd_reco_tracks)
            add_ckf_based_merger(path, cdc_reco_tracks=cdc_reco_tracks, svd_reco_tracks=svd_reco_tracks,
                                 use_mc_truth=use_mc_truth, direction="backward")
            if add_both_directions:
                add_ckf_based_merger(path, cdc_reco_tracks=cdc_reco_tracks, svd_reco_tracks=svd_reco_tracks,
                                     use_mc_truth=use_mc_truth, direction="forward")
            add_svd_ckf(path, cdc_reco_tracks=cdc_reco_tracks, svd_reco_tracks=svd_reco_tracks,
                        use_mc_truth=use_mc_truth, direction="backward")
            if add_both_directions:
                add_svd_ckf(path, cdc_reco_tracks=cdc_reco_tracks, svd_reco_tracks=svd_reco_tracks,
                            use_mc_truth=use_mc_truth, direction="forward", filter_cut=0.01)

        elif svd_ckf_mode == "only_ckf":
            add_svd_ckf(path, cdc_reco_tracks=cdc_reco_tracks, svd_reco_tracks=svd_reco_tracks,
                        use_mc_truth=use_mc_truth, direction="backward")
            if add_both_directions:
                add_svd_ckf(path, cdc_reco_tracks=cdc_reco_tracks, svd_reco_tracks=svd_reco_tracks,
                            use_mc_truth=use_mc_truth, direction="forward", filter_cut=0.01)

        else:
            raise ValueError(f"Do not understand the svd_ckf_mode {svd_ckf_mode}")

        path.add_module("DAFRecoFitter", recoTracksStoreArrayName=svd_reco_tracks)

        # Write out the combinations of tracks
        path.add_module("RelatedTracksCombiner", VXDRecoTracksStoreArrayName=svd_reco_tracks,
                        CDCRecoTracksStoreArrayName=cdc_reco_tracks,
                        recoTracksStoreArrayName=svd_cdc_reco_tracks)

    if trigger_mode in ["all"]:
        if is_pxd_used(components):
            add_pxd_ckf(path, svd_cdc_reco_tracks=svd_cdc_reco_tracks, pxd_reco_tracks=pxd_reco_tracks,
                        use_mc_truth=use_mc_truth)

            path.add_module("RelatedTracksCombiner", CDCRecoTracksStoreArrayName=svd_cdc_reco_tracks,
                            VXDRecoTracksStoreArrayName=pxd_reco_tracks, recoTracksStoreArrayName=reco_tracks)


def add_cdc_track_finding(path, reco_tracks="RecoTracks",
                          with_ca=True, with_single_segment_tracks=True,
                          track_rejecter_after_ca=False,  # run fake rejecter also on full tracks from CA
                          with_clone_rejection=True,
                          clone_rejection_filter="truth",
                          clone_rejection_filter_parameters={
                              "rootFileName": "TrackQualityFilter.root", },
                          mark_clones_as_background=False, delete_clones=False,
                          use_second_hits=False):
    """
    Convenience function for adding all cdc track finder modules
    to the path.

    The result is a StoreArray with name @param reco_tracks full of RecoTracks (not TrackCands any more!).
    Use the GenfitTrackCandidatesCreator Module to convert back.

    :param path: basf2 path
    :param reco_tracks: Name of the output RecoTracks. Defaults to RecoTracks.
    :param use_second_hits: If true, the second hit information will be used in the CDC track finding.
    """

    # Init the geometry for cdc tracking and the hits
    path.add_module("TFCDC_WireHitPreparer",
                    useSecondHits=use_second_hits,
                    flightTimeEstimation="outwards")

    # Constructs clusters and reduce background hits
    path.add_module("TFCDC_ClusterPreparer",
                    ClusterFilter="mva_bkg",
                    ClusterFilterParameters={"cut": 0.2})

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

    if with_ca:
        # Use CA to look for further tracks in combinations of remaining axial-stereo segment pairs
        path.add_module("TFCDC_TrackFinderSegmentPairAutomaton",
                        tracks="CDCTrackVector2")

        # Overwrites the origin CDCTrackVector
        path.add_module("TFCDC_TrackCombiner",
                        inputTracks="CDCTrackVector",
                        secondaryInputTracks="CDCTrackVector2",
                        tracks="CDCTrackVector")

    # Improve the quality of all tracks and output
    path.add_module("TFCDC_TrackQualityAsserter",
                    corrections=[
                        "LayerBreak",
                        "OneSuperlayer",
                        "Small",
                    ])

    if with_ca and with_single_segment_tracks:
        # Add curlers in the axial inner most superlayer
        path.add_module("TFCDC_TrackCreatorSingleSegments",
                        MinimalHitsBySuperLayerId={0: 15})

        if track_rejecter_after_ca:
            path.add_module("TFCDC_TrackRejecter",
                            inputTracks="CDCTrackVector",
                            filter="mva",
                            filterParameters={"cut": 0.1})

    if with_clone_rejection:
        path.add_module("TFCDC_TrackQualityEstimator",
                        filter=clone_rejection_filter,
                        filterParameters=clone_rejection_filter_parameters,
                        markAsBackground=mark_clones_as_background,
                        deleteClones=delete_clones)

    # Export CDCTracks to RecoTracks representation
    path.add_module("TFCDC_TrackExporter",
                    RecoTracksStoreArrayName=reco_tracks)

    # Correct time seed (only necessary for the CDC tracks)
    path.add_module("IPTrackTimeEstimator",
                    useFittedInformation=False,
                    recoTracksStoreArrayName=reco_tracks)

    # run fast t0 estimation from CDC hits only
    path.add_module("CDCHitBasedT0Extraction")


def add_cdc_cr_track_finding(path, reco_tracks="RecoTracks", trigger_point=(0, 0, 0), merge_tracks=True,
                             use_second_cdc_hits=False):
    """
    Convenience function for adding all cdc track finder modules currently dedicated for the CDC-TOP testbeam
    to the path.

    The result is a StoreArray with name @param reco_tracks full of RecoTracks (not TrackCands any more!).

    Arguments
    ---------
    path: basf2.Path
       The path to be filled
    reco_tracks: str
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
                    RecoTracksStoreArrayName=reco_tracks)


def add_vxd_track_finding(path, svd_clusters="", reco_tracks="RecoTracks", components=None, suffix=""):
    """
    Convenience function for adding all vxd track finder modules
    to the path. This is for version1 of the trackfinder

    The result is a StoreArray with name @param reco_tracks full of RecoTracks (not TrackCands any more!).
    Use the GenfitTrackCandidatesCreator Module to convert back.

    :param path: basf2 path
    :param reco_tracks: Name of the output RecoTracks, Defaults to RecoTracks.
    :param components: List of the detector components to be used in the reconstruction. Defaults to None which means all
                components.
    :param suffix: all names of intermediate Storearrays will have the suffix appended. Useful in cases someone needs to put several
                   instances of track finding in one path.
    """

    # Temporary array
    # add a suffix to be able to have different
    vxd_trackcands = '__VXDGFTrackCands' + suffix

    vxd_trackfinder = path.add_module('VXDTF',
                                      GFTrackCandidatesColName=vxd_trackcands)
    vxd_trackfinder.param('svdClustersName', svd_clusters)

    # WARNING: workaround for possible clashes between fitting and VXDTF
    # stays until the redesign of the VXDTF is finished.
    vxd_trackfinder.param('TESTERexpandedTestingRoutines', False)
    if is_pxd_used(components):
        vxd_trackfinder.param('sectorSetup',
                              ['shiftedL3IssueTestVXDStd-moreThan400MeV_PXDSVD',
                               'shiftedL3IssueTestVXDStd-100to400MeV_PXDSVD',
                               'shiftedL3IssueTestVXDStd-25to100MeV_PXDSVD'
                               ])
        vxd_trackfinder.param('tuneCutoffs', 0.22)
    else:
        vxd_trackfinder.param('sectorSetup',
                              ['shiftedL3IssueTestSVDStd-moreThan400MeV_SVD',
                               'shiftedL3IssueTestSVDStd-100to400MeV_SVD',
                               'shiftedL3IssueTestSVDStd-25to100MeV_SVD'
                               ])
        vxd_trackfinder.param('tuneCutoffs', 0.06)

    # Convert VXD trackcands to reco tracks
    path.add_module("RecoTrackCreator", svdHitsStoreArrayName=svd_clusters, trackCandidatesStoreArrayName=vxd_trackcands,
                    recoTracksStoreArrayName=reco_tracks, recreateSortingParameters=True)


def add_vxd_track_finding_vxdtf2(path, svd_clusters="", reco_tracks="RecoTracks", components=None, suffix="",
                                 useTwoStepSelection=True, PXDminSVDSPs=3,
                                 sectormap_file=None, custom_setup_name=None,
                                 filter_overlapping=True, TFstrictSeeding=True, TFstoreSubsets=False,
                                 quality_estimator='tripletFit', use_quality_index_cutter=False,
                                 track_finder_module='TrackFinderVXDCellOMat'):
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
    :param filter_overlapping: DEBUGGING ONLY: Whether to use SVDOverlapResolver, Default: True
    :param TFstrictSeeding: DEBUGGING ONLY: Whether to use strict seeding for paths in the TrackFinder. Default: True
    :param TFstoreSubsets: DEBUGGING ONLY: Whether to store subsets of paths in the TrackFinder. Default: False
    :param quality_estimator: DEBUGGING ONLY: Which QualityEstimator to use.
                              Default: tripletFit ('tripletFit' currently does not work with PXD)
    :param use_quality_index_cutter: DEBUGGING ONLY: Whether to use VXDTrackCandidatesQualityIndexCutter to cut TCs
                                      with QI below 0.1. To be used in conjunction with quality_estimator='mcInfo'.
                                      Default: False
    :param track_finder_module: DEBUGGING ONLY: Which TrackFinder module to use. Default: TrackFinderVXDCellOMat,
                                other option: TrackFinderVXDBasicPathFinder
    """
    ##########################
    # some setting for VXDTF2
    ##########################
    overlap_filter = 'greedy'  # other option is  'hopfield'
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

    nameSPs = 'SpacePoints' + suffix

    pxdSPCreatorName = 'PXDSpacePointCreator' + suffix
    if pxdSPCreatorName not in [e.name() for e in path.modules()]:
        if use_pxd:
            spCreatorPXD = register_module('PXDSpacePointCreator')
            spCreatorPXD.set_name(pxdSPCreatorName)
            spCreatorPXD.param('NameOfInstance', 'PXDSpacePoints')
            spCreatorPXD.param('SpacePoints', nameSPs)
            path.add_module(spCreatorPXD)

    # check for the name instead of the type as the HLT also need those module under (should have different names)
    svdSPCreatorName = 'SVDSpacePointCreator' + suffix
    if svdSPCreatorName not in [e.name() for e in path.modules()]:
        # always use svd!
        spCreatorSVD = register_module('SVDSpacePointCreator')
        spCreatorSVD.set_name(svdSPCreatorName)
        spCreatorSVD.param('OnlySingleClusterSpacePoints', False)
        spCreatorSVD.param('NameOfInstance', 'SVDSpacePoints')
        spCreatorSVD.param('SpacePoints', nameSPs)
        spCreatorSVD.param('SVDClusters', svd_clusters)
        path.add_module(spCreatorSVD)

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

    nameSegNet = 'SegmentNetwork' + suffix
    segNetProducer = register_module('SegmentNetworkProducer')
    segNetProducer.param('CreateNeworks', 3)
    segNetProducer.param('NetworkOutputName', nameSegNet)
    segNetProducer.param('SpacePointsArrayNames', [nameSPs])
    segNetProducer.param('printNetworks', False)
    segNetProducer.param('sectorMapName', custom_setup_name or setup_name)
    segNetProducer.param('addVirtualIP', False)
    segNetProducer.param('observerType', 0)
    path.add_module(segNetProducer)

    #################
    # VXDTF2 Step 2
    # TrackFinder
    #################

    # append a suffix to the storearray name
    nameSPTCs = 'SPTrackCands' + suffix

    trackFinder = register_module(track_finder_module)
    trackFinder.param('NetworkName', nameSegNet)
    trackFinder.param('SpacePointTrackCandArrayName', nameSPTCs)
    trackFinder.param('SpacePoints', nameSPs)
    trackFinder.param('printNetworks', False)
    trackFinder.param('setFamilies', useTwoStepSelection)
    trackFinder.param('selectBestPerFamily', useTwoStepSelection)
    trackFinder.param('xBestPerFamily', 5)
    trackFinder.param('strictSeeding', TFstrictSeeding)
    trackFinder.param('storeSubsets', TFstoreSubsets)
    path.add_module(trackFinder)

    if(useTwoStepSelection):
        subSetModule = register_module('AddVXDTrackCandidateSubSets')
        subSetModule.param('NameSpacePointTrackCands', nameSPTCs)
        path.add_module(subSetModule)

    #################
    # VXDTF2 Step 3
    # Analyzer
    #################

    # When using PXD require at least PXDminSVDSPs SVD SPs for the SPTCs
    if(use_pxd):
        pxdSVDCut = register_module('PXDSVDCut')
        pxdSVDCut.param('minSVDSPs', PXDminSVDSPs)
        pxdSVDCut.param('SpacePointTrackCandsStoreArrayName', nameSPTCs)
        path.add_module(pxdSVDCut)

    # Quality
    qualityEstimator = register_module('QualityEstimatorVXD')
    qualityEstimator.param('EstimationMethod', quality_estimator)
    qualityEstimator.param('SpacePointTrackCandsStoreArrayName', nameSPTCs)
    path.add_module(qualityEstimator)

    if use_quality_index_cutter:
        qualityIndexCutter = register_module('VXDTrackCandidatesQualityIndexCutter')
        qualityIndexCutter.param('minRequiredQuality', 0.1)
        qualityIndexCutter.param('NameSpacePointTrackCands', nameSPTCs)
        path.add_module(qualityIndexCutter)

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
        overlapResolver.param('ResolveMethod', overlap_filter.lower())
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


def is_svd_used(components):
    """Return true, if the SVD is present in the components list"""
    return components is None or 'SVD' in components


def is_pxd_used(components):
    """Return true, if the PXD is present in the components list"""
    return components is None or 'PXD' in components


def is_cdc_used(components):
    """Return true, if the CDC is present in the components list"""
    return components is None or 'CDC' in components


def add_tracking_for_PXDDataReduction_simulation(path, components, use_vxdtf2=True, svd_cluster='__ROIsvdClusters'):
    """
    This function adds the standard reconstruction modules for tracking to be used for the simulation of PXD data reduction
    to a path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components, always exclude the PXD.
    """

    if not is_svd_used(components):
        return

    # Material effects
    if 'SetupGenfitExtrapolation' not in path:
        material_effects = register_module('SetupGenfitExtrapolation')
        material_effects.set_name(
            'SetupGenfitExtrapolationForPXDDataReduction')
        path.add_module(material_effects)

    # SET StoreArray names

    svd_reco_tracks = "__ROIsvdRecoTracks"

    # SVD ONLY TRACK FINDING
    add_vxd_track_finding_vxdtf2(path, components=['SVD'], reco_tracks=svd_reco_tracks, suffix="__ROI",
                                 svd_clusters=svd_cluster)

    # TRACK FITTING

    # track fitting
    dafRecoFitter = register_module("DAFRecoFitter")
    dafRecoFitter.set_name("SVD-only DAFRecoFitter")
    dafRecoFitter.param('recoTracksStoreArrayName', svd_reco_tracks)
    dafRecoFitter.param('svdHitsStoreArrayName', svd_cluster)
    path.add_module(dafRecoFitter)


def add_vxd_standalone_cosmics_finder(
        path,
        reco_tracks="RecoTracks",
        pxd_spacepoints_name="PXDSpacePoints",
        svd_spacepoints_name="SVDSpacePoints",
        quality_cut=0.0001,
        min_sps=3,
        max_rejected_sps=5):
    """
    Convenience function for adding VXD standalone cosmics track finding for B = 0 Tesla
    to the path.

    The result is a StoreArray with name @param reco_tracks containing one or zero reco tracks per event.
    This track candidates have an arbitrary but large momentum seed in the direction of the fitted line.
    The position and momentum seed is obtained using a principal component analysis method.

    :param path: basf2 path
    :param reco_tracks: Name of the output RecoTracks; defaults to RecoTracks.
    :param spacepoints_name: name of store array containing the spacepoints; defaults to SpacePoints
    :param quality_cut: Cut on the chi squared value of the line fit; candidates with values above the cut will be
                        rejected; defaults to 0.0001
    :param min_sps: Minimal number of SpacePoints required to build a track candidate; defaults to 3;
    :param max_rejected_sps: Maximal number of retries to refit a track after the worst spacepoint was removed;
                             defaults to 5;
    """

    sp_creator_pxd = register_module('PXDSpacePointCreator')
    sp_creator_pxd.param('SpacePoints', pxd_spacepoints_name)
    path.add_module(sp_creator_pxd)

    # SVDSpacePointCreator is applied in funtion add_svd_reconstruction

    track_finder = register_module('TrackFinderVXDCosmicsStandalone')
    track_finder.param('SpacePointTrackCandArrayName', "")
    track_finder.param('SpacePoints', [pxd_spacepoints_name, svd_spacepoints_name])
    track_finder.param('QualityCut', quality_cut)
    track_finder.param('MinSPs', min_sps)
    track_finder.param('MaxRejectedSPs', max_rejected_sps)
    path.add_module(track_finder)

    converter = register_module('SPTC2RTConverter')
    converter.param('recoTracksStoreArrayName', reco_tracks)
    path.add_module(converter)
