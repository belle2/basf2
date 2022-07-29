#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2

# Many scripts import these functions from `tracking`, so leave these imports here
from tracking.path_utils import (  # noqa
    add_cdc_cr_track_finding,
    add_cdc_track_finding,
    add_cr_track_fit_and_track_creator,
    add_eclcdc_track_finding,
    add_geometry_modules,
    add_hit_preparation_modules,
    add_mc_matcher,
    add_prune_tracks,
    add_pxd_cr_track_finding,
    add_pxd_track_finding,
    add_svd_track_finding,
    add_track_fit_and_track_creator,
    add_prefilter_track_fit_and_track_creator,
    add_postfilter_track_fit,
    add_vxd_track_finding_vxdtf2,
    add_svd_standalone_tracking,
    add_vtx_track_finding,
    add_vtx_track_finding_vxdtf2,
    is_cdc_used,
    is_ecl_used,
    is_pxd_used,
    is_svd_used,
    is_vtx_used,
    use_local_sectormap,
)

from pxd import add_pxd_reconstruction


def add_tracking_reconstruction(path, components=None, pruneTracks=False, skipGeometryAdding=False,
                                mcTrackFinding=False, trackFitHypotheses=None,
                                reco_tracks="RecoTracks", prune_temporary_tracks=True, fit_tracks=True,
                                use_second_cdc_hits=False, skipHitPreparerAdding=False,
                                use_svd_to_cdc_ckf=True, use_ecl_to_cdc_ckf=False,
                                add_cdcTrack_QI=True, add_vxdTrack_QI=False, add_recoTrack_QI=False,
                                pxd_filtering_offline=False, append_full_grid_cdc_eventt0=False,
                                useVTX=False, use_vtx_to_cdc_ckf=True, useVTXClusterShapes=True):
    """
    This function adds the **standard tracking reconstruction** modules
    to a path:

    # . first we find tracks using the CDC hits only, see :ref:`CDC Track Finding<tracking_trackFindingCDC>`
    # . CDC tracks are extrapolated to SVD and SVD hits are attached, see :ref:`CDC to SVD CKF<tracking_cdc2svd_ckf>`
    # . remaining  SVD hits are used to find SVD tracks, see :ref:`SVD Track Finding<tracking_trackFindingSVD>`
    # . SVD tracks are extrapolated to CDC to attach CDC hits, see :ref:`SVD to CDC CKF<tracking_svd2cdc_ckf>`
    # . SVD and CDC tracks are merged and fitted, see :ref:`Track Fitting<tracking_trackFitting>`
    # . merged SVD+CDC tracks are extrapolated to PXD to attach PXD hits, see :ref:`SVD to PXD CKF<tracking_svd2pxd_ckf>`

    .. note::

       PXD hits are not available on HLT. At the end of the tracking chain on HLT we have the\
       :ref:`PXD Region Of Interest Finding<tracking_pxdDataReduction>`, that consists of extrapolating\
       the tracks on the PXD sensors and defining regions in which we expect to find the hit.\
       Only fired pixels inside these regions reach Event Builder 2.

    # . after all the tracks from the IP are found, we look for special classes of tracks,\
    in particular we search for displaced vertices to reconstruct K-short, Lambda and\
    photon-conversions, see :ref:`V0 Finding<tracking_v0Finding>`

    .. note::

       this last step is not run on HLT


    :param path: the path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param pruneTracks: if true, delete all hits except the first and the last in the found tracks.
    :param skipGeometryAdding: (advanced flag) the tracking modules need the geometry module and will add it,
        if it is not already present in the path. In a setup with multiple (conditional) paths however, it can not
        determine, if the geometry is already loaded. This flag can be used o just turn off the geometry adding at
        all (but you will have to add it on your own then).
    :param skipHitPreparerAdding: (advanced flag) if true, do not add the hit preparation (esp. VXD cluster creation
        modules. This is useful if they have been added before already.
    :param mcTrackFinding: if true, use the MC track finders instead of the realistic ones.
    :param reco_tracks: name of the StoreArray where the reco tracks should be stored
    :param prune_temporary_tracks: if false, store all information of the single CDC and VXD tracks before merging.
        If true, prune them.
    :param fit_tracks: if false, the final track find and the TrackCreator module will no be executed
    :param use_second_cdc_hits: if true, the second hit information will be used in the CDC track finding.
    :param trackFitHypotheses: which pdg hypothesis to fit. Defaults to [211, 321, 2212].
    :param use_svd_to_cdc_ckf: if true, add SVD to CDC CKF module.
    :param use_ecl_to_cdc_ckf: if true, add ECL to CDC CKF module.
    :param add_cdcTrack_QI: if true, add the MVA track quality estimation
        to the path that sets the quality indicator property of the found CDC standalone tracks
    :param add_vxdTrack_QI: if true, add the MVA track quality estimation
        to the path that sets the quality indicator property of the found VXDTF2 tracks
        (ATTENTION: Standard triplet QI of VXDTF2 is replaced in this case
        -> setting this option to 'True' will have some influence on the final track collection)
    :param add_recoTrack_QI: if true, add the MVA track quality estimation
        to the path that sets the quality indicator property of all found reco tracks
        (Both other QIs needed as input.)
    :param pxd_filtering_offline: If True, PXD data reduction (ROI filtering) is applied during the track reconstruction.
        The reconstructed SVD/CDC tracks are used to define the ROIs and reject all PXD clusters outside of these.
    :param append_full_grid_cdc_eventt0: If True, the module FullGridChi2TrackTimeExtractor is added to the path
                                      and provides the CDC temporary EventT0.
    :param useVTX: If true, the VTX reconstruction is performed.
    :param use_vtx_to_cdc_ckf: if true, add VTX to CDC CKF module.
    :param useVTXClusterShapes: use the VTX cluster shapes
    """

    add_prefilter_tracking_reconstruction(
        path,
        components=components,
        skipGeometryAdding=skipGeometryAdding,
        mcTrackFinding=mcTrackFinding,
        trackFitHypotheses=trackFitHypotheses,
        reco_tracks=reco_tracks,
        prune_temporary_tracks=prune_temporary_tracks,
        fit_tracks=fit_tracks,
        use_second_cdc_hits=use_second_cdc_hits,
        skipHitPreparerAdding=skipHitPreparerAdding,
        use_svd_to_cdc_ckf=use_svd_to_cdc_ckf,
        use_ecl_to_cdc_ckf=use_ecl_to_cdc_ckf,
        add_cdcTrack_QI=add_cdcTrack_QI,
        add_vxdTrack_QI=add_vxdTrack_QI,
        add_recoTrack_QI=add_recoTrack_QI,
        pxd_filtering_offline=pxd_filtering_offline,
        append_full_grid_cdc_eventt0=append_full_grid_cdc_eventt0,
        useVTX=useVTX,
        use_vtx_to_cdc_ckf=use_vtx_to_cdc_ckf,
        useVTXClusterShapes=useVTXClusterShapes)

    add_postfilter_tracking_reconstruction(path,
                                           components=components,
                                           pruneTracks=pruneTracks,
                                           fit_tracks=fit_tracks,
                                           reco_tracks=reco_tracks,
                                           prune_temporary_tracks=prune_temporary_tracks)


