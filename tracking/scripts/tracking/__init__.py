#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from basf2 import *
from ROOT import Belle2


def is_svd_used(components):
    """Return true, if the SVD is present in the components list"""
    return components is None or 'SVD' in components


def is_pxd_used(components):
    """Return true, if the PXD is present in the components list"""
    return components is None or 'PXD' in components


def is_cdc_used(components):
    """Return true, if the CDC is present in the components list"""
    return components is None or 'CDC' in components


def add_mc_tracking_reconstruction(path, components=None, pruneTracks=False):
    """
    This function adds the standard reconstruction modules for MC tracking
    to a path.
    """
    add_tracking_reconstruction(path,
                                components=components,
                                pruneTracks=pruneTracks,
                                mcTrackFinding=True)


def add_tracking_reconstruction(path, components=None, pruneTracks=False, mcTrackFinding=False):
    """
    This function adds the standard reconstruction modules for tracking
    to a path.  If mcTrackFinding is set it uses MC truth based track finding,
    realistic track finding otherwise.
    """

    if not is_svd_used(components) and not is_cdc_used(components):
        return

    # check for detector geometry, necessary for track extrapolation in genfit
    if 'Geometry' not in path:
        geometry = register_module('Geometry')
        if components:
            geometry.param('components', components)
        path.add_module(geometry)

    # Material effects for all track extrapolations
    if 'SetupGenfitExtrapolation' not in path:
        material_effects = register_module('SetupGenfitExtrapolation')
        path.add_module(material_effects)

    if mcTrackFinding:
        add_mc_track_finding(path, components)
    else:
        add_track_finding(path, components)

    # Match the tracks to the MC truth. The matching works based on
    # the output of the TrackFinderMCTruthRecoTracks.
    mctrackfinder = register_module('TrackFinderMCTruthRecoTracks')
    mctrackfinder.param('RecoTracksStoreArrayName', 'MCRecoTracks')
    mctrackfinder.param('WhichParticles', [])
    path.add_module(mctrackfinder)

    mctrackmatcher = register_module('MCRecoTracksMatcher')
    mctrackmatcher.param('mcRecoTracksStoreArrayName', 'MCRecoTracks')
    mctrackmatcher.param('UsePXDHits', is_pxd_used(components))
    mctrackmatcher.param('UseSVDHits', is_svd_used(components))
    mctrackmatcher.param('UseCDCHits', is_cdc_used(components))
    path.add_module(mctrackmatcher)

    # track fitting
    path.add_module("DAFRecoFitter").set_name("Combined_DAFRecoFitter")

    # create Belle2 Tracks from the genfit Tracks
    path.add_module('TrackCreator')

    # V0 finding
    path.add_module('V0Finder')

    # prune genfit tracks
    if pruneTracks:
        add_prune_tracks(path, components)


def add_prune_tracks(path, components=None):
    """
    Adds removal of the intermediate states at each measurement from the fitted tracks.
    """

    # do not add any pruning, if no tracking detectors are in the components
    if components and not ('SVD' in components or 'CDC' in components):
        return

    path.add_module('PruneRecoTracks')
    path.add_module("PruneGenfitTracks")


def add_track_finding(path, components=None):
    """
    Adds the realistic track finding to the path.
    The result is a StoreArray 'RecoTracks' full of RecoTracks (not TrackCands any more!).
    Use the GenfitTrackCandidatesCreator Module to convert back.
    """
    if not is_svd_used(components) and not is_cdc_used(components):
        return

    use_svd = is_svd_used(components)
    use_cdc = is_cdc_used(components)

    # if only CDC or VXD are used, the track finding result
    # will be directly written to the final RecoTracks array
    # because no merging is required
    cdc_reco_tracks = "RecoTracks"
    vxd_reco_tracks = "RecoTracks"

    # CDC track finder
    if use_cdc:
        if use_svd:
            cdc_reco_tracks = "CDCRecoTracks"

        add_cdc_track_finding(path, reco_tracks=cdc_reco_tracks)

    # VXD track finder
    if use_svd:
        if use_cdc:
            vxd_reco_tracks = "VXDRecoTracks"

        add_vxd_track_finding(path, components=components, reco_tracks=vxd_reco_tracks)

    # track merging
    if use_svd and use_cdc:
        merged_recotracks = 'RecoTracks'

        # Fit all reco tracks This will be unneeded once the merger is rewritten.
        path.add_module("DAFRecoFitter", recoTracksStoreArrayName=vxd_reco_tracks).set_name("VXD_DAFRecoFitter")
        path.add_module("DAFRecoFitter", recoTracksStoreArrayName=cdc_reco_tracks).set_name("CDC_DAFRecoFitter")

        # Merge CDC and CXD tracks
        vxd_cdcTracksMerger = register_module('VXDCDCTrackMerger')
        vxd_cdcTracksMerger_param = {
            'CDCRecoTrackColName': cdc_reco_tracks,
            'VXDRecoTrackColName': vxd_reco_tracks,
            'MergedRecoTrackColName': merged_recotracks
        }
        vxd_cdcTracksMerger.param(vxd_cdcTracksMerger_param)
        path.add_module(vxd_cdcTracksMerger)

        # We have to prune the two RecoTracks before merging
        path.add_module('PruneRecoTracks', storeArrayName=cdc_reco_tracks)
        path.add_module('PruneRecoTracks', storeArrayName=vxd_reco_tracks)


