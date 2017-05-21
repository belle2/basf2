#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *


def add_tracking_reconstruction(path, components=None, pruneTracks=False, skipGeometryAdding=False,
                                mcTrackFinding=False, trigger_mode="all", additionalTrackFitHypotheses=None,
                                reco_tracks="RecoTracks", keep_temporary_tracks=False):
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
    :param mcTrackFinding: Use the MC track finders instead of the realistic ones.
    :param trigger_mode: For a description of the available trigger modes see add_reconstruction.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    :param keep_temporary_tracks: If true, store all information of the single CDC and VXD tracks before merging.
        If false, prune them.
    """

    if not is_svd_used(components) and not is_cdc_used(components):
        return

    if not skipGeometryAdding:
        # Add the geometry in all trigger modes if not already in the path
        add_geometry_modules(path, components=components)

    # Material effects for all track extrapolations
    if trigger_mode in ["all", "hlt"] and 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation', energyLossBrems=False, noiseBrems=False)

    if mcTrackFinding:
        # Always add the MC finder in all trigger modes.
        add_mc_track_finding(path, components=components, reco_tracks=reco_tracks)
    else:
        add_track_finding(path, components=components, trigger_mode=trigger_mode, reco_tracks=reco_tracks,
                          keep_temporary_tracks=keep_temporary_tracks)

    if trigger_mode in ["hlt", "all"]:
        add_mc_matcher(path, components=components, reco_tracks=reco_tracks)
        add_track_fit_and_track_creator(path, components=components, pruneTracks=pruneTracks,
                                        additionalTrackFitHypotheses=additionalTrackFitHypotheses,
                                        reco_tracks=reco_tracks)


def add_cr_tracking_reconstruction(path, components=None, pruneTracks=False, skipGeometryAdding=False, eventTimingExtraction=False):
    """
    This function adds the reconstruction modules for cr tracking to a path.

    :param path: The path to which to add the tracking reconstruction modules
    :param components: the list of geometry components in use or None for all components.
    :param pruneTracks: Delete all hits except the first and the last in the found tracks.
    :param skipGeometryAdding: Advances flag: The tracking modules need the geometry module and will add it,
        if it is not already present in the path. In a setup with multiple (conditional) paths however, it cannot
        determine if the geometry is already loaded. This flag can be used to just turn off the geometry adding
        (but you will have to add it on your own).
    :param eventTimingExtraction: extract time with either the TrackTimeExtraction or
        FullGridTrackTimeExtraction modules.
    """

    # make sure CDC is used
    if not is_cdc_used(components):
        return

    if not skipGeometryAdding:
        # Add the geometry in all trigger modes if not already in the path
        add_geometry_modules(path, components)

    # Material effects for all track extrapolations
    if 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation', energyLossBrems=False, noiseBrems=False)

    # track finding
    add_cdc_cr_track_finding(path)

    # track fitting
    add_cdc_cr_track_fit_and_track_creator(path, components, pruneTracks, eventTimingExtraction)


def add_geometry_modules(path, components=None):
    """
    Helper function to add the geometry related modules needed for tracking
    to the path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    """
    # check for detector geometry, necessary for track extrapolation in genfit
    if 'Geometry' not in path:
        geometry = register_module('Geometry')
        if components:
            geometry.param('components', components)
        path.add_module(geometry)

    # Material effects for all track extrapolations
    if 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation', energyLossBrems=False, noiseBrems=False)


def add_mc_tracking_reconstruction(path, components=None, pruneTracks=False):
    """
    This function adds the standard reconstruction modules for MC tracking
    to a path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param pruneTracks: Delete all hits expect the first and the last from the found tracks.
    """
    add_tracking_reconstruction(path,
                                components=components,
                                pruneTracks=pruneTracks,
                                mcTrackFinding=True)


def add_track_fit_and_track_creator(path, components=None, pruneTracks=False, additionalTrackFitHypotheses=None,
                                    reco_tracks="RecoTracks"):
    """
    Helper function to add the modules performing the
    track fit, the V0 fit and the Belle2 track creation to the path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param pruneTracks: Delete all hits expect the first and the last from the found tracks.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    """
    # Correct time seed
    path.add_module("IPTrackTimeEstimator", recoTracksStoreArrayName=reco_tracks, useFittedInformation=False)
    # track fitting
    path.add_module("DAFRecoFitter", recoTracksStoreArrayName=reco_tracks).set_name("Combined_DAFRecoFitter")
    # create Belle2 Tracks from the genfit Tracks
    path.add_module('TrackCreator', defaultPDGCode=211, recoTrackColName=reco_tracks,
                    additionalPDGCodes=[13, 321, 2212] if additionalTrackFitHypotheses is None else additionalTrackFitHypotheses)
    # V0 finding
    path.add_module('V0Finder', RecoTrackColName=reco_tracks)

    # prune genfit tracks
    if pruneTracks:
        add_prune_tracks(path, components=components, reco_tracks=reco_tracks)


def add_cdc_cr_track_fit_and_track_creator(
    path, components=None, pruneTracks=False, eventTimingExtraction=False, lightPropSpeed=12.9925, triggerPos=[
        0, 0, 0], normTriggerPlaneDirection=[
            0, 1, 0], readOutPos=[
                0, 0, -50.0]):
    """
    Helper function to add the modules performing the cdc cr track fit
    and track creation to the path.

    :param path: The path to which to add the tracking reconstruction modules
    :param components: the list of geometry components in use or None for all components.
    :param pruneTracks: Delete all hits expect the first and the last from the found tracks.
    :param eventTimingExtraction: extract time with either the TrackTimeExtraction or
        FullGridTrackTimeExtraction modules.
    """

    # Time seed
    path.add_module("PlaneTriggerTrackTimeEstimator",
                    pdgCodeToUseForEstimation=13,
                    triggerPlanePosition=triggerPos,
                    triggerPlaneDirection=normTriggerPlaneDirection,
                    useFittedInformation=False)

    # Initial track fitting
    path.add_module("DAFRecoFitter",
                    probCut=0.00001,
                    pdgCodesToUseForFitting=13,
                    )

    # Correct time seed
    path.add_module("PlaneTriggerTrackTimeEstimator",
                    pdgCodeToUseForEstimation=13,
                    triggerPlanePosition=triggerPos,
                    triggerPlaneDirection=normTriggerPlaneDirection,
                    useFittedInformation=True,
                    useReadoutPosition=True,
                    readoutPosition=readOutPos,
                    readoutPositionPropagationSpeed=lightPropSpeed
                    )

    # Track fitting
    path.add_module("DAFRecoFitter",
                    # probCut=0.00001,
                    pdgCodesToUseForFitting=13,
                    )

    if eventTimingExtraction is True:
        # Select the tracks for the time extraction.
        path.add_module("SelectionForTrackTimeExtraction")

        # Extract the time
        path.add_module("FullGridTrackTimeExtraction")

        # Track fitting
        path.add_module("DAFRecoFitter",
                        # probCut=0.00001,
                        pdgCodesToUseForFitting=13,
                        )

    # Create Belle2 Tracks from the genfit Tracks
    path.add_module('TrackCreator',
                    defaultPDGCode=13,
                    useClosestHitToIP=True
                    )

    # Prune genfit tracks
    if pruneTracks:
        add_prune_tracks(path, components)


def add_mc_matcher(path, components=None, reco_tracks="RecoTracks"):
    """
    Match the tracks to the MC truth. The matching works based on
    the output of the TrackFinderMCTruthRecoTracks.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    """
    path.add_module('TrackFinderMCTruthRecoTracks',
                    RecoTracksStoreArrayName='MCRecoTracks',
                    WhichParticles=[],
                    UsePXDHits=is_pxd_used(components),
                    UseSVDHits=is_svd_used(components),
                    UseCDCHits=is_cdc_used(components))

    path.add_module('MCRecoTracksMatcher',
                    mcRecoTracksStoreArrayName='MCRecoTracks',
                    prRecoTracksStoreArrayName=reco_tracks,
                    UsePXDHits=is_pxd_used(components),
                    UseSVDHits=is_svd_used(components),
                    UseCDCHits=is_cdc_used(components))


def add_prune_tracks(path, components=None, reco_tracks="RecoTracks"):
    """
    Adds removal of the intermediate states at each measurement from the fitted tracks.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    """

    # do not add any pruning, if no tracking detectors are in the components
    if components and not ('SVD' in components or 'CDC' in components):
        return

    path.add_module('PruneRecoTracks', storeArrayName=reco_tracks)
    path.add_module("PruneGenfitTracks")


def add_track_finding(path, components=None, trigger_mode="all", reco_tracks="RecoTracks", keep_temporary_tracks=False):
    """
    Adds the realistic track finding to the path.
    The result is a StoreArray 'RecoTracks' full of RecoTracks (not TrackCands any more!).
    Use the GenfitTrackCandidatesCreator Module to convert back.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param trigger_mode: For a description of the available trigger modes see add_reconstruction.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    :param keep_temporary_tracks: If true, store all information of the single CDC and VXD tracks before merging.
        If false, prune them.
    """
    if not is_svd_used(components) and not is_cdc_used(components):
        return

    use_svd = is_svd_used(components)
    use_cdc = is_cdc_used(components)

    # if only CDC or VXD are used, the track finding result
    # will be directly written to the final RecoTracks array
    # because no merging is required

    if use_cdc and use_svd:
        cdc_reco_tracks = "CDCRecoTracks"
        vxd_reco_tracks = "VXDRecoTracks"
    else:
        cdc_reco_tracks = reco_tracks
        vxd_reco_tracks = reco_tracks

    # CDC track finder
    if use_cdc and trigger_mode in ["fast_reco", "all"]:
        add_cdc_track_finding(path, reco_tracks=cdc_reco_tracks)

    # VXD track finder
    if use_svd and trigger_mode in ["hlt", "all"]:
        add_vxd_track_finding(path, components=components, reco_tracks=vxd_reco_tracks)

        # track merging
        if use_svd and use_cdc:
            # Fit all reco tracks This will be unneeded once the merger is rewritten.
            path.add_module("DAFRecoFitter", recoTracksStoreArrayName=vxd_reco_tracks).set_name("VXD_DAFRecoFitter")
            path.add_module("DAFRecoFitter", recoTracksStoreArrayName=cdc_reco_tracks).set_name("CDC_DAFRecoFitter")

            # Merge CDC and CXD tracks
            path.add_module('VXDCDCTrackMerger', CDCRecoTrackColName=cdc_reco_tracks,
                            VXDRecoTrackColName=vxd_reco_tracks, MergedRecoTrackColName=reco_tracks)

            # Prune the temporary products if requested
            if not keep_temporary_tracks:
                path.add_module('PruneRecoTracks', storeArrayName=cdc_reco_tracks)
                path.add_module('PruneRecoTracks', storeArrayName=vxd_reco_tracks)


def add_mc_track_finding(path, components=None, reco_tracks="RecoTracks"):
    """
    Add the MC based TrackFinder to the path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    :param reco_tracks: Name of the StoreArray where the reco tracks should be stored
    """
    if is_cdc_used(components) or is_pxd_used(components) or is_svd_used(components):
        # find MCTracks in CDC, SVD and PXD (or a subset of it)
        path.add_module('TrackFinderMCTruthRecoTracks',
                        RecoTracksStoreArrayName=reco_tracks,
                        UsePXDHits=is_pxd_used(components),
                        UseSVDHits=is_svd_used(components),
                        UseCDCHits=is_cdc_used(components))


def add_cdc_track_finding(path, reco_tracks="RecoTracks", with_ca=False):
    """
    Convenience function for adding all cdc track finder modules
    to the path.

    The result is a StoreArray with name @param reco_tracks full of RecoTracks (not TrackCands any more!).
    Use the GenfitTrackCandidatesCreator Module to convert back.

    :param path: basf2 path
    :param reco_tracks: Name of the output RecoTracks. Defaults to RecoTracks.
    """

    # Init the geometry for cdc tracking and the hits
    path.add_module("TFCDC_WireHitPreparer",
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

    if with_ca:
        # Add curlers in the axial inner most superlayer
        path.add_module("TFCDC_TrackCreatorSingleSegments",
                        MinimalHitsBySuperLayerId={0: 15})

    # Export CDCTracks to RecoTracks representation
    path.add_module("TFCDC_TrackExporter",
                    RecoTracksStoreArrayName=reco_tracks)

    # Correct time seed (only necessary for the CDC tracks)
    path.add_module("IPTrackTimeEstimator",
                    useFittedInformation=False,
                    recoTracksStoreArrayName=reco_tracks)


def add_cdc_cr_track_finding(path, reco_tracks="RecoTracks", trigger_point=(0, 0, 0), merge_tracks=True):
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
    """

    # Init the geometry for cdc tracking and the hits
    path.add_module("TFCDC_WireHitPreparer",
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
        path.add_module("TFCDC_CosmicsTrackMerger",
                        inputTracks="OrientedCDCTrackVector",
                        tracks="MergedCDCTrackVector",
                        useSimpleApproach=True,
                        )

        output_tracks = "MergedCDCTrackVector"

    # Correct time seed - assumes velocity near light speed
    path.add_module("TFCDC_TrackFlightTimeAdjuster",
                    inputTracks=output_tracks,
                    )

    # Export CDCTracks to RecoTracks representation
    path.add_module("TFCDC_TrackExporter",
                    inputTracks=output_tracks,
                    RecoTracksStoreArrayName=reco_tracks)


def add_vxd_track_finding(path, reco_tracks="RecoTracks", components=None):
    """
    Convenience function for adding all vxd track finder modules
    to the path.

    The result is a StoreArray with name @param reco_tracks full of RecoTracks (not TrackCands any more!).
    Use the GenfitTrackCandidatesCreator Module to convert back.

    :param path: basf2 path
    :param reco_tracks: Name of the output RecoTracks, Defaults to RecoTracks.
    :param components: List of the detector components to be used in the reconstruction. Defaults to None which means all
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


def is_svd_used(components):
    """Return true, if the SVD is present in the components list"""
    return components is None or 'SVD' in components


def is_pxd_used(components):
    """Return true, if the PXD is present in the components list"""
    return components is None or 'PXD' in components


def is_cdc_used(components):
    """Return true, if the CDC is present in the components list"""
    return components is None or 'CDC' in components


def add_tracking_for_PXDDataReduction_simulation(path, components=None, skipGeometryAdding=False):
    """
    This function adds the standard reconstruction modules for tracking to be used for the simulation of PXD data reduction
    to a path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components, always exclude the PXD.
    :param skipGeometryAdding: Advances flag: The tracking modules need the geometry module and will add it,
        if it is not already present in the path. In a setup with multiple (conditional) paths however, it can not
        determine, if the geometry is already loaded. This flag can be used o just turn off the geometry adding at
        all (but you will have to add it on your own then).
    """

    if not is_svd_used(components) and not is_cdc_used(components):
        return

    if not skipGeometryAdding:
        # Add the geometry in all trigger modes if not already in the path
        add_geometry_modules(path, components)

    # Material effects
    if 'SetupGenfitExtrapolation' not in path:
        material_effects = register_module('SetupGenfitExtrapolation')
        material_effects.set_name('SetupGenfitExtrapolationForPXDDataReduction')
        path.add_module(material_effects)

    # SET StoreArray names
    svd_tracks = '__ROIsvdTracks'
    svd_track_fit_results = "__ROIsvdTrackFitResults"
    svd_reco_tracks = "__ROIsvdRecoTracks"
    svd_gf_trackcands = '__ROIsvdGFTrackCands'

    # SVD ONLY TRACK FINDING
    vxd_trackfinder = path.add_module('VXDTF')
    vxd_trackfinder.set_name('SVD-only VXDTF')
    vxd_trackfinder.param('GFTrackCandidatesColName', svd_gf_trackcands)
    vxd_trackfinder.param('TESTERexpandedTestingRoutines', False)
    vxd_trackfinder.param('sectorSetup',
                          ['shiftedL3IssueTestSVDStd-moreThan400MeV_SVD',
                           'shiftedL3IssueTestSVDStd-100to400MeV_SVD',
                           'shiftedL3IssueTestSVDStd-25to100MeV_SVD'
                           ])
    vxd_trackfinder.param('tuneCutoffs', 0.06)

    # Convert VXD trackcands to reco tracks
    # not in the path yet, wait for the transition to RecoTracks before
    recoTrackCreator = register_module("RecoTrackCreator")
    recoTrackCreator.param('trackCandidatesStoreArrayName', svd_gf_trackcands)
    recoTrackCreator.param('recoTracksStoreArrayName', svd_reco_tracks)
    recoTrackCreator.param('recreateSortingParameters', True)
    path.add_module(recoTrackCreator)

    # TRACK FITTING

    # Correct time seed - needed?
    # path.add_module("IPTrackTimeEstimator", useFittedInformation=False)

    # track fitting
    dafRecoFitter = register_module("DAFRecoFitter")
    dafRecoFitter.set_name("SVD-only DAFRecoFitter")
    dafRecoFitter.param('recoTracksStoreArrayName', svd_reco_tracks)
    path.add_module(dafRecoFitter)