def add_prefilter_tracking_reconstruction(path, components=None, skipGeometryAdding=False,
                                          mcTrackFinding=False, trackFitHypotheses=None, reco_tracks="RecoTracks",
                                          prune_temporary_tracks=True, fit_tracks=True,
                                          use_second_cdc_hits=False, skipHitPreparerAdding=False,
                                          use_svd_to_cdc_ckf=True, use_ecl_to_cdc_ckf=False,
                                          add_cdcTrack_QI=True, add_vxdTrack_QI=False, add_recoTrack_QI=False,
                                          pxd_filtering_offline=False, append_full_grid_cdc_eventt0=False,
                                          useVTX=False, use_vtx_to_cdc_ckf=False, useVTXClusterShapes=True):
    """
    This function adds the tracking reconstruction modules required to calculate HLT filter decision
    to a path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param skipGeometryAdding: Advances flag: The tracking modules need the geometry module and will add it,
        if it is not already present in the path. In a setup with multiple (conditional) paths however, it can not
        determine, if the geometry is already loaded. This flag can be used o just turn off the geometry adding at
        all (but you will have to add it on your own then).
    :param skipHitPreparerAdding: Advanced flag: do not add the hit preparation (esp. VXD cluster creation
        modules. This is useful if they have been added before already.
    :param mcTrackFinding: Use the MC track finders instead of the realistic ones.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    :param prune_temporary_tracks: If false, store all information of the single CDC and VXD tracks before merging.
        If true, prune them.
    :param fit_tracks: If false, the final track find and the TrackCreator module will no be executed
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    :param trackFitHypotheses: Which pdg hypothesis to fit. Defaults to [211, 321, 2212].
    :param use_svd_to_cdc_ckf: if true, add SVD to CDC CKF module.
    :param use_ecl_to_cdc_ckf: if true, add ECL to CDC CKF module.
    :param add_cdcTrack_QI: If true, add the MVA track quality estimation
        to the path that sets the quality indicator property of the found CDC standalone tracks
    :param add_vxdTrack_QI: If true, add the MVA track quality estimation
        to the path that sets the quality indicator property of the found VXDTF2 tracks
        (ATTENTION: Standard triplet QI of VXDTF2 is replaced in this case
        -> setting this option to 'True' will have some influence on the final track collection)
    :param add_recoTrack_QI: If true, add the MVA track quality estimation
        to the path that sets the quality indicator property of all found reco tracks
        (Both other QIs needed as input.)
    :param pxd_filtering_offline: If True, PXD data reduction (ROI filtering) is applied during the track reconstruction.
        The reconstructed SVD/CDC tracks are used to define the ROIs and reject all PXD clusters outside of these.
    :param append_full_grid_cdc_eventt0: If True, the module FullGridChi2TrackTimeExtractor is added to the path
                                      and provides the CDC temporary EventT0.
    :param useVTX: If true, the VTX reconstruction is performed.
    :param use_vtx_to_cdc_ckf: if true, add VTX to CDC CKF module.
    :param useVTXClusterShapes: If true, use cluster shape corrections for hit position finding.
    """

    if not is_svd_used(components) and not is_cdc_used(components) and not is_vtx_used(components):
        return

    if (add_cdcTrack_QI or add_vxdTrack_QI or add_recoTrack_QI) and not fit_tracks:
        b2.B2ERROR("MVA track qualiy indicator requires `fit_tracks` to be enabled. Turning all off.")
        add_cdcTrack_QI = False
        add_vxdTrack_QI = False
        add_recoTrack_QI = False

    if add_recoTrack_QI and (not add_cdcTrack_QI or not add_vxdTrack_QI):
        b2.B2ERROR("RecoTrack qualiy indicator requires CDC and VXD QI as input. Turning it all of.")
        add_cdcTrack_QI = False
        add_vxdTrack_QI = False
        add_recoTrack_QI = False

    if not skipGeometryAdding:
        add_geometry_modules(path, components=components)

    if not skipHitPreparerAdding:
        add_hit_preparation_modules(path, components=components, pxd_filtering_offline=pxd_filtering_offline,
                                    useVTX=useVTX, useVTXClusterShapes=useVTXClusterShapes)

    # Material effects for all track extrapolations
    if 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation',
                        energyLossBrems=False, noiseBrems=False)

    if mcTrackFinding:
        add_mc_track_finding(path, components=components, reco_tracks=reco_tracks,
                             use_second_cdc_hits=use_second_cdc_hits)
    else:
        add_track_finding(path, components=components, reco_tracks=reco_tracks,
                          prune_temporary_tracks=prune_temporary_tracks,
                          use_second_cdc_hits=use_second_cdc_hits,
                          use_svd_to_cdc_ckf=use_svd_to_cdc_ckf,
                          use_ecl_to_cdc_ckf=use_ecl_to_cdc_ckf,
                          add_cdcTrack_QI=add_cdcTrack_QI,
                          add_vxdTrack_QI=add_vxdTrack_QI,
                          pxd_filtering_offline=pxd_filtering_offline,
                          useVTX=useVTX,
                          use_vtx_to_cdc_ckf=use_vtx_to_cdc_ckf)

    # Only run the track time extraction on the full reconstruction chain for now. Later, we may
    # consider to do the CDC-hit based method already during the fast reconstruction stage
    add_time_extraction(path, append_full_grid_cdc_eventt0, components=components)

    add_mc_matcher(path, components=components, reco_tracks=reco_tracks,
                   use_second_cdc_hits=use_second_cdc_hits)

    if fit_tracks:
        add_prefilter_track_fit_and_track_creator(path,
                                                  trackFitHypotheses=trackFitHypotheses,
                                                  reco_tracks=reco_tracks,
                                                  add_mva_quality_indicator=add_recoTrack_QI)


