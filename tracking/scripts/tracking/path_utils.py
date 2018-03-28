from pybasf2 import B2WARNING

from basf2 import register_module
from ckf.path_functions import add_pxd_ckf, add_ckf_based_merger, add_svd_ckf
from pxd import add_pxd_reconstruction
from svd import add_svd_reconstruction


def add_geometry_modules(path, components=None):
    """
    Helper function to add the geometry related modules needed for tracking
    to the path.

    :param path: The path to add the tracking reconstruction modules to
    :param components: the list of geometry components in use or None for all components.
    """
    # check for detector geometry, necessary for track extrapolation in genfit
    if 'Geometry' not in path:
        geometry = register_module('Geometry', useDB=True)
        if components is not None:
            B2WARNING("Custom detector components specified, disabling Geometry from Database")
            geometry.param('useDB', False)
            geometry.param('components', components)
        path.add_module(geometry)

    # Material effects for all track extrapolations
    if 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation',
                        energyLossBrems=False, noiseBrems=False)


def add_hit_preparation_modules(path, components=None):
    """
    Helper fucntion to prepare the hit information to be used by tracking.
    """

    # Preparation of the SVD clusters
    if is_svd_used(components):
        add_svd_reconstruction(path)

    # Preparation of the PXD clusters
    if is_pxd_used(components):
        add_pxd_reconstruction(path)


