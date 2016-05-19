#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from basf2 import *
from ROOT import Belle2


def is_vxd_used(components):
    """Return true, if the VXD is present in the components list"""
    return components is None or 'SVD' in components


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

    if components and not ('SVD' in components or 'CDC' in components):
        return

    use_vxd = is_vxd_used(components)
    use_cdc = is_cdc_used(components)

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
        path.add_module("GenfitTrackCandidatesCreator")

    # Match the tracks to the MC truth.  The matching works based on
    # the output of the TrackFinderMCTruth.
    mctrackfinder = register_module('TrackFinderMCTruth')
    mctrackfinder.param('GFTrackCandidatesColName', 'MCTrackCands')
    mctrackfinder.param('WhichParticles', [])
    path.add_module(mctrackfinder)

    mctrackmatcher = register_module('MCMatcherTracks')
    mctrackmatcher.param('MCGFTrackCandsColName', 'MCTrackCands')
    mctrackmatcher.param('UsePXDHits', use_vxd)
    mctrackmatcher.param('UseSVDHits', use_vxd)
    mctrackmatcher.param('UseCDCHits', use_cdc)
    path.add_module(mctrackmatcher)

    # track fitting
    trackfitter = path.add_module('GenFitter')
    trackfitter.param({"PDGCodes": [211]})
    trackfitter.set_name('combined GenFitter')

    # create Belle2 Tracks from the genfit Tracks
    trackbuilder = register_module('TrackBuilder')
    path.add_module(trackbuilder)

    # V0 finding
    v0finder = register_module('V0Finder')
    path.add_module(v0finder)

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
    if components and not ('SVD' in components or 'CDC' in components):
        return

    use_vxd = is_vxd_used(components)
    use_cdc = is_cdc_used(components)

    # if only CDC or VXD are used, the track finding result
    # will be directly written to the final RecoTracks array
    # because no merging is required
    cdc_reco_tracks = "RecoTracks"
    vxd_reco_tracks = "RecoTracks"

    # CDC track finder
    if use_cdc:
        if use_vxd:
            cdc_reco_tracks = "CDCRecoTracks"

        add_cdc_track_finding(path, reco_tracks=cdc_reco_tracks)

    # VXD track finder
    if use_vxd:
        if use_cdc:
            vxd_reco_tracks = "VXDRecoTracks"

        add_vxd_track_finding(path, components=components, reco_tracks=vxd_reco_tracks)

    # track merging
    if use_vxd and use_cdc:
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
    # tracking
    if components and not ('PXD' in components or 'SVD' in components or 'CDC'
                           in components):
        return

    # find MCTracks in CDC, SVD, and PXD
    mc_trackfinder = register_module('TrackFinderMCTruth')
    # Default setting in the module may be either way, therefore
    # accomodate both cases explicitly.
    if components is None or 'PXD' in components:
        mc_trackfinder.param('UsePXDHits', 1)
    else:
        mc_trackfinder.param('UsePXDHits', 0)
    if components is None or 'SVD' in components:
        mc_trackfinder.param('UseSVDHits', 1)
    else:
        mc_trackfinder.param('UseSVDHits', 0)
    if components is None or 'CDC' in components:
        mc_trackfinder.param('UseCDCHits', 1)
    else:
        mc_trackfinder.param('UseCDCHits', 0)
    path.add_module(mc_trackfinder)


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
    path.add_module("WireHitTopologyPreparer")

    # Find segments and reduce background hits
    path.add_module("SegmentFinderCDCFacetAutomaton",
                    ClusterFilter="tmva",
                    ClusterFilterParameters={"cut": 0.2})

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

    vxd_trackfinder = path.add_module('VXDTF')
    vxd_trackfinder.param('GFTrackCandidatesColName', vxd_trackcands)
    # WARNING: workaround for possible clashes between fitting and VXDTF
    # stays until the redesign of the VXDTF is finished.
    vxd_trackfinder.param('TESTERexpandedTestingRoutines', False)
    if components is not None and 'PXD' not in components:
        vxd_trackfinder.param('sectorSetup',
                              ['shiftedL3IssueTestSVDStd-moreThan400MeV_SVD',
                               'shiftedL3IssueTestSVDStd-100to400MeV_SVD',
                               'shiftedL3IssueTestSVDStd-25to100MeV_SVD'
                               ])
        vxd_trackfinder.param('tuneCutoffs', 0.06)
    else:
        vxd_trackfinder.param('sectorSetup',
                              ['shiftedL3IssueTestVXDStd-moreThan400MeV_PXDSVD',
                               'shiftedL3IssueTestVXDStd-100to400MeV_PXDSVD',
                               'shiftedL3IssueTestVXDStd-25to100MeV_PXDSVD'
                               ])
        vxd_trackfinder.param('tuneCutoffs', 0.22)

    # Convert VXD trackcands to reco tracks
    path.add_module("RecoTrackCreator", trackCandidatesStoreArrayName=vxd_trackcands,
                    recoTracksStoreArrayName=reco_tracks, recreateSortingParameters=True)