def add_postfilter_tracking_reconstruction(path, components=None, pruneTracks=False, fit_tracks=True, reco_tracks="RecoTracks",
                                           prune_temporary_tracks=True):
    """
    This function adds the tracking reconstruction modules not required to calculate HLT filter
    decision to a path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param pruneTracks: Delete all hits except the first and the last in the found tracks.
    :param fit_tracks: If false, the V0 module module will no be executed
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    :param prune_temporary_tracks: If false, store all information of the single CDC and VXD tracks before merging.
        If true, prune them.
    """

    if fit_tracks:
        add_postfilter_track_fit(path, components=components, pruneTracks=pruneTracks, reco_tracks=reco_tracks)

    if prune_temporary_tracks or pruneTracks:
        path.add_module("PruneRecoHits")

    path.add_module('TrackTimeEstimator')


def add_time_extraction(path, append_full_grid_cdc_eventt0=False, components=None):
    """
    Add time extraction components via tracking

    :param path: The path to add the tracking reconstruction modules to
    :param append_full_grid_cdc_eventt0: If True, the module FullGridChi2TrackTimeExtractor is added to the path
                                      and provides the CDC temporary EventT0.
    :param components: the list of geometry components in use or None for all components.
    """
    if is_svd_used(components):
        path.add_module("SVDEventT0Estimator")

    if is_cdc_used(components) and append_full_grid_cdc_eventt0:
        path.add_module("FullGridChi2TrackTimeExtractor")


