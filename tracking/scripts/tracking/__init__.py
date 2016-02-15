#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from basf2 import *
from ROOT import Belle2


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

    use_vxd = components is None or 'SVD' in components
    use_cdc = components is None or 'CDC' in components

    # check for detector geometry, necessary for track extrapolation in genfit
    if 'Geometry' not in path:
        geometry = register_module('Geometry')
        if components:
            geometry.param('components', components)
        path.add_module(geometry)

    # Material effects for all track extrapolations
    material_effects = register_module('SetupGenfitExtrapolation')
    path.add_module(material_effects)

    if mcTrackFinding:
        add_mc_track_finding(path, components)
    else:
        add_track_finding(path, components)

    # Match the tracks to the MC truth.  The matching works based on
    # the output of the TrackFinderMCTruth.
    mctrackfinder = register_module('TrackFinderMCTruth')
    mctrackfinder.param('GFTrackCandidatesColName', 'MCTrackCands')
    mctrackfinder.param('WhichParticles', [])
    path.add_module(mctrackfinder)
    mctrackmatcher = register_module('MCMatcherTracks')
    mctrackmatcher.param('MCGFTrackCandsColName', 'MCTrackCands')
    # FIXME 2015/10/30: Stopgap for the release, ideally the module
    # would not care whether the PXD / SVD clusters are available.
    mctrackmatcher.param('UsePXDHits', use_vxd)
    mctrackmatcher.param('UseSVDHits', use_vxd)
    path.add_module(mctrackmatcher)

    # track fitting
    trackfitter = register_module('GenFitter')
    trackfitter.param({'BuildBelle2Tracks': False,
                       "PDGCodes": [211]})
    trackfitter.set_name('combined GenFitter')
    path.add_module(trackfitter)

    # create Belle2 Tracks from the genfit Tracks
    trackbuilder = register_module('TrackBuilder')
    path.add_module(trackbuilder)

    # V0 finding
    v0finder = register_module('V0Finder')
    path.add_module(v0finder)

    # prune genfit tracks
    if pruneTracks:
        add_prune_tracks(path)


def add_prune_tracks(path):
    """
    Adds removal of the intermediate states at each measurement from the fitted tracks.
    """
    path.add_module("PruneGenfitTracks")


def add_track_finding(path, components=None):
    """
    Adds the realistic track finding to the path.
    """
    if components and not ('SVD' in components or 'CDC' in components):
        return

    use_vxd = components is None or 'SVD' in components
    use_cdc = components is None or 'CDC' in components

    # CDC track finder: trasan
    if use_cdc:
        cdc_trackcands = ''
        if use_vxd:
            cdc_trackcands = 'CDCGFTrackCands'
        trackcands = cdc_trackcands
        add_cdc_track_finding(path, trackcands)

    # VXD track finder
    if use_vxd:
        vxd_trackcands = ''
        if use_cdc:
            vxd_trackcands = 'VXDGFTrackCands'
        vxd_trackfinder = register_module('VXDTF')
        vxd_trackfinder.param('GFTrackCandidatesColName', vxd_trackcands)
        # WARNING: workaround for possible clashes between fitting and VXDTF - stays until the redesign of the VXDTF is finished.
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
        path.add_module(vxd_trackfinder)

    # track merging
    if use_vxd and use_cdc:
        vxd_tracklets = 'VXDGFTracks'
        cdc_tracklets = 'CDCGFTracks'
    # track fitting
        VXDtrackFitter = register_module('GenFitter')
        VXDtrackFitter.param('GFTrackCandidatesColName', vxd_trackcands)
        VXDtrackFitter.param('BuildBelle2Tracks', False)
        VXDtrackFitter.param("PDGCodes", [211])
        VXDtrackFitter.param('GFTracksColName', vxd_tracklets)
        VXDtrackFitter.param('PruneFlags', 'FL')
        VXDtrackFitter.set_name('VXD-only GenFitter')

        CDCtrackFitter = register_module('GenFitter')
        CDCtrackFitter.param('GFTrackCandidatesColName', cdc_trackcands)
        CDCtrackFitter.param('BuildBelle2Tracks', False)
        CDCtrackFitter.param("PDGCodes", [211])
        CDCtrackFitter.param('GFTracksColName', cdc_tracklets)
        CDCtrackFitter.param('PruneFlags', 'FL')
        CDCtrackFitter.set_name('CDC-only GenFitter')

        vxd_cdcTracksMerger = register_module('VXDCDCTrackMerger')
        vxd_cdcTracksMerger_param = {
            'VXDGFTrackCandsColName': vxd_trackcands,
            'VXDGFTracksColName': vxd_tracklets,
            'CDCGFTrackCandsColName': cdc_trackcands,
            'CDCGFTracksColName': cdc_tracklets,
            'relMatchedTracks': 'MatchedTracksIdx',
            'chi2_max': 100,
            'recover': 1
        }
        vxd_cdcTracksMerger.param(vxd_cdcTracksMerger_param)

        path.add_module(VXDtrackFitter)
        path.add_module(CDCtrackFitter)
        path.add_module(vxd_cdcTracksMerger)


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


def add_cdc_track_finding(path, cdc_trackcands="TrackCands"):
    """
    Convenience function for adding all cdc track finder modules
    to the path

    Arguments
    ---------
    path: basf2 path
    cdc_trackcands: Name of the output genfit TrackCands. Defaults to TrackCands.
    """

    # Init the geometry for cdc tracking and the hits
    path.add_module("WireHitTopologyPreparer")

    # Find segments and reduce background hits
    path.add_module("SegmentFinderCDCFacetAutomaton",
                    ClusterFilter="tmva",
                    ClusterFilterParameters={"cut": 0.2})

    # Find axial tracks
    path.add_module("TrackFinderCDCLegendreTracking",
                    WriteGFTrackCands=False)

    # Improve the quality of the axial tracks
    path.add_module("TrackQualityAsserterCDC",
                    WriteGFTrackCands=False,
                    TracksStoreObjNameIsInput=True,
                    corrections=["B2B"])

    # Find the stereo hits to those axial tracks
    path.add_module('StereoHitFinderCDCLegendreHistogramming',
                    useSingleMatchAlgorithm=True,
                    TracksStoreObjNameIsInput=True,
                    WriteGFTrackCands=False)

    # Delete segments which where fully used in the last events
    path.add_module("UsedSegmentsDeleter")

    # Combine segments with axial tracks
    path.add_module('SegmentTrackCombinerDev',
                    TracksStoreObjNameIsInput=True,
                    WriteGFTrackCands=False,
                    SegmentTrackFilterFirstStepFilter="tmva",
                    SegmentTrackFilterFirstStepFilterParameters={"cut": 0.75},
                    TrackFilter="tmva",
                    TrackFilterParameters={"cut": 0.1})

    # Improve the quality of all tracks and output
    path.add_module("TrackQualityAsserterCDC",
                    GFTrackCandsStoreArrayName=cdc_trackcands,
                    WriteGFTrackCands=True,
                    TracksStoreObjNameIsInput=True,
                    corrections=["LayerBreak", "LargeBreak2", "OneSuperlayer", "Small"])