def add_mc_track_finding(path, components=None):
    """
    Add the MC based TrackFinder to the path.
    """
    if is_cdc_used(components) or is_pxd_used(components) or is_svd_used(components):
        # find MCTracks in CDC, SVD and PXD (or a subset of it)
        path.add_module('TrackFinderMCTruthRecoTracks',
                        UsePXDHits=is_pxd_used(components),
                        UseSVDHits=is_svd_used(components),
                        UseCDCHits=is_cdc_used(components))


def add_cdc_track_finding(path, reco_tracks="RecoTracks"):
    """
    Convenience function for adding all cdc track finder modules
    to the path.

    The result is a StoreArray with name @param reco_tracks full of RecoTracks (not TrackCands any more!).
    Use the GenfitTrackCandidatesCreator Module to convert back.

    Arguments
    ---------
    path: basf2 path
    reco_tracks: Name of the output RecoTracks. Defaults to RecoTracks.
    """

    # Init the geometry for cdc tracking and the hits
    path.add_module("WireHitTopologyPreparer",
                    flightTimeEstimation="outwards",
                    )

    # Find segments and reduce background hits
    path.add_module("SegmentFinderCDCFacetAutomaton",
                    ClusterFilter="tmva",
                    ClusterFilterParameters={"cut": 0.2},
                    FacetUpdateDriftLength=True,
                    FacetFilter="chi2",
                    )

    # Find axial tracks
    path.add_module("TrackFinderCDCLegendreTracking",
                    WriteRecoTracks=False)

    # Improve the quality of the axial tracks
    path.add_module("TrackQualityAsserterCDC",
                    WriteRecoTracks=False,
                    TracksStoreObjNameIsInput=True,
                    corrections=["B2B"])

    # Find the stereo hits to those axial tracks
    path.add_module('StereoHitFinderCDCLegendreHistogramming',
                    useSingleMatchAlgorithm=True,
                    TracksStoreObjNameIsInput=True,
                    WriteRecoTracks=False)

    # Delete segments which where fully used in the last events
    path.add_module("UsedSegmentsDeleter")

    # Combine segments with axial tracks
    path.add_module('SegmentTrackCombinerDev',
                    TracksStoreObjNameIsInput=True,
                    WriteRecoTracks=False,
                    SegmentTrackFilterFirstStepFilter="tmva",
                    SegmentTrackFilterFirstStepFilterParameters={"cut": 0.75},
                    TrackFilter="tmva",
                    TrackFilterParameters={"cut": 0.1})

    # Improve the quality of all tracks and output
    path.add_module("TrackQualityAsserterCDC",
                    WriteRecoTracks=True,
                    TracksStoreObjNameIsInput=True,
                    RecoTracksStoreArrayName=reco_tracks,
                    corrections=["LayerBreak", "LargeBreak2", "OneSuperlayer", "Small"])


def add_vxd_track_finding(path, reco_tracks="RecoTracks", components=None):
    """
    Convenience function for adding all vxd track finder modules
    to the path.

    The result is a StoreArray with name @param reco_tracks full of RecoTracks (not TrackCands any more!).
    Use the GenfitTrackCandidatesCreator Module to convert back.

    Arguments
    ---------
    path: basf2 path
    reco_tracks: Name of the output RecoTracks, Defaults to RecoTracks.
    components: List of the detector components to be used in the reconstruction. Defaults to None which means all
                components.
    """

    # Temporary array
    vxd_trackcands = '__VXDGFTrackCands'

    vxd_trackfinder = path.add_module('VXDTF', GFTrackCandidatesColName=vxd_trackcands)
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
    path.add_module("RecoTrackCreator", trackCandidatesStoreArrayName=vxd_trackcands,
                    recoTracksStoreArrayName=reco_tracks, recreateSortingParameters=True)