def add_cr_tracking_reconstruction(path, components=None, prune_tracks=False,
                                   skip_geometry_adding=False, event_time_extraction=True,
                                   merge_tracks=True, use_second_cdc_hits=False,
                                   useVTX=False):
    """
    This function adds the reconstruction modules for cr tracking to a path.

    :param path: The path to which to add the tracking reconstruction modules

    :param components: the list of geometry components in use or None for all components.
    :param prune_tracks: Delete all hits except the first and the last in the found tracks.

    :param skip_geometry_adding: Advanced flag: The tracking modules need the geometry module and will add it,
        if it is not already present in the path. In a setup with multiple (conditional) paths however, it cannot
        determine if the geometry is already loaded. This flag can be used to just turn off the geometry adding
        (but you will have to add it on your own).
    :param event_time_extraction: extract the event time
    :param merge_tracks: The upper and lower half of the tracks should be merged together in one track
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    :param useVTX: If True, the VTX is used instead of the VXD

    """

    # make sure CDC is used
    if not is_cdc_used(components):
        return

    if not skip_geometry_adding:
        add_geometry_modules(path, components)

    add_hit_preparation_modules(path, components=components, useVTX=useVTX)

    # Material effects for all track extrapolations
    if 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation',
                        energyLossBrems=False, noiseBrems=False)

    # track finding
    add_cr_track_finding(path, reco_tracks="RecoTracks", components=components,
                         merge_tracks=merge_tracks, use_second_cdc_hits=use_second_cdc_hits,
                         useVTX=useVTX)

    # track fitting
    # if tracks were merged, use the unmerged collection for time extraction
    add_cr_track_fit_and_track_creator(path, components=components, prune_tracks=prune_tracks,
                                       event_timing_extraction=event_time_extraction)

    if merge_tracks:
        # Do also fit the not merged tracks
        add_cr_track_fit_and_track_creator(path, components=components, prune_tracks=prune_tracks,
                                           event_timing_extraction=False,
                                           reco_tracks="NonMergedRecoTracks", tracks="NonMergedTracks")


def add_mc_tracking_reconstruction(path, components=None, pruneTracks=False, use_second_cdc_hits=False, useVTX=False,
                                   useVTXClusterShapes=True):
    """
    This function adds the standard reconstruction modules for MC tracking
    to a path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param pruneTracks: Delete all hits expect the first and the last from the found tracks.
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    :param useVTX: If true, the VTX reconstruction is performed.
    :param useVTXClusterShapes: If true, use cluster shape corrections for hit position finding.
    """
    add_tracking_reconstruction(path,
                                components=components,
                                pruneTracks=pruneTracks,
                                mcTrackFinding=True,
                                use_second_cdc_hits=use_second_cdc_hits, useVTX=useVTX,
                                useVTXClusterShapes=useVTXClusterShapes)