def add_track_fit_and_track_creator(path, components=None, pruneTracks=False, trackFitHypotheses=None,
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
    path.add_module("IPTrackTimeEstimator",
                    recoTracksStoreArrayName=reco_tracks, useFittedInformation=False)
    # track fitting
    path.add_module("DAFRecoFitter", recoTracksStoreArrayName=reco_tracks).set_name(
        "Combined_DAFRecoFitter")
    # create Belle2 Tracks from the genfit Tracks
    # The following particle hypothesis will be fitted: Pion, Kaon and Proton
    # Muon fit is working but gives very similar as the Pion due to the closeness of masses
    # -> therefore not in the default fit list
    # Electron fit has as systematic bias and therefore not done here. Therefore, pion fits
    # will be used for electrons which gives a better result as GenFit's current electron
    # implementation.
    path.add_module('TrackCreator', recoTrackColName=reco_tracks,
                    pdgCodes=[211, 321, 2212] if not trackFitHypotheses else trackFitHypotheses)
    # V0 finding
    path.add_module('V0Finder', RecoTracks=reco_tracks)

    # prune genfit tracks
    if pruneTracks:
        add_prune_tracks(path, components=components, reco_tracks=reco_tracks)


def add_cr_track_fit_and_track_creator(path, components=None,
                                       data_taking_period='gcr2017', top_in_counter=False,
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
    :param event_timing_extraction: extract time with either the TrackTimeExtraction or
        FullGridTrackTimeExtraction modules.

    :param top_in_counter: time of propagation from the hit point to the PMT in the trigger counter is subtracted
           (assuming PMT is put at -z of the counter).
    """

    if data_taking_period != "phase2":
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
        path.add_module("FullGridTrackTimeExtraction",
                        recoTracksStoreArrayName=reco_tracks,
                        maximalT0Shift=40,
                        minimalT0Shift=-40,
                        numberOfGrids=6
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
                    UseCDCHits=is_cdc_used(components),
                    SplitAfterDeltaT=split_after_delta_t)

    path.add_module('MCRecoTracksMatcher',
                    mcRecoTracksStoreArrayName=mc_reco_tracks,
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


def add_svd_track_finding(path, components, input_reco_tracks, output_reco_tracks, svd_ckf_mode="VXDTF2_after",
                          use_mc_truth=False, add_both_directions=True, temporary_reco_tracks="SVDRecoTracks", **kwargs):
    """Add SVD track finding to the path"""

    if not is_svd_used(components):
        return

    if not input_reco_tracks:
        # We do not have an input track store array. So lets just add vxdtf track finding
        add_vxd_track_finding_vxdtf2(path, components=["SVD"], reco_tracks=output_reco_tracks)
        return

    if use_mc_truth:
        # MC CKF needs MC matching information
        path.add_module("MCRecoTracksMatcher", UsePXDHits=False, UseSVDHits=False,
                        UseCDCHits=is_cdc_used(components),
                        mcRecoTracksStoreArrayName="MCRecoTracks",
                        prRecoTracksStoreArrayName=input_reco_tracks)

    if svd_ckf_mode == "VXDTF2_before":
        add_vxd_track_finding_vxdtf2(path, components=["SVD"], reco_tracks=temporary_reco_tracks)
        add_ckf_based_merger(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                             use_mc_truth=use_mc_truth, direction="backward", **kwargs)
        if add_both_directions:
            add_ckf_based_merger(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                                 use_mc_truth=use_mc_truth, direction="forward", **kwargs)

    elif svd_ckf_mode == "VXDTF2_before_with_second_ckf":
        add_vxd_track_finding_vxdtf2(path, components=["SVD"], reco_tracks=temporary_reco_tracks)
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

        add_vxd_track_finding_vxdtf2(path, components=["SVD"], reco_tracks=temporary_reco_tracks)
        add_ckf_based_merger(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                             use_mc_truth=use_mc_truth, direction="backward", **kwargs)
        if add_both_directions:
            add_ckf_based_merger(path, cdc_reco_tracks=input_reco_tracks, svd_reco_tracks=temporary_reco_tracks,
                                 use_mc_truth=use_mc_truth, direction="forward", **kwargs)

    elif svd_ckf_mode == "VXDTF2_alone":
        add_vxd_track_finding_vxdtf2(path, components=["SVD"], reco_tracks=temporary_reco_tracks)
        path.add_module('VXDCDCTrackMerger',
                        CDCRecoTrackColName=input_reco_tracks,
                        VXDRecoTrackColName=temporary_reco_tracks)

    else:
        raise ValueError(f"Do not understand the svd_ckf_mode {svd_ckf_mode}")

        # Write out the combinations of tracks
    path.add_module("RelatedTracksCombiner", VXDRecoTracksStoreArrayName=temporary_reco_tracks,
                    CDCRecoTracksStoreArrayName=input_reco_tracks,
                    recoTracksStoreArrayName=output_reco_tracks)


def add_cdc_track_finding(path, output_reco_tracks="RecoTracks", with_ca=False, use_second_hits=False):
    """
    Convenience function for adding all cdc track finder modules
    to the path.

    The result is a StoreArray with name @param reco_tracks full of RecoTracks (not TrackCands any more!).
    Use the GenfitTrackCandidatesCreator Module to convert back.

    :param path: basf2 path
    :param output_reco_tracks: Name of the output RecoTracks. Defaults to RecoTracks.
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

    # Export CDCTracks to RecoTracks representation
    path.add_module("TFCDC_TrackExporter",
                    inputTracks=output_tracks,
                    RecoTracksStoreArrayName=output_reco_tracks)

    # Correct time seed (only necessary for the CDC tracks)
    path.add_module("IPTrackTimeEstimator",
                    useFittedInformation=False,
                    recoTracksStoreArrayName=output_reco_tracks)

    # run fast t0 estimation from CDC hits only
    path.add_module("CDCHitBasedT0Extraction")


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


def add_vxd_track_finding_vxdtf2(path, svd_clusters="", reco_tracks="RecoTracks", components=None, suffix="",
                                 useTwoStepSelection=True, PXDminSVDSPs=3, use_quality_estimator_mva=True,
                                 QEMVA_weight_file='tracking/data/VXDQE_weight_files/Default-CoG-noTime.xml',
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
    :param use_quality_estimator_mva: Whether to use the MVA methode to refine the quality estimator; default is True
    :param QEMVA_weight_file: Weight file to be used by the MVA Quality Estimator
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
            spCreatorPXD.param('SpacePoints', "PXD" + nameSPs)
            path.add_module(spCreatorPXD)

    # check for the name instead of the type as the HLT also need those module under (should have different names)
    svdSPCreatorName = 'SVDSpacePointCreator' + suffix
    if svdSPCreatorName not in [e.name() for e in path.modules()]:
        # always use svd!
        spCreatorSVD = register_module('SVDSpacePointCreator')
        spCreatorSVD.set_name(svdSPCreatorName)
        spCreatorSVD.param('OnlySingleClusterSpacePoints', False)
        spCreatorSVD.param('NameOfInstance', 'SVDSpacePoints')
        spCreatorSVD.param('SpacePoints', "SVD" + nameSPs)
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

    spacePointArrayNames = ["SVD" + nameSPs]
    if use_pxd:
        spacePointArrayNames += ["PXD" + nameSPs]

    nameSegNet = 'SegmentNetwork' + suffix
    segNetProducer = register_module('SegmentNetworkProducer')
    segNetProducer.param('NetworkOutputName', nameSegNet)
    segNetProducer.param('SpacePointsArrayNames', spacePointArrayNames)
    segNetProducer.param('sectorMapName', custom_setup_name or setup_name)
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
    trackFinder.param('printNetworks', False)
    trackFinder.param('setFamilies', useTwoStepSelection)
    trackFinder.param('selectBestPerFamily', useTwoStepSelection)
    trackFinder.param('xBestPerFamily', 30)
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
    qualityEstimator = register_module('QualityEstimatorVXD' if not use_quality_estimator_mva
                                       else 'QualityEstimatorMVA')
    qualityEstimator.param('EstimationMethod', quality_estimator)
    qualityEstimator.param('SpacePointTrackCandsStoreArrayName', nameSPTCs)
    if use_quality_estimator_mva:
        qualityEstimator.param('WeightFileIdentifier', QEMVA_weight_file)
        qualityEstimator.param('UseTimingInfo', False)
        qualityEstimator.param('ClusterInformation', 'Average')

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