def add_track_finding(path, components=None, reco_tracks="RecoTracks",
                      prune_temporary_tracks=True, use_second_cdc_hits=False,
                      use_mc_truth=False, svd_ckf_mode="SVD_after", add_both_directions=True,
                      svd_standalone_mode="VXDTF2",
                      use_svd_to_cdc_ckf=True, use_ecl_to_cdc_ckf=False,
                      add_cdcTrack_QI=True, add_vxdTrack_QI=False,
                      pxd_filtering_offline=False, use_HLT_ROIs=False,
                      useVTX=False, vtx_ckf_mode="VXDTF2_after",
                      use_vtx_to_cdc_ckf=True, use_ckf_based_cdc_vtx_merger=False,):
    """
    Add the CKF to the path with all the track finding related to and needed for it.
    :param path: The path to add the tracking reconstruction modules to
    :param reco_tracks: The store array name where to output all tracks
    :param use_mc_truth: Use the truth information in the CKF modules
    :param svd_ckf_mode: how to apply the CKF (with or without SVD standalone tracking). Defaults to "SVD_after".
    :param add_both_directions: Curlers may be found in the wrong orientation by the CDC track finder, so try to
           extrapolate also in the other direction.
    :param svd_standalone_mode: Which SVD standalone tracking is used.
           Options are "VXDTF2", "SVDHough", "VXDTF2_and_SVDHough", and "SVDHough_and_VXDTF2".
           Defaults to "VXDTF2"
    :param use_second_cdc_hits: whether to use the secondary CDC hit during CDC track finding or not
    :param components: the list of geometry components in use or None for all components.
    :param prune_temporary_tracks: If false, store all information of the single CDC and VXD tracks before merging.
        If true, prune them.
    :param use_svd_to_cdc_ckf: if true, add SVD to CDC CKF module.
    :param use_ecl_to_cdc_ckf: if true, add ECL to CDC CKF module.
    :param useVTX: If true, the VTX reconstruction is performed.
    :param vtx_ckf_mode: how to apply the CKF (with VXDTF2 or without). Defaults to "VXDTF2_after".
    :param use_vtx_to_cdc_ckf: if true, add VTX to CDC CKF module.
    :param use_ckf_based_cdc_vtx_merger: Use the CKF based merger for VTX+CDC tracks or the simple VXDCDCTrackMerger.
    :param add_cdcTrack_QI: If true, add the MVA track quality estimation
        to the path that sets the quality indicator property of the found CDC standalone tracks
    :param add_vxdTrack_QI: If true, add the MVA track quality estimation
        to the path that sets the quality indicator property of the found VXDTF2 tracks
        (ATTENTION: Standard triplet QI of VXDTF2 is replaced in this case
        -> setting this option to 'True' will have some influence on the final track collection)
    :param pxd_filtering_offline: If True, PXD data reduction (ROI filtering) is applied during the track reconstruction.
        The reconstructed SVD/CDC tracks are used to define the ROIs and reject all PXD clusters outside of these.
    :param use_HLT_ROIs: Don't calculate the ROIs here but use the ones from the HLT (does obviously not work for simulation)
    """

    if not is_svd_used(components) and not is_cdc_used(components) and not is_vtx_used(components):
        return

    if use_ecl_to_cdc_ckf and not is_cdc_used(components):
        b2.B2WARNING("ECL CKF cannot be used without CDC. Turning it off.")
        use_ecl_to_cdc_ckf = False

    if use_ecl_to_cdc_ckf and not is_ecl_used(components):
        b2.B2ERROR("ECL CKF cannot be used without ECL. Turning it off.")
        use_ecl_to_cdc_ckf = False

    # register EventTrackingInfo
    if 'RegisterEventLevelTrackingInfo' not in path:
        path.add_module('RegisterEventLevelTrackingInfo')

    # output tracks
    cdc_reco_tracks = "CDCRecoTracks"
    svd_cdc_reco_tracks = "SVDCDCRecoTracks"
    vtx_cdc_reco_tracks = "VTXCDCRecoTracks"
    ecl_reco_tracks = "ECLRecoTracks"
    combined_ecl_reco_tracks = "combinedECLRecoTracks"

    # temporary collections
    svd_reco_tracks = "SVDRecoTracks"
    pxd_reco_tracks = "PXDRecoTracks"
    vtx_reco_tracks = "VTXRecoTracks"

    # collections that will be pruned
    temporary_reco_track_list = []

    # the name of the most recent track collection
    latest_reco_tracks = None

    if not is_pxd_used(components) and not useVTX:
        if use_ecl_to_cdc_ckf and is_cdc_used(components):
            combined_ecl_reco_tracks = reco_tracks
        elif (not use_ecl_to_cdc_ckf) and is_svd_used(components):
            svd_cdc_reco_tracks = reco_tracks
        elif (not use_ecl_to_cdc_ckf) and (not is_svd_used(components)) and is_cdc_used(components):
            cdc_reco_tracks = reco_tracks
    elif useVTX and is_vtx_used(components):
        if use_ecl_to_cdc_ckf and is_cdc_used(components):
            combined_ecl_reco_tracks = reco_tracks
        elif (not use_ecl_to_cdc_ckf):
            vtx_cdc_reco_tracks = reco_tracks
    elif useVTX and not is_vtx_used(components) and is_cdc_used(components):
        cdc_reco_tracks = reco_tracks

    if is_cdc_used(components):
        add_cdc_track_finding(path, use_second_hits=use_second_cdc_hits, output_reco_tracks=cdc_reco_tracks,
                              add_mva_quality_indicator=add_cdcTrack_QI)
        temporary_reco_track_list.append(cdc_reco_tracks)
        latest_reco_tracks = cdc_reco_tracks

    if is_svd_used(components) and not useVTX:
        add_svd_track_finding(path, components=components, input_reco_tracks=latest_reco_tracks,
                              output_reco_tracks=svd_cdc_reco_tracks, use_mc_truth=use_mc_truth,
                              temporary_reco_tracks=svd_reco_tracks,
                              svd_ckf_mode=svd_ckf_mode, add_both_directions=add_both_directions,
                              use_svd_to_cdc_ckf=use_svd_to_cdc_ckf, prune_temporary_tracks=prune_temporary_tracks,
                              add_mva_quality_indicator=add_vxdTrack_QI,
                              svd_standalone_mode=svd_standalone_mode)
        temporary_reco_track_list.append(svd_reco_tracks)
        temporary_reco_track_list.append(svd_cdc_reco_tracks)
        latest_reco_tracks = svd_cdc_reco_tracks

    if is_vtx_used(components) and useVTX:
        add_vtx_track_finding(path, components=components, input_reco_tracks=latest_reco_tracks,
                              output_reco_tracks=vtx_cdc_reco_tracks, use_mc_truth=use_mc_truth,
                              temporary_reco_tracks=vtx_reco_tracks,
                              vtx_ckf_mode=vtx_ckf_mode, add_both_directions=add_both_directions,
                              use_vtx_to_cdc_ckf=use_vtx_to_cdc_ckf, prune_temporary_tracks=prune_temporary_tracks,
                              use_ckf_based_cdc_vtx_merger=use_ckf_based_cdc_vtx_merger,
                              add_mva_quality_indicator=False)  # add_subtrack_mva_quality_indicators)
        temporary_reco_track_list.append(vtx_reco_tracks)
        temporary_reco_track_list.append(vtx_cdc_reco_tracks)
        latest_reco_tracks = vtx_cdc_reco_tracks

    if use_ecl_to_cdc_ckf and is_cdc_used(components):
        add_eclcdc_track_finding(path, components=components, output_reco_tracks=ecl_reco_tracks,
                                 prune_temporary_tracks=prune_temporary_tracks)

        # TODO: add another merging step? (SVD track found by vxdtf2, and CDC track found by ECL CKF)?

        path.add_module("RecoTrackStoreArrayCombiner",
                        Temp1RecoTracksStoreArrayName=latest_reco_tracks,
                        Temp2RecoTracksStoreArrayName=ecl_reco_tracks,
                        recoTracksStoreArrayName=combined_ecl_reco_tracks)
        temporary_reco_track_list.append(ecl_reco_tracks)
        temporary_reco_track_list.append(combined_ecl_reco_tracks)
        latest_reco_tracks = combined_ecl_reco_tracks

    if is_pxd_used(components) and not useVTX:
        if pxd_filtering_offline:
            roiName = "ROIs"
            if not use_HLT_ROIs:
                path.add_module("DAFRecoFitter", recoTracksStoreArrayName=latest_reco_tracks)

                roiName = "ROIs_offline"
                add_roiFinder(path, reco_tracks=latest_reco_tracks, roiName=roiName)

            pxd_digifilter = b2.register_module('PXDdigiFilter')
            pxd_digifilter.param('ROIidsName', roiName)
            pxd_digifilter.param('PXDDigitsName', 'PXDDigits')
            pxd_digifilter.param('PXDDigitsInsideROIName', 'PXDDigits')
            pxd_digifilter.param('overrideDB', True)
            pxd_digifilter.param('usePXDDataReduction', True)
            path.add_module(pxd_digifilter)

            add_pxd_reconstruction(path)

        add_pxd_track_finding(path, components=components, input_reco_tracks=latest_reco_tracks,
                              use_mc_truth=use_mc_truth, output_reco_tracks=reco_tracks,
                              temporary_reco_tracks=pxd_reco_tracks,
                              add_both_directions=add_both_directions)
        temporary_reco_track_list.append(pxd_reco_tracks)

    if prune_temporary_tracks:
        for temporary_reco_track_name in temporary_reco_track_list:
            if temporary_reco_track_name != reco_tracks:
                path.add_module('PruneRecoTracks', storeArrayName=temporary_reco_track_name)


def add_cr_track_finding(path, reco_tracks="RecoTracks", components=None,
                         merge_tracks=True, use_second_cdc_hits=False,
                         useVTX=False):

    # register EventTrackingInfo
    if 'RegisterEventLevelTrackingInfo' not in path:
        path.add_module('RegisterEventLevelTrackingInfo')

    if not is_cdc_used(components):
        b2.B2FATAL("CDC must be in components")

    reco_tracks_from_track_finding = reco_tracks
    if merge_tracks:
        reco_tracks_from_track_finding = "NonMergedRecoTracks"

    cdc_reco_tracks = "CDCRecoTracks"
    if not is_pxd_used(components) and not is_svd_used(components):
        cdc_reco_tracks = reco_tracks_from_track_finding

    svd_cdc_reco_tracks = "SVDCDCRecoTracks"
    if not is_pxd_used(components):
        svd_cdc_reco_tracks = reco_tracks_from_track_finding

    vtx_cdc_reco_tracks = "VTXCDCRecoTracks"
    if useVTX and not is_vtx_used(components):
        vtx_cdc_reco_tracks = reco_tracks_from_track_finding

    full_reco_tracks = reco_tracks_from_track_finding

    # CDC track finding with default settings
    add_cdc_cr_track_finding(path, merge_tracks=False, use_second_cdc_hits=use_second_cdc_hits,
                             output_reco_tracks=cdc_reco_tracks)

    latest_reco_tracks = cdc_reco_tracks

    if is_svd_used(components) and not useVTX:
        add_svd_track_finding(path, components=components, input_reco_tracks=latest_reco_tracks,
                              output_reco_tracks=svd_cdc_reco_tracks,
                              svd_ckf_mode="cosmics", add_both_directions=True, svd_standalone_mode="VXDTF2")
        latest_reco_tracks = svd_cdc_reco_tracks

    if is_pxd_used(components) and not useVTX:
        add_pxd_cr_track_finding(path, components=components, input_reco_tracks=latest_reco_tracks,
                                 output_reco_tracks=full_reco_tracks, add_both_directions=True,
                                 filter_cut=0.01)

    if is_vtx_used(components) and useVTX:
        add_vtx_track_finding(path, components=components, input_reco_tracks=latest_reco_tracks,
                              output_reco_tracks=vtx_cdc_reco_tracks,
                              vtx_ckf_mode="cosmics", add_both_directions=True)
        latest_reco_tracks = svd_cdc_reco_tracks

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
    if is_cdc_used(components) or is_pxd_used(components) or is_svd_used(components) or is_vtx_used(components):
        # find MCTracks in CDC, SVD, PXD and VTX (or a subset of it)
        path.add_module('TrackFinderMCTruthRecoTracks',
                        RecoTracksStoreArrayName=reco_tracks,
                        UseSecondCDCHits=use_second_cdc_hits,
                        UsePXDHits=is_pxd_used(components),
                        UseSVDHits=is_svd_used(components),
                        UseCDCHits=is_cdc_used(components),
                        UseVTXHits=is_vtx_used(components))


def add_tracking_for_PXDDataReduction_simulation(path, components, svd_cluster='__ROIsvdClusters'):
    """
    This function adds the standard reconstruction modules for tracking to be used for the simulation of PXD data
    reduction to a path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components, always exclude the PXD.
    """

    if not is_svd_used(components):
        return

    # Material effects
    if 'SetupGenfitExtrapolation' not in path:
        material_effects = b2.register_module('SetupGenfitExtrapolation')
        material_effects.set_name(
            'SetupGenfitExtrapolationForPXDDataReduction')
        path.add_module(material_effects)

    # SET StoreArray names
    svd_reco_tracks = "__ROIsvdRecoTracks"

    # SVD ONLY TRACK FINDING
    add_svd_standalone_tracking(path, components=['SVD'], reco_tracks=svd_reco_tracks, suffix="__ROI",
                                svd_clusters=svd_cluster)

    # TRACK FITTING
    dafRecoFitter = b2.register_module("DAFRecoFitter")
    dafRecoFitter.set_name("SVD-only DAFRecoFitter")
    dafRecoFitter.param('recoTracksStoreArrayName', svd_reco_tracks)
    dafRecoFitter.param('svdHitsStoreArrayName', svd_cluster)
    path.add_module(dafRecoFitter)


def add_roiFinder(path, reco_tracks="RecoTracks", roiName="ROIs"):
    """
    Add the ROI finding to the path creating ROIs out of reco tracks by extrapolating them to the PXD volume.
    :param path: Where to add the module to.
    :param reco_tracks: Which tracks to use in the extrapolation step.
    :param roiName: Name of the produced/stored ROIs.
    """

    pxdDataRed = b2.register_module('PXDROIFinder')
    param_pxdDataRed = {
        'recoTrackListName': reco_tracks,
        'PXDInterceptListName': 'PXDIntercepts',
        'ROIListName': roiName,
        'tolerancePhi': 0.15,
        'toleranceZ': 0.5,
        'sigmaSystU': 0.02,
        'sigmaSystV': 0.02,
        'numSigmaTotU': 10,
        'numSigmaTotV': 10,
        'maxWidthU': 0.5,
        'maxWidthV': 0.5,
    }
    pxdDataRed.param(param_pxdDataRed)
    path.add_module(pxdDataRed)


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

    # register EventTrackingInfo
    if 'RegisterEventLevelTrackingInfo' not in path:
        path.add_module('RegisterEventLevelTrackingInfo')

    sp_creator_pxd = b2.register_module('PXDSpacePointCreator')
    sp_creator_pxd.param('SpacePoints', pxd_spacepoints_name)
    path.add_module(sp_creator_pxd)

    # SVDSpacePointCreator is applied in funtion add_svd_reconstruction

    track_finder = b2.register_module('TrackFinderVXDCosmicsStandalone')
    track_finder.param('SpacePointTrackCandArrayName', "")
    track_finder.param('SpacePoints', [pxd_spacepoints_name, svd_spacepoints_name])
    track_finder.param('QualityCut', quality_cut)
    track_finder.param('MinSPs', min_sps)
    track_finder.param('MaxRejectedSPs', max_rejected_sps)
    path.add_module(track_finder)

    converter = b2.register_module('SPTC2RTConverter')
    converter.param('recoTracksStoreArrayName', reco_tracks)
    path.add_module(converter)


def add_vtx_standalone_cosmics_finder(
        path,
        reco_tracks="RecoTracks",
        vtx_spacepoints_name="VTXSpacePoints",
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

    # register EventTrackingInfo
    if 'RegisterEventLevelTrackingInfo' not in path:
        path.add_module('RegisterEventLevelTrackingInfo')

    if 'VTXSpacePointCreator' not in path:
        sp_creator_vtx = b2.register_module('VTXSpacePointCreator')
        sp_creator_vtx.param('SpacePoints', vtx_spacepoints_name)
        path.add_module(sp_creator_vtx)

    # Or is VTXSpacePointCreator already applied in funtion add_vtx_reconstruction?

    track_finder = b2.register_module('TrackFinderVXDCosmicsStandalone')
    track_finder.param('SpacePointTrackCandArrayName', "")
    track_finder.param('SpacePoints', [vtx_spacepoints_name])
    track_finder.param('QualityCut', quality_cut)
    track_finder.param('MinSPs', min_sps)
    track_finder.param('MaxRejectedSPs', max_rejected_sps)
    path.add_module(track_finder)

    converter = b2.register_module('SPTC2RTConverter')
    converter.param('recoTracksStoreArrayName', reco_tracks)
    path.add_module(converter)
